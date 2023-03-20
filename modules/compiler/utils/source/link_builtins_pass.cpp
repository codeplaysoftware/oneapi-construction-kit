// Copyright (C) Codeplay Software Limited. All Rights Reserved.

// This pass will manually link in any functions required from a given
// 'builtins' module, into the current module. It exists for a few reasons:
// * LLVM's LinkModules is destructive to the source module - it will happily
//   destroy the source module as it links it into the destination. This is
//   fine for most cases, but not ours. In our case, we want to load the
//   builtins module once (in our finalizer) and then re-use that loaded module
//   multiple times (saves significant memory & processing requirements on our
//   hot path).
// * We can strip out unnecessary symbols as we perform our link step - meaning
//   we can do what amounts to a simple global DCE pass for free.
// * We can run our link step as an LLVM pass. Previously, we would link our
//   kernel module into the lazily loaded builtins module (the recommended way
//   to link between a small and a very large LLVM module), which we would not
//   be able to do in a pass (as the Module the pass refers to effectively dies
//   as the linking would occur).

#include <compiler/utils/StructTypeRemapper.h>
#include <compiler/utils/builtin_info.h>
#include <compiler/utils/link_builtins_pass.h>
#include <compiler/utils/mangling.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Error.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Utils/ValueMapper.h>
#include <multi_llvm/multi_llvm.h>

using namespace llvm;

namespace {

// This is a list of sub-group builtins which vecz handles specially. Linking
// builtins before vecz would result in them being inlined which would break
// sub-groups. For example:
//   int sub_group_all(int predicate) { return predicate; }
// FIXME: Could these builtins be lowered to declare-only mux equivalents to
// make them inline-safe at any stage? See CA-4679.
const StringSet<> DeferredBuiltins = {
    "get_sub_group_local_id",
    "get_sub_group_size",
    "sub_group_all",
    "sub_group_any",
    "sub_group_reduce_add",
    "sub_group_reduce_min",
    "sub_group_reduce_max",
    "sub_group_broadcast",
    "sub_group_scan_exclusive_add",
    "sub_group_scan_exclusive_min",
    "sub_group_scan_exclusive_max",
    "sub_group_scan_inclusive_add",
    "sub_group_scan_inclusive_min",
    "sub_group_scan_inclusive_max",
};

bool isDeferredBuiltin(Function &F) {
  compiler::utils::NameMangler Mangler(&F.getContext(), F.getParent());

  SmallVector<Type *, 4> Types;
  SmallVector<compiler::utils::TypeQualifiers, 4> Quals;
  StringRef BaseName = Mangler.demangleName(F.getName(), Types, Quals);
  return DeferredBuiltins.find(BaseName) != DeferredBuiltins.end();
}

class GlobalVarMaterializer final : public ValueMaterializer {
 public:
  GlobalVarMaterializer(Module &M) : M(M) {}

  /// @brief List of variables created during materialization.
  const ArrayRef<GlobalVariable *> getGlobalVariables() const {
    return GlobalVars;
  }

  /// @brief Materialize the given value.
  ///
  /// @param[in] value - Value to materialize.
  ///
  /// @return A value that lives in the destination module, or nullptr if the
  /// given value could not be materialized (e.g. it is not a global variable).
  virtual Value *materialize(Value *value) override final {
    auto GV = dyn_cast<GlobalVariable>(value);
    if (!GV) {
      return nullptr;
    }

    auto *NewGV = M.getGlobalVariable(GV->getName());

    if (!NewGV) {
      NewGV = new GlobalVariable(M, GV->getValueType(), GV->isConstant(),
                                 GV->getLinkage(), nullptr, GV->getName(),
                                 nullptr, GV->getThreadLocalMode(),
                                 GV->getType()->getAddressSpace());
      NewGV->copyAttributesFrom(GV);
      GlobalVars.push_back(GV);
    }

    return NewGV;
  }

 private:
  Module &M;
  SmallVector<GlobalVariable *, 8> GlobalVars;
};
}  // namespace

/// @brief If we have the same structs in the main module and builtins with
/// different names, copy the body
/// @param [in] M LLVM Module
/// @param [out] Map A structure mapping from builtin structure types to the
/// corresponding module types
void compiler::utils::LinkBuiltinsPass::cloneStructs(
    Module &M, Module &BuiltinsModule, compiler::utils::StructMap &Map) {
  for (auto *StructTy : M.getIdentifiedStructTypes()) {
    auto StructName = StructTy->getName();

    for (auto *BuiltinStructTy : BuiltinsModule.getIdentifiedStructTypes()) {
      auto BuiltinStructName = BuiltinStructTy->getName();

      const char *Suffix = ".0123456789";

      // check if the names match (minus the suffix LLVM sometimes adds to
      // struct types to differentiate between them)
      if (StructName.rtrim(Suffix).equals(BuiltinStructName.rtrim(Suffix))) {
        if (StructTy->isOpaque() && !BuiltinStructTy->isOpaque()) {
          StructTy->setBody(BuiltinStructTy->elements(),
                            BuiltinStructTy->isPacked());
        }

        Map[BuiltinStructTy] = StructTy;
      }
    }
  }
}

void compiler::utils::LinkBuiltinsPass::cloneBuiltins(
    Module &M, Module &BuiltinsModule,
    SmallVectorImpl<Function *> &BuiltinFnDecls,
    compiler::utils::StructTypeRemapper *StructMap) {
  // Clone the builtin and its callees.
  DenseSet<Function *> Callees;

  while (!BuiltinFnDecls.empty()) {
    Function *const BuiltinFn = BuiltinFnDecls.pop_back_val();

    // if we are already tracking the callee, we can skip the function
    if (!Callees.insert(BuiltinFn).second) {
      continue;
    }

    auto Error = BuiltinsModule.materialize(BuiltinFn);

    assert(!Error && "Bitcode materialization failed!");

    // Find any callees in the function and add them to the list.
    for (auto &BB : *BuiltinFn) {
      for (auto &I : BB) {
        // if we have a call instruction
        if (auto *const CI = dyn_cast<CallInst>(&I)) {
          // and the called function is known
          if (Function *Callee = CI->getCalledFunction()) {
            // Only process the builtin if we are not doing early builtins
            // or it's not one of the deferred ones
            if (!EarlyLinking || !isDeferredBuiltin(*Callee)) {
              // we need to clone the function
              BuiltinFnDecls.push_back(Callee);
            }
          }
        }
      }
    }
  }

  // Copy builtin and callees to the target module
  ValueToValueMapTy ValueMap;
  SmallVector<ReturnInst *, 4> Returns;
  // Avoid linking errors.
  const auto DefaultLinkage = GlobalValue::LinkOnceAnyLinkage;

  // Declare the callees in the module if they don't already exist.
  for (Function *Callee : Callees) {
    const auto Linkage = Callee->isIntrinsic() || Callee->isDeclaration()
                             ? Callee->getLinkage()
                             : DefaultLinkage;

    auto *NewCallee = M.getFunction(Callee->getName());
    if (!NewCallee) {
      auto *FnTy = Callee->getFunctionType();
      if (StructMap) {
        // We need to remap any image types for function arguments
        auto *RetTy = StructMap->remapType(FnTy->getReturnType());
        SmallVector<Type *, 4> ParamTys;
        for (auto *ParamTy : FnTy->params()) {
          ParamTys.push_back(StructMap->remapType(ParamTy));
        }
        FnTy = FunctionType::get(RetTy, ParamTys, FnTy->isVarArg());
      }

      NewCallee = Function::Create(FnTy, Linkage, Callee->getName(), &M);
      NewCallee->setCallingConv(Callee->getCallingConv());
    } else {
      NewCallee->setLinkage(Linkage);
    }

    auto NewArgIterator = NewCallee->arg_begin();
    for (Argument &Arg : Callee->args()) {
      NewArgIterator->setName(Arg.getName());
      ValueMap[&Arg] = &*(NewArgIterator++);
    }

    NewCallee->copyAttributesFrom(Callee);
    ValueMap[Callee] = NewCallee;
  }

  // Clone the callees' bodies into the module.
  GlobalVarMaterializer GVMaterializer(M);

  for (Function *Callee : Callees) {
    // if the function isn't an intrinsic or a declaration
    if (!Callee->isIntrinsic() && !Callee->isDeclaration()) {
      auto NewCallee = cast<Function>(ValueMap[Callee]);
      auto Changes =
          NewCallee->getParent() != Callee->getParent()
              ? multi_llvm::CloneFunctionChangeType::DifferentModule
              : multi_llvm::CloneFunctionChangeType::LocalChangesOnly;
      multi_llvm::CloneFunctionInto(NewCallee, Callee, ValueMap, Changes,
                                    Returns, "", nullptr, StructMap,
                                    &GVMaterializer);
      Returns.clear();
    }
  }

  // Clone global variable initializers.
  for (auto *Var : GVMaterializer.getGlobalVariables()) {
    auto *NewVar = cast<GlobalVariable>(ValueMap[Var]);
    Constant *OldInit = Var->getInitializer();
    Constant *NewInit = MapValue(OldInit, ValueMap);
    NewVar->setInitializer(NewInit);
  }
}

PreservedAnalyses compiler::utils::LinkBuiltinsPass::run(
    Module &M, ModuleAnalysisManager &MAM) {
  auto &BI = MAM.getResult<BuiltinInfoAnalysis>(M);

  auto *BuiltinsModule = BI.getBuiltinsModule();
  // if we don't actually have a builtins module
  if (nullptr == BuiltinsModule) {
    return PreservedAnalyses::all();
  }

  SmallVector<Function *, 8> BuiltinFnDecls;

  for (auto &F : M.functions()) {
    auto BuiltinF = BuiltinsModule->getFunction(F.getName());
    if (F.isDeclaration() && nullptr != BuiltinF) {
      // Only process the builtin if we are not doing early builtins
      // or it's not one of the deferred ones
      if (!EarlyLinking || !isDeferredBuiltin(*BuiltinF)) {
        // we need to clone the function
        BuiltinFnDecls.push_back(BuiltinF);
      }
    }
  }

  if (BuiltinFnDecls.empty()) {
    return PreservedAnalyses::all();
  }

  StructMap Map;
  cloneStructs(M, *BuiltinsModule, Map);
  StructTypeRemapper structMap(Map);
  cloneBuiltins(M, *BuiltinsModule, BuiltinFnDecls,
                Map.empty() ? nullptr : &structMap);

  return PreservedAnalyses::none();
}