// Copyright (C) Codeplay Software Limited. All Rights Reserved.

#include "vectorization_context.h"

#include <compiler/utils/builtin_info.h>
#include <compiler/utils/group_collective_helpers.h>
#include <compiler/utils/mangling.h>
#include <llvm/ADT/Statistic.h>
#include <llvm/IR/Attributes.h>
#include <llvm/Target/TargetMachine.h>
#include <multi_llvm/creation_apis_helper.h>
#include <multi_llvm/opaque_pointers.h>
#include <multi_llvm/vector_type_helper.h>

#include <algorithm>
#include <cassert>

#include "analysis/vectorization_unit_analysis.h"
#include "debugging.h"
#include "llvm_helpers.h"
#include "memory_operations.h"
#include "transform/packetization_helpers.h"
#include "vectorization_helpers.h"
#include "vectorization_unit.h"
#include "vecz/vecz_choices.h"
#include "vecz/vecz_target_info.h"

#define DEBUG_TYPE "vecz"

using namespace llvm;
using namespace vecz;

STATISTIC(VeczContextFailBuiltin,
          "Context: builtins with no vector equivalent [ID#V84]");
STATISTIC(VeczContextFailScalarizeCall,
          "Context: non-scalarizable vector builtin [ID#V86]");

/// @brief Prefix used to distinguish internal vecz builtins from OpenCL
/// builtins and user functions.
const char *VectorizationContext::InternalBuiltinPrefix = "__vecz_b_";

VectorizationContext::VectorizationContext(llvm::Module &target,
                                           TargetInfo &vti,
                                           compiler::utils::BuiltinInfo &bi)
    : VTI(vti), Module(target), BI(bi), DL(&Module.getDataLayout()) {}

TargetTransformInfo VectorizationContext::getTargetTransformInfo(
    Function &F) const {
  auto *const TM = targetInfo().getTargetMachine();
  if (TM) {
    return TM->getTargetTransformInfo(F);
  } else {
    return TargetTransformInfo(F.getParent()->getDataLayout());
  }
}

VectorizationUnit *VectorizationContext::getActiveVU(const Function *F) const {
  const auto I = ActiveVUs.find(F);
  if (I == ActiveVUs.end()) {
    return nullptr;
  }
  VectorizationUnit *VU = I->second;
  assert(VU->vectorizedFunction() == F);
  return VU;
}

compiler::utils::BuiltinInfo &VectorizationContext::builtins() { return BI; }

const compiler::utils::BuiltinInfo &VectorizationContext::builtins() const {
  return BI;
}

VectorizationUnit *VectorizationContext::createVectorizationUnit(
    llvm::Function &F, ElementCount VF, unsigned Dimension,
    const VectorizationChoices &Ch) {
  KernelUnits.push_back(
      multi_llvm::make_unique<VectorizationUnit>(F, VF, Dimension, *this, Ch));
  return KernelUnits.back().get();
}

bool VectorizationContext::isVector(const Instruction &I) {
  if (I.getType()->isVectorTy()) {
    return true;
  }
  for (const Use &op : I.operands()) {
    if (op->getType()->isVectorTy()) {
      return true;
    }
  }
  return false;
}

bool VectorizationContext::canExpandBuiltin(const Function *ScalarFn) const {
  // Builtins that return no value must have side-effects.
  if (ScalarFn->getReturnType()->isVoidTy()) {
    return false;
  }
  for (const Argument &Arg : ScalarFn->args()) {
    // Most builtins that take pointers have side-effects. Be conservative.
    if (Arg.getType()->isPointerTy()) {
      return false;
    }
  }
  return true;
}

VectorizationResult &VectorizationContext::getOrCreateBuiltin(
    llvm::Function &F, unsigned SimdWidth) {
  compiler::utils::BuiltinInfo &BI = builtins();
  auto const Cached = VectorizedBuiltins.find(&F);
  if (Cached != VectorizedBuiltins.end()) {
    auto const Found = Cached->second.find(SimdWidth);
    if (Found != Cached->second.end()) {
      return Found->second;
    }
  }

  auto const Builtin = BI.analyzeBuiltin(F);

  // Try to find a vector equivalent for the builtin.
  Function *const VectorCallee =
      isInternalBuiltin(&F)
          ? getInternalVectorEquivalent(&F, SimdWidth)
          : BI.getVectorEquivalent(Builtin, SimdWidth, &Module);

  auto &result = VectorizedBuiltins[&F][SimdWidth];
  if (!VectorCallee) {
    ++VeczContextFailBuiltin;
    return result;
  }

  compiler::utils::NameMangler Mangler(&F.getContext(), &Module);
  auto const BuiltinName = Mangler.demangleName(F.getName()).str();

  result.func = VectorCallee;

  // Gather information about the function's arguments.
  auto const Props = Builtin.properties;
  unsigned i = 0;
  for (Argument &Arg : F.args()) {
    Type *pointerRetPointeeTy = nullptr;
    VectorizationResult::Arg::Kind kind = VectorizationResult::Arg::SCALAR;

    if (Arg.getType()->isPointerTy()) {
      pointerRetPointeeTy =
          compiler::utils::getPointerReturnPointeeTy(F, Props);
      kind = VectorizationResult::Arg::POINTER_RETURN;
    } else {
      kind = VectorizationResult::Arg::VECTORIZED;
    }
    result.args.emplace_back(kind, VectorCallee->getArg(i)->getType(),
                             pointerRetPointeeTy);
    i++;
  }
  return result;
}

VectorizationResult VectorizationContext::getVectorizedFunction(
    Function &callee, ElementCount factor) {
  VectorizationResult result;
  if (factor.isScalable()) {
    // We can't vectorize builtins by a scalable factor yet.
    return result;
  }

  auto simdWidth = factor.getFixedValue();
  if (auto *vecTy =
          dyn_cast<multi_llvm::FixedVectorType>(callee.getReturnType())) {
    auto const Builtin = BI.analyzeBuiltin(callee);
    Function *scalarEquiv = builtins().getScalarEquivalent(Builtin, &Module);
    if (!scalarEquiv) {
      ++VeczContextFailScalarizeCall;
      return VectorizationResult();
    }

    auto scalarWidth = vecTy->getNumElements();

    result = getOrCreateBuiltin(*scalarEquiv, simdWidth * scalarWidth);
  } else {
    result = getOrCreateBuiltin(callee, simdWidth);
  }
  return result;
}

bool VectorizationContext::isInternalBuiltin(const Function *F) {
  return F->getName().startswith(VectorizationContext::InternalBuiltinPrefix);
}

Function *VectorizationContext::getOrCreateInternalBuiltin(StringRef Name,
                                                           FunctionType *FT) {
  Function *F = Module.getFunction(Name);
  if (!F && FT) {
    F = dyn_cast_or_null<Function>(
        Module.getOrInsertFunction(Name, FT).getCallee());
  }

  return F;
}

Function *VectorizationContext::getOrCreateMaskedFunction(CallInst *CI) {
  Function *F = CI->getCalledFunction();
  if (!F) {
    F = dyn_cast<Function>(CI->getCalledOperand()->stripPointerCasts());
  }
  VECZ_FAIL_IF(!F);  // TODO: CA-1505: Support indirect function calls.
  LLVMContext &ctx = F->getContext();

  // We will handle printf statements, but handling every possible vararg
  // function can become a bit too complex, among other things because name
  // mangling with arbitrary types can become a bit complex. printf is the only
  // vararg OpenCL builtin, so only user functions are affected by this.
  bool isVarArg = F->isVarArg();
  VECZ_FAIL_IF(isVarArg && F->getName() != "printf");
  // Copy the argument types. This is done from the CallInst instead of the
  // called Function because the called Function might be a VarArg function, in
  // which case we need to create the wrapper with the expanded argument list.
  SmallVector<Type *, 8> argTys;
  for (auto const &U : CI->args()) {
    argTys.push_back(U->getType());
  }
  AttributeList fnAttrs = F->getAttributes();
  unsigned firstImmArg;
  const bool hasImmArg =
      F->isIntrinsic() &&
      fnAttrs.hasAttrSomewhere(Attribute::ImmArg, &firstImmArg);
  if (hasImmArg) {
    firstImmArg -= AttributeList::FirstArgIndex;
    // We can only handle a single `i1` `Immarg` parameter. If we outgrow this
    // limitation we need a different approach to the single inner branch
    int count = 0;
    for (unsigned i = firstImmArg, n = argTys.size(); i < n; ++i) {
      if (!fnAttrs.hasAttributeAtIndex(AttributeList::FirstArgIndex + i,
                                       Attribute::ImmArg)) {
        continue;
      }
      // We only support one ImmArg or i1 type
      if (count++ || argTys[i] != Type::getInt1Ty(ctx)) {
        return nullptr;
      }
      fnAttrs = fnAttrs.removeAttributeAtIndex(ctx, i, Attribute::ImmArg);
    }
  }
  // Add one extra argument for the mask
  argTys.push_back(Type::getInt1Ty(ctx));
  // Generate the function name
  compiler::utils::NameMangler mangler(&ctx);
  SmallVector<compiler::utils::TypeQualifiers, 8> quals(
      argTys.size(), compiler::utils::TypeQualifiers());
  std::string newFName;
  raw_string_ostream O(newFName);
  O << VectorizationContext::InternalBuiltinPrefix << "masked_" << F->getName();
  // We need to mangle the names of the vararg masked functions, since we will
  // generate different masked functions for invocations with different argument
  // types. For non-vararg functions, we don't need the mangling so we skip it.
  if (isVarArg) {
    O << "_";
    for (auto T : argTys) {
      VECZ_FAIL_IF(!mangler.mangleType(
          O, T,
          compiler::utils::TypeQualifiers(compiler::utils::eTypeQualNone)));
    }
  }
  O.flush();
  // Check if we have a masked version already
  auto maskedVersion = MaskedVersions.find(newFName);
  if (maskedVersion != MaskedVersions.end()) {
    LLVM_DEBUG(dbgs() << "vecz: Found existing masked function " << newFName
                      << "\n");
    return maskedVersion->second;
  }
  // Create the function type
  FunctionType *newFunctionTy =
      FunctionType::get(F->getReturnType(), argTys, false);
  Function *newFunction = Function::Create(
      newFunctionTy, GlobalValue::PrivateLinkage, newFName, F->getParent());
  const CallingConv::ID cc = CI->getCallingConv();
  LLVM_DEBUG(dbgs() << "vecz: Created masked function " << newFName << "\n");

  // Create the function's basic blocks
  BasicBlock *entryBlock = BasicBlock::Create(ctx, "entry", newFunction);
  BasicBlock *activeBlock = BasicBlock::Create(ctx, "active", newFunction);
  BasicBlock *mergeBlock = BasicBlock::Create(ctx, "exit", newFunction);

  // Create a new call instruction to call the masked function
  SmallVector<Value *, 8> CIArgs;
  for (Value &arg : newFunction->args()) {
    CIArgs.push_back(&arg);
  }
  // Remove the mask argument
  CIArgs.pop_back();

  FunctionType *FTy = CI->getFunctionType();
  AttributeList callAttrs = CI->getAttributes();
  SmallVector<std::pair<Value *, BasicBlock *>, 4> PhiOperands;
  if (hasImmArg) {
    Value *immArg = newFunction->getArg(firstImmArg);
    BasicBlock *immTrue =
        BasicBlock::Create(ctx, "active.imm.1", newFunction, mergeBlock);
    CIArgs[firstImmArg] = ConstantInt::getTrue(ctx);
    CallInst *c0 = multi_llvm::createCallInst(CI->getCalledOperand(), FTy,
                                              CIArgs, "", immTrue);
    c0->setCallingConv(cc);
    c0->setAttributes(callAttrs);
    BranchInst::Create(mergeBlock, immTrue);

    CIArgs[firstImmArg] = ConstantInt::getFalse(ctx);
    // Now the false half
    BasicBlock *immFalse =
        BasicBlock::Create(ctx, "active.imm.0", newFunction, mergeBlock);

    CallInst *c1 = multi_llvm::createCallInst(CI->getCalledOperand(), FTy,
                                              CIArgs, "", immFalse);
    c1->setCallingConv(cc);
    c1->setAttributes(callAttrs);
    BranchInst::Create(mergeBlock, immFalse);
    BranchInst::Create(immTrue, immFalse, immArg, activeBlock);
    PhiOperands.push_back({c0, immTrue});
    PhiOperands.push_back({c1, immFalse});

    // Now fix up the new function's signature. It can't be inheriting illegal
    // attributes; only intrinsics may have the `ImmArg` Attribute. The verifier
    // complains loudly otherwise, and then comes into our houses at night, and
    // wrecks up the place...
    for (unsigned i = 0, n = fnAttrs.getNumAttrSets(); i < n; ++i) {
      fnAttrs = fnAttrs.removeAttributeAtIndex(ctx, i, Attribute::ImmArg);
    }
  } else {
    // We are using the called Value instead of F because it might contain
    // a bitcast or something, which makes the function types different.
    CallInst *c = multi_llvm::createCallInst(CI->getCalledOperand(), FTy,
                                             CIArgs, "", activeBlock);
    c->setCallingConv(cc);
    c->setAttributes(callAttrs);
    PhiOperands.push_back({c, activeBlock});
    BranchInst::Create(mergeBlock, activeBlock);
  }
  newFunction->setCallingConv(cc);
  newFunction->setAttributes(fnAttrs);

  // Get the last argument (the mask) and use it as our branch predicate as to
  // the live blocks or a no-op
  Value *mask = newFunction->arg_end() - 1;
  BranchInst::Create(activeBlock, mergeBlock, mask, entryBlock);

  Type *returnTy = F->getReturnType();
  if (returnTy != Type::getVoidTy(ctx)) {
    PHINode *result = PHINode::Create(returnTy, 2, "", mergeBlock);
    for (auto &phiOp : PhiOperands) {
      result->addIncoming(phiOp.first, phiOp.second);
    }
    result->addIncoming(getDefaultValue(returnTy), entryBlock);
    ReturnInst::Create(ctx, result, mergeBlock);
  } else {
    ReturnInst::Create(ctx, mergeBlock);
  }

  MaskedVersions.insert(std::make_pair(newFName, newFunction));
  insertMaskedFunction(newFunction, F);
  return newFunction;
}

namespace {
Optional<std::tuple<bool, multi_llvm::RecurKind, bool>> isSubgroupScan(
    StringRef fnName, Type *const ty) {
  compiler::utils::Lexer L(fnName);
  if (!L.Consume(VectorizationContext::InternalBuiltinPrefix)) {
    return None;
  }
  if (!L.Consume("sub_group_scan_")) {
    return None;
  }
  bool isInt = ty->isIntOrIntVectorTy();
  bool isInclusive = L.Consume("inclusive_");
  if (isInclusive || L.Consume("exclusive_")) {
    StringRef OpKind;
    if (L.ConsumeAlpha(OpKind)) {
      multi_llvm::RecurKind opKind;
      if (OpKind == "add") {
        opKind =
            isInt ? multi_llvm::RecurKind::Add : multi_llvm::RecurKind::FAdd;
      } else if (OpKind == "min") {
        assert(!isInt && "unexpected internal scan builtin");
        opKind = multi_llvm::RecurKind::FMin;
      } else if (OpKind == "max") {
        assert(!isInt && "unexpected internal scan builtin");
        opKind = multi_llvm::RecurKind::FMax;
      } else if (OpKind == "smin") {
        opKind = multi_llvm::RecurKind::SMin;
      } else if (OpKind == "smax") {
        opKind = multi_llvm::RecurKind::SMax;
      } else if (OpKind == "umin") {
        opKind = multi_llvm::RecurKind::UMin;
      } else if (OpKind == "umax") {
        opKind = multi_llvm::RecurKind::UMax;
      } else {
        return None;
      }
      bool isVP = L.Consume("_vp");
      return std::make_tuple(isInclusive, opKind, isVP);
    }
  }
  return None;
}
};  // namespace

bool VectorizationContext::defineInternalBuiltin(Function *F) {
  assert(F->isDeclaration() && "builtin is already defined");

  // Handle masked memory loads and stores.
  if (Optional<MemOpDesc> Desc = MemOpDesc::analyzeMemOpFunction(*F)) {
    if (Desc->isMaskedMemOp()) {
      return emitMaskedMemOpBody(*F, *Desc);
    }

    // Handle interleaved memory loads and stores.
    if (Desc->isInterleavedMemOp()) {
      return emitInterleavedMemOpBody(*F, *Desc);
    }

    // Handle masked interleaved memory loads and stores
    if (Desc->isMaskedInterleavedMemOp()) {
      return emitMaskedInterleavedMemOpBody(*F, *Desc);
    }

    // Handle scatter stores and gather loads.
    if (Desc->isScatterGatherMemOp()) {
      return emitScatterGatherMemOpBody(*F, *Desc);
    }

    // Handle masked scatter stores and gather loads.
    if (Desc->isMaskedScatterGatherMemOp()) {
      return emitMaskedScatterGatherMemOpBody(*F, *Desc);
    }
  }

  // Handle subgroup scan operations.
  if (auto scanInfo = isSubgroupScan(F->getName(), F->getReturnType())) {
    bool isInclusive = std::get<0>(*scanInfo);
    multi_llvm::RecurKind opKind = std::get<1>(*scanInfo);
    bool isVP = std::get<2>(*scanInfo);
    return emitSubgroupScanBody(*F, isInclusive, opKind, isVP);
  }

  return false;
}

bool VectorizationContext::emitMaskedMemOpBody(Function &F,
                                               MemOpDesc const &Desc) const {
  Value *Data = Desc.getDataOperand(&F);
  Value *Ptr = Desc.getPointerOperand(&F);
  Value *Mask = Desc.getMaskOperand(&F);
  Value *VL = Desc.isVLOp() ? Desc.getVLOperand(&F) : nullptr;
  Type *DataTy = Desc.isLoad() ? F.getReturnType() : Data->getType();
  assert(multi_llvm::isOpaqueOrPointeeTypeMatches(
      cast<PointerType>(Ptr->getType()), DataTy));

  BasicBlock *Entry = BasicBlock::Create(F.getContext(), "entry", &F);
  IRBuilder<> B(Entry);
  Value *Result = nullptr;
  if (Desc.isLoad()) {
    Result =
        VTI.createMaskedLoad(B, DataTy, Ptr, Mask, VL, Desc.getAlignment());
    B.CreateRet(Result);
  } else {
    Result = VTI.createMaskedStore(B, Data, Ptr, Mask, VL, Desc.getAlignment());
    B.CreateRetVoid();
  }
  VECZ_FAIL_IF(!Result);
  return true;
}

bool VectorizationContext::emitInterleavedMemOpBody(
    Function &F, MemOpDesc const &Desc) const {
  return emitMaskedInterleavedMemOpBody(F, Desc);
}

bool VectorizationContext::emitMaskedInterleavedMemOpBody(
    Function &F, MemOpDesc const &Desc) const {
  Value *Data = Desc.getDataOperand(&F);
  auto *const Ptr = Desc.getPointerOperand(&F);
  VECZ_FAIL_IF(!isa<VectorType>(Desc.getDataType()) || !Ptr);

  auto *const Mask = Desc.getMaskOperand(&F);
  auto *const VL = Desc.isVLOp() ? Desc.getVLOperand(&F) : nullptr;
  auto const Align = Desc.getAlignment();
  auto const Stride = Desc.getStride();

  BasicBlock *Entry = BasicBlock::Create(F.getContext(), "entry", &F);
  IRBuilder<> B(Entry);

  // If the mask is missing, assume that this is a normal interleaved memop that
  // we want to emit as an unmasked interleaved memop
  if (Desc.isLoad()) {
    auto *const Result =
        Mask ? VTI.createMaskedInterleavedLoad(B, F.getReturnType(), Ptr, Mask,
                                               Stride, VL, Align)
             : VTI.createInterleavedLoad(B, F.getReturnType(), Ptr, Stride, VL,
                                         Align);
    VECZ_FAIL_IF(!Result);
    B.CreateRet(Result);
  } else {
    auto *const Result =
        Mask ? VTI.createMaskedInterleavedStore(B, Data, Ptr, Mask, Stride, VL,
                                                Align)
             : VTI.createInterleavedStore(B, Data, Ptr, Stride, VL, Align);
    VECZ_FAIL_IF(!Result);
    B.CreateRetVoid();
  }
  return true;
}

bool VectorizationContext::emitScatterGatherMemOpBody(
    Function &F, MemOpDesc const &Desc) const {
  return emitMaskedScatterGatherMemOpBody(F, Desc);
}

bool VectorizationContext::emitMaskedScatterGatherMemOpBody(
    Function &F, MemOpDesc const &Desc) const {
  Value *Data = Desc.getDataOperand(&F);
  auto *const VecDataTy = dyn_cast<VectorType>(Desc.getDataType());
  auto *const Ptr = Desc.getPointerOperand(&F);
  VECZ_FAIL_IF(!VecDataTy || !Ptr);

  auto *const Mask = Desc.getMaskOperand(&F);
  auto *const VL = Desc.isVLOp() ? Desc.getVLOperand(&F) : nullptr;
  auto const Align = Desc.getAlignment();

  BasicBlock *Entry = BasicBlock::Create(F.getContext(), "entry", &F);
  IRBuilder<> B(Entry);

  // If the mask is missing, assume that this is a normal scatter/gather memop
  // that we want to emit as an unmasked scatter/gather memop
  if (Desc.isLoad()) {
    auto *const Result =
        Mask ? VTI.createMaskedGatherLoad(B, VecDataTy, Ptr, Mask, VL, Align)
             : VTI.createGatherLoad(B, VecDataTy, Ptr, VL, Align);
    VECZ_FAIL_IF(!Result);
    B.CreateRet(Result);
  } else {
    auto *const Result =
        Mask ? VTI.createMaskedScatterStore(B, Data, Ptr, Mask, VL, Align)
             : VTI.createScatterStore(B, Data, Ptr, VL, Align);
    VECZ_FAIL_IF(!Result);
    B.CreateRetVoid();
  }
  return true;
}

// Emit a subgroup scan operation.
// If the vectorization factor is fixed, we can do a scan in log2(N) steps,
// by noting that an inclusive scan can be split into two, and recombined into
// a single result by adding the last element of the first half onto every
// element of the second half. To deal with exclusive scans, we rotate the
// result by one element and insert the neutral element at the beginning.
//
// For now, when using scalable vectorization factor, this takes the form of a
// simple loop that accumulates the scan operation in scalar form, extracting
// and inserting elements of the resulting vector on each iteration:
//   %v = <A,B,C,D>
//   Iteration 0:
//     %e.0 = extractelement %v, 0          (A)
//     %s.0 = add N, %e.0                   (A)
//     %v.0 = insertelement undef, %s.0, 0  (<A,U,U,U>)
//   Iteration 1:
//     %e.1 = extractelement %v, 1          (B)
//     %s.1 = add %s.0, %e.1                (A+B)
//     %v.1 = insertelement  %v.0, %s.1, 1  (<A,A+B,U,U>)
//   Iteration 2:
//     %e.2 = extractelement %v, 2          (C)
//     %s.2 = add %s.1, %e.2                (A+B+C)
//     %v.2 = insertelement  %v.1, %s.2, 2  (<A,A+B,A+B+C,U>)
//   Iteration 3:
//     %e.3 = extractelement %v, 3          (D)
//     %s.3 = add %s.2, %e.2                (A+B+C+D)
//     %v.3 = insertelement  %v.2, %s.3, 3  (<A,A+B,A+B+C,A+B+C+D>)
//   Result:
//     %v.3 = <A,A+B,A+B+C,A+B+C+D>
//
// Exclusive scans operate by pre-filling the vector with the neutral value,
// looping from 1 onwards, and extracting from one less than the current
// iteration:
//   %z = insertelement undef, N, 0
//   Iteration 0:
//     %e.0 = extractelement %v, 0          (A)
//     %s.0 = add N, %e.0                   (A)
//     %v.0 = insertelement %z, %s.0, 1     (<N,A,U,U>)
// This loop operates up to the VL input, if it is a vector-predicated scan.
// Elements past the vector length will receive a default zero value.
// Note: This method is not optimal for fixed-length code, but serves as a way
// of producing scalable- and fixed-length vector code equivalently.
bool VectorizationContext::emitSubgroupScanBody(Function &F, bool IsInclusive,
                                                multi_llvm::RecurKind OpKind,
                                                bool IsVP) const {
  LLVMContext &Ctx = F.getContext();

  auto *const Entry = BasicBlock::Create(Ctx, "entry", &F);
  IRBuilder<> B(Entry);

  Type *const VecTy = F.getReturnType();
  Type *const EltTy = multi_llvm::getVectorElementType(VecTy);
  ElementCount EC = multi_llvm::getVectorElementCount(VecTy);

  Function::arg_iterator Arg = F.arg_begin();

  Value *const Vec = Arg;
  Value *const VL = IsVP ? ++Arg : nullptr;

  // If it's not a scalable vector, we can do it the fast way.
  if (!EC.isScalable() && !IsVP) {
    auto *const NeutralVal = compiler::utils::getNeutralVal(OpKind, EltTy);
    auto const Width = EC.getFixedValue();
    auto *const UndefVal = UndefValue::get(VecTy);

    // Put the Neutral element in a vector so we can shuffle it in.
    auto *const NeutralVec =
        B.CreateInsertElement(UndefVal, NeutralVal, B.getInt64(0));

    auto *Result = Vec;
    unsigned N = 1u;

    SmallVector<int, 16> mask(Width);
    while (N < Width) {
      // Build shuffle mask.
      // The sequence of masks will be, for a width of 16
      // (in hexadecimal for concision, where x represents the neutral value
      // element):
      //
      // x0x2x4x6x8xAxCxE
      // xx11xx55xx99xxDD
      // xxxx3333xxxxBBBB
      // xxxxxxxx77777777
      //
      auto const N2 = N << 1u;
      auto MaskIt = mask.begin();
      for (size_t i = 0; i < Width; i += N2) {
        for (size_t j = 0; j < N; ++j) {
          *MaskIt++ = Width;
        }

        auto const k = i + N - 1;
        for (size_t j = 0; j < N; ++j) {
          *MaskIt++ = k;
        }
      }
      N = N2;
      auto *const Shuffle =
          createOptimalShuffle(B, Result, NeutralVec, mask, Twine("scan_impl"));
      Result = multi_llvm::createBinOpForRecurKind(B, Result, Shuffle, OpKind);
    }

    if (!IsInclusive) {
      // If it is an exclusive scan, rotate the result.
      auto *const IdentityVal = compiler::utils::getIdentityVal(OpKind, EltTy);
      VECZ_FAIL_IF(!IdentityVal);
      Result = VTI.createVectorSlideUp(B, Result, IdentityVal, VL);
    }

    B.CreateRet(Result);
    return true;
  }

  // If the vector is scalable, we don't know the number of iterations required,
  // so we have to use a loop and shuffle masks generated from the step vector.

  auto *const IVTy = B.getInt32Ty();
  auto *const IndexTy = VectorType::get(IVTy, EC);
  auto *const Step = B.CreateStepVector(IndexTy, "step");
  auto *const VZero = Constant::getNullValue(IndexTy);

  auto *const Loop = BasicBlock::Create(Ctx, "loop", &F);
  auto *const Exit = BasicBlock::Create(Ctx, "exit", &F);

  // The length of the vector.
  Value *Width = nullptr;
  if (IsVP) {
    Width = VL;
  } else if (EC.isScalable()) {
    Width = B.CreateVScale(ConstantInt::get(IVTy, EC.getKnownMinValue()));
  } else {
    Width = ConstantInt::get(IVTy, EC.getFixedValue());
  }

  B.CreateBr(Loop);

  // Loop induction starts at 1 and doubles each time.
  auto *const IVStart = ConstantInt::get(IVTy, 1);

  // Create the loop instructions
  B.SetInsertPoint(Loop);

  // The induction variable (IV) which determines both our loop bounds and our
  // vector indices.
  auto *N = B.CreatePHI(IVTy, 2, "iv");
  N->addIncoming(IVStart, Entry);

  // A vector phi representing the vectorized value we're building up.
  auto *VecPhi = B.CreatePHI(VecTy, 2, "vec");
  VecPhi->addIncoming(Vec, Entry);

  // A vector phi representing the vectorized value we're building up.
  auto *MaskPhi = B.CreatePHI(IndexTy, 2, "mask.phi");
  MaskPhi->addIncoming(Step, Entry);

  // This will create shuffle masks like the following sequence:
  //
  // 1032547698BADCFE = (0123456789ABCDEF ^ splat(1))
  // 33117755BB99FFDD = (1032547698BADCFE ^ splat(2)) | splat(1)
  // 77773333FFFFBBBB = (33117755BB99FFDD ^ splat(4)) | splat(2)
  // FFFFFFFF77777777 = (77773333FFFFBBBB ^ splat(8)) | splat(4)
  //
  // We don't mix the neutral element into the vector in this case, but use a
  // Select instruction to choose between the updated or original value, so that
  // backends can lower it as a masked binary operation. The select condition
  // therefore needs to be like the following sequence:
  //
  // 0101010101010101
  // 0011001100110011
  // 0000111100001111
  // 0000000011111111

  auto *const SplatN = B.CreateVectorSplat(EC, N, "splatN");
  auto *const Mask = B.CreateXor(MaskPhi, SplatN, "mask");
  auto *const Shuffle = VTI.createVectorShuffle(B, VecPhi, Mask, VL);
  auto *const Accum =
      multi_llvm::createBinOpForRecurKind(B, VecPhi, Shuffle, OpKind);

  auto *const NBit = B.CreateAnd(MaskPhi, SplatN, "isolate");
  auto *const Which = B.CreateICmpNE(NBit, VZero, "which");
  auto *const NewVec = B.CreateSelect(Which, Accum, VecPhi, "newvec");

  auto *const NewMask = B.CreateOr(Mask, SplatN, "newmask");
  auto *const N2 = B.CreateShl(N, ConstantInt::get(IVTy, 1), "N2",
                               /*HasNUW*/ true, /*HasNSW*/ true);

  VecPhi->addIncoming(NewVec, Loop);
  MaskPhi->addIncoming(NewMask, Loop);
  N->addIncoming(N2, Loop);

  // Loop exit condition
  auto *const Cond = B.CreateICmpULT(N2, Width, "iv.cmp");
  B.CreateCondBr(Cond, Loop, Exit);

  // Function exit instructions:
  B.SetInsertPoint(Exit);

  // Create an LCSSA PHI node.
  auto *const ResultPhi = B.CreatePHI(VecTy, 1, "res.phi");
  ResultPhi->addIncoming(NewVec, Loop);

  Value *Result = ResultPhi;
  if (!IsInclusive) {
    // If it is an exclusive scan, rotate the result.
    auto *const IdentityVal = compiler::utils::getIdentityVal(OpKind, EltTy);
    VECZ_FAIL_IF(!IdentityVal);
    Result = VTI.createVectorSlideUp(B, Result, IdentityVal, VL);
  }

  B.CreateRet(Result);
  return true;
}

Function *VectorizationContext::getInternalVectorEquivalent(
    Function *ScalarFn, unsigned SimdWidth) {
  // Handle masked memory loads and stores.
  if (!ScalarFn) {
    return nullptr;
  }
  if (auto Desc = MemOpDesc::analyzeMaskedMemOp(*ScalarFn)) {
    auto *NewDataTy =
        multi_llvm::FixedVectorType::get(Desc->getDataType(), SimdWidth);
    return getOrCreateMaskedMemOpFn(
        *this, NewDataTy, cast<PointerType>(Desc->getPointerType()),
        Desc->getAlignment(), Desc->isLoad(), Desc->isVLOp());
  }

  return nullptr;
}

bool VectorizationContext::isMaskedFunction(const llvm::Function *F) const {
  return MaskedFunctionsMap.count(F) > 0;
}

bool VectorizationContext::insertMaskedFunction(llvm::Function *F,
                                                llvm::Function *WrappedF) {
  auto result = MaskedFunctionsMap.insert({F, WrappedF});
  return result.second;
}

llvm::Function *VectorizationContext::getOriginalMaskedFunction(
    llvm::Function *F) {
  auto Iter = MaskedFunctionsMap.find(F);
  if (Iter != MaskedFunctionsMap.end()) {
    return dyn_cast_or_null<llvm::Function>(Iter->second);
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

char DefineInternalBuiltinsPass::PassID = 0;

PreservedAnalyses DefineInternalBuiltinsPass::run(Module &M,
                                                  ModuleAnalysisManager &AM) {
  llvm::FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  // Remove internal builtins that may not be needed any more.
  SmallVector<Function *, 4> ToRemove;

  bool NonePreserved = false;
  // Implement internal builtins that we now know are needed.
  // We find all declarations that should be builtins, and then define them if
  // they have users that have associated vectorization units.
  // On failure to define, we notify those vectorization units of failure
  // and remove any partially defined body.
  // Unused declarations are removed
  for (Function &F : M.functions()) {
    if (!F.isDeclaration() || !VectorizationContext::isInternalBuiltin(&F)) {
      continue;
    }
    if (F.use_empty()) {
      ToRemove.push_back(&F);
      NonePreserved = true;
      continue;
    }
    llvm::SmallPtrSet<VectorizationUnit *, 1> UserVUs;
    for (Use &U : F.uses()) {
      if (CallInst *CI = dyn_cast<CallInst>(U.getUser())) {
        auto R = FAM.getResult<VectorizationUnitAnalysis>(*CI->getFunction());
        if (R.hasResult()) {
          UserVUs.insert(&R.getVU());
        }
      }
    }
    if (std::all_of(UserVUs.begin(), UserVUs.end(),
                    [](VectorizationUnit *VU) { return VU->failed(); })) {
      // If the vectorization has failed, we do not want to define the internal
      // builtins, both because its a waste of time and because we might try to
      // instantiate some invalid builtin that would have been replaced by the
      // packetization process.
      continue;
    }

    VectorizationContext &Ctx = (*UserVUs.begin())->context();
    bool DefinedBuiltin = Ctx.defineInternalBuiltin(&F);
    if (!DefinedBuiltin) {
      // If we've failed to define this builtin, ensure we clean up the
      // half-complete body. We can't simply delete it because it will have
      // uses in the vector kernel. This will revert it to a declaration, which
      // will be cleaned up later by the global optimizer.
      if (!F.isDeclaration()) {
        // defineInternalBuiltin may have partially defined the function body.
        // Clean it up. FIXME defineInternalBuiltin should probably clean up
        // after itself if there is a failure condition
        F.deleteBody();
      }
      for (VectorizationUnit *VU : UserVUs) {
        VU->setFailed("failed to define an internal builtin");
      }
      continue;
    }
    NonePreserved = true;
  }

  for (Function *F : ToRemove) {
    F->eraseFromParent();
  }

  return NonePreserved ? PreservedAnalyses::none() : PreservedAnalyses::all();
}