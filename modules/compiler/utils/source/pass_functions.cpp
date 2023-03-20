// Copyright (C) Codeplay Software Limited. All Rights Reserved.

#include <compiler/utils/address_spaces.h>
#include <compiler/utils/attributes.h>
#include <compiler/utils/builtin_info.h>
#include <compiler/utils/device_info.h>
#include <compiler/utils/metadata.h>
#include <compiler/utils/pass_functions.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <multi_llvm/multi_llvm.h>
#include <multi_llvm/vector_type_helper.h>

#include <cassert>

llvm::AnalysisKey compiler::utils::DeviceInfoAnalysis::Key;

namespace compiler {
namespace utils {

uint64_t computeApproximatePrivateMemoryUsage(const llvm::Function &fn) {
  const llvm::Module *module = fn.getParent();
  const auto &layout = module->getDataLayout();
  uint64_t bytes = 0;

  // BarrierPass asserts that `allocas` only exist in the entry block
  for (auto &inst : fn.getEntryBlock()) {
    if (!llvm::isa<llvm::AllocaInst>(inst)) {
      continue;
    }
    const auto &alloca_inst = llvm::cast<llvm::AllocaInst>(inst);
    auto const *type = alloca_inst.getType();
    if (type->getAddressSpace() != AddressSpace::Private) {
      continue;
    }
    auto *alloc_type = alloca_inst.getAllocatedType();
    const auto alloc_size = layout.getTypeAllocSize(alloc_type);
    if (alloca_inst.isArrayAllocation()) {
      auto *arr_size_val = alloca_inst.getArraySize();
      auto *const_int = llvm::dyn_cast<llvm::ConstantInt>(arr_size_val);
      assert(const_int != nullptr &&
             "OpenCL or Vulkan Array Allocation of dynamic size");
      const uint64_t arr_size = const_int->getUniqueInteger().getLimitedValue();
      bytes += arr_size * alloc_size;

    } else {
      bytes += alloc_size;
    }
  }
  return bytes;
}

void remapConstantExpr(llvm::ConstantExpr *expr, llvm::Constant *from,
                       llvm::Constant *to) {
  llvm::SmallVector<llvm::Constant *, 8> newOps;
  // iterate through the constant expression and create a vector of old and new
  // ones
  for (unsigned i = 0, e = expr->getNumOperands(); i != e; ++i) {
    auto op = expr->getOperand(i);
    if (op == from) {
      newOps.push_back(to);
    } else {
      newOps.push_back(op);
    }
  }

  // Create a new expression with the list of operands and replace all uses with
  llvm::Constant *newConstant = expr->getWithOperands(newOps);
  expr->replaceAllUsesWith(newConstant);
}

bool funcContainsDebugMetadata(const llvm::Function &func,
                               llvm::ValueToValueMapTy &vmap) {
  // Check if function references debug info
  bool foundDI = false;

  // Function has a DISubprogram entry attached
  if (auto DISubprogram = func.getSubprogram()) {
    vmap.MD()[DISubprogram].reset(DISubprogram);
    foundDI = true;
  }

  for (auto &BB : func) {
    for (auto &Inst : BB) {
      if (auto DL = Inst.getDebugLoc()) {
        llvm::DILocation *loc = DL.get();
        vmap.MD()[loc].reset(loc);
        foundDI = true;
      }

      if (auto DebugIntrinsic = llvm::dyn_cast<llvm::DbgInfoIntrinsic>(&Inst)) {
        llvm::DILocalVariable *DIVar = nullptr;
        if (auto DbgVarIntrinsic =
                llvm::dyn_cast<llvm::DbgVariableIntrinsic>(DebugIntrinsic)) {
          DIVar = DbgVarIntrinsic->getVariable();
        } else {
          continue;  // TODO CA-1115 - we don't handle DbgLabelInsts yet
        }
        if (DIVar) {
          vmap.MD()[DIVar].reset(DIVar);
          auto varLoc = DIVar->getScope();
          vmap.MD()[varLoc].reset(varLoc);
          foundDI = true;
        }
      }
    }
  }

  return foundDI;
}

void replaceConstantExpressionWithInstruction(llvm::Constant *const constant) {
  // remove all dead constant users (sometimes these are left over by previous
  // passes)
  constant->removeDeadConstantUsers();

  // Only handle constants which are ConstantExpr or ConstantVector
  assert((llvm::isa<llvm::ConstantExpr>(constant) ||
          llvm::isa<llvm::ConstantVector>(constant)) &&
         "Unsupported constant type in IR");

  // For each user of a constant we will check to see if they in turn are
  // constants. If they are convert them to instructions first (still
  // referencing this constant). We can are then clear to convert the current
  // constant to an instruction as the only users left are instructions.

  llvm::SmallVector<llvm::User *, 8> users;
  // Create the list of users of this constant. We don't want duplicates here,
  // which often happens with ConstantVectors, as we only want convert them to
  // an instruction once. We want determinism here so use a vector to maintain
  // order.
  for (auto *constantUser : constant->users()) {
    if (std::find(users.begin(), users.end(), constantUser) == users.end()) {
      users.push_back(constantUser);
    }
  }

  for (auto *constantUser : users) {
    if (llvm::isa<llvm::Instruction>(constantUser)) {
      // instructions are our best case, do nothing!
    } else if (llvm::Constant *subConstant =
                   llvm::dyn_cast<llvm::Constant>(constantUser)) {
      replaceConstantExpressionWithInstruction(subConstant);
    } else {
      constantUser->print(llvm::errs());
      llvm_unreachable("Constant user is not a constant or instruction!!");
    }
  }

  // we record each use
  llvm::SmallVector<llvm::Use *, 8> uses;

  for (auto &use : constant->uses()) {
    uses.push_back(&use);
  }

  for (auto *use : uses) {
    // get the instruction that is the user of the use
    auto inst = llvm::cast<llvm::Instruction>(use->getUser());

    // get the function for this use
    auto useFunc = inst->getFunction();

    llvm::Instruction *newInst = nullptr;
    // create a new instruction that matches the constant expression
    if (llvm::ConstantExpr *constantExpr =
            llvm::dyn_cast<llvm::ConstantExpr>(constant)) {
      newInst = constantExpr->getAsInstruction();
      // insert the instruction at the beginning of the entry block
      newInst->insertBefore(useFunc->getEntryBlock().getFirstNonPHI());
    } else if (llvm::ConstantVector *constantVec =
                   llvm::dyn_cast<llvm::ConstantVector>(constant)) {
      // If it is a ConstantVector then only handle the case where it is
      // a single splatted value. This is the only kind generated at present.
      auto splatVal = constantVec->getSplatValue();
      assert(splatVal &&
             "ConstantVector does not contained identical constants so cannot "
             "be splatted!");
      // Take the splatted Value and create two instructions. The first is
      // InsertElement to place it in a new vector and the second is a
      // ShuffleVector to duplicate the value across the vector.
      auto numEls = constantVec->getNumOperands();
      llvm::Value *undef = llvm::UndefValue::get(
          multi_llvm::FixedVectorType::get(splatVal->getType(), numEls));
      llvm::Type *i32Ty = llvm::Type::getInt32Ty(constant->getContext());
      auto insert = llvm::InsertElementInst::Create(
          undef, splatVal, llvm::ConstantInt::get(i32Ty, 0));
      insert->insertBefore(useFunc->getEntryBlock().getFirstNonPHI());
      llvm::Value *zeros = llvm::ConstantAggregateZero::get(
          multi_llvm::FixedVectorType::get(i32Ty, numEls));
      newInst = new llvm::ShuffleVectorInst(insert, undef, zeros);
      newInst->insertAfter(insert);
    }

    // replace the use of the constant with the instruction
    use->set(newInst);
  }

  // lastly, destroy the constant we just replaced
  constant->destroyConstant();
}

llvm::AttributeList getCopiedFunctionAttrs(const llvm::Function &oldFn,
                                           int numParams) {
  unsigned numParamsToCopy =
      numParams < 0 ? oldFn.arg_size() : (unsigned)numParams;
  llvm::SmallVector<llvm::AttributeSet, 4> newArgAttrs(numParamsToCopy);
  llvm::AttributeList oldAttrs = oldFn.getAttributes();
  // clone any argument attributes we're copying over. Note we can't simply
  // call Function::copyAttributes as not all arguments are present in the new
  // function.
  for (unsigned i = 0, e = numParamsToCopy; i != e; i++) {
    newArgAttrs[i] = oldAttrs.getParamAttrs(i);
  }

  return llvm::AttributeList::get(oldFn.getContext(), oldAttrs.getFnAttrs(),
                                  oldAttrs.getRetAttrs(), newArgAttrs);
}

void copyFunctionAttrs(const llvm::Function &oldFn, llvm::Function &newFn,
                       int numParams) {
  newFn.setAttributes(getCopiedFunctionAttrs(oldFn, numParams));
}

bool cloneFunctionsAddArg(
    llvm::Module &module,
    std::function<ParamTypeAttrsPair(llvm::Module &)> paramTypeFunc,
    std::function<void(const llvm::Function &, bool &ClonedWithBody,
                       bool &ClonedNoBody)>
        toBeCloned,
    const UpdateMDCallbackFn &updateMetaDataCallback) {
  // mapping from new -> old function
  llvm::ValueMap<llvm::Function *, llvm::Function *> newToOldMap;

  // Preserve the value map across all function clones
  llvm::ValueToValueMapTy vmap;

  ParamTypeAttrsPair const paramInfo = paramTypeFunc(module);

  // For each function we run the function toBeCloned to set the bools
  // doCloneNoBody and doCloneWithBody
  // first, run through our functions and make copies of all functions that:
  //   1) are not declarations (these will be builtins we expand later) or
  //   doCloneNoBody is set (don't clone but flesh out)
  //   2) are not new functions that we just added
  //   3) Functions marked by doCloneWithBody
  for (auto &func : module.functions()) {
    bool doCloneWithBody = false;
    bool doCloneNoBody = false;

    toBeCloned(func, doCloneWithBody, doCloneNoBody);
    bool isDecl = func.isDeclaration();
    bool processFunc = (0 == newToOldMap.count(&func));

    if (!isDecl) {
      processFunc = processFunc && doCloneWithBody;
    } else {
      processFunc = processFunc && doCloneNoBody;
    }

    if (processFunc) {
      auto funcTy = func.getFunctionType();

      unsigned numParams = funcTy->getNumParams();
      llvm::SmallVector<llvm::Type *, 8> newParamTypes(numParams + 1);

      // add each param from the original function to the new one
      for (unsigned i = 0; i < numParams; i++) {
        newParamTypes[i] = funcTy->getParamType(i);
      }
      // and lastly add our extra arg as the last param
      newParamTypes[numParams] = paramInfo.first;

      auto newFuncTy = llvm::FunctionType::get(funcTy->getReturnType(),
                                               newParamTypes, false);

      // create our new function, using the linkage from the old one
      auto newFunc =
          llvm::Function::Create(newFuncTy, func.getLinkage(), "", &module);

      // set the correct calling convention
      newFunc->setCallingConv(func.getCallingConv());

      // take the name of the old function
      newFunc->takeName(&func);

      // Copy names over for the parameters
      llvm::Function::arg_iterator DestI = newFunc->arg_begin();
      for (const auto &I : func.args()) {
        (*DestI).setName(I.getName());  // Copy the name over...
        DestI++;
      }

      if (isDecl) {
        // copy debug info for function over; CloneFunctionInto takes care of
        // this if this function has a body
        newFunc->setSubprogram(func.getSubprogram());
        // copy the metadata into the new function, ignoring any debug info.
        copyFunctionMetadata(func, *newFunc);
      } else {
        // map all original function arguments to the new function arguments
        for (auto iter = func.arg_begin(), iter_end = func.arg_end(),
                  new_iter = newFunc->arg_begin();
             iter != iter_end; ++iter, ++new_iter) {
          vmap[(&*iter)] = (&*new_iter);
        }

        llvm::SmallVector<llvm::ReturnInst *, 8> returns;

        // we have module changes if our function contains any debug info
        assert(newFunc->getParent() &&
               "assumed newFunc has an associated module");
        const bool hasDbgMetadata = funcContainsDebugMetadata(func, vmap);
        const bool differentModules = newFunc->getParent() != func.getParent();
        auto changeType =
            differentModules
                ? multi_llvm::CloneFunctionChangeType::DifferentModule
                : multi_llvm::CloneFunctionChangeType::LocalChangesOnly;
        if (hasDbgMetadata) {
          changeType = std::max(
              changeType, multi_llvm::CloneFunctionChangeType::GlobalChanges);
        }
        multi_llvm::CloneFunctionInto(newFunc, &func, vmap, changeType,
                                      returns);
      }

      // Add in the new parameter attributes here, because CloneFunctionInto
      // wipes out pre-existing attributes on newFunc which aren't in oldFunc.
      newFunc->addParamAttrs(numParams, llvm::AttrBuilder(newFunc->getContext(),
                                                          paramInfo.second));

      // map new func -> old func
      newToOldMap[newFunc] = &func;

      // remove the body of the old function that we are going to delete
      // anyway, so that none of its callsites get processed in the remainder
      // of this pass
      func.deleteBody();
    }
  }

  // next, remap all callsites that would have called the old function, to the
  // new function we just created
  for (auto pair : newToOldMap) {
    llvm::Function *newFunc = pair.first;
    llvm::Function *oldFunc = pair.second;

    remapClonedCallsites(*oldFunc, *newFunc, true);

    // next, let the caller update any metadata.
    if (updateMetaDataCallback) {
      updateMetaDataCallback(*oldFunc, *newFunc,
                             newFunc->getFunctionType()->getNumParams() - 1);
    }
  }

  // lastly, remove all the old functions we no longer need
  for (auto pair : newToOldMap) {
    // the old function, no longer used
    llvm::Function *const oldFunc = pair.second;

    // then destroy the function
    oldFunc->eraseFromParent();
  }

  return true;
}

void remapClonedCallsites(llvm::Function &oldFunc, llvm::Function &newFunc,
                          bool extraArg) {
  // list of calls we need to erase at the end
  llvm::SmallVector<llvm::CallInst *, 32> callsToErase;

  // for everything that uses our old function
  for (auto *user : oldFunc.users()) {
    // if the user calls our old function
    if (auto ci = llvm::dyn_cast<llvm::CallInst>(user)) {
      // store the name from the old call
      const std::string name = ci->getName().str();

      // get the number of args at the old callsite
      const unsigned numArgs = ci->arg_size();

      // the number of args at the new callsite. If we're adding an extra
      // argument this is incremented.
      const unsigned newNumArgs = extraArg ? numArgs + 1 : numArgs;

      // create a buffer for our args
      llvm::SmallVector<llvm::Value *, 8> args(newNumArgs);

      // set all the new call args to be the old call args
      for (unsigned i = 0; i < numArgs; i++) {
        args[i] = ci->getArgOperand(i);
      }

      // if we're adding an extra param it's always the last
      // argument, so propagate the value on from the parent
      if (extraArg) {
        llvm::Function *parentFunc = ci->getFunction();
        llvm::Argument *lastArg = getLastArgument(parentFunc);
        args[numArgs] = lastArg;
      }

      // create our new call instruction to replace the old one
      auto newCi = llvm::CallInst::Create(&newFunc, args, name, ci);

      // use the debug location from the old call (if any)
      newCi->setDebugLoc(ci->getDebugLoc());

      // set the calling convention for our new call the same as the old one
      newCi->setCallingConv(ci->getCallingConv());

      // replace anything that uses the old call with the new one
      ci->replaceAllUsesWith(newCi);

      // and remember to erase the old callsite
      callsToErase.push_back(ci);
    } else if (llvm::ConstantExpr *constant =
                   llvm::dyn_cast<llvm::ConstantExpr>(user)) {
      remapConstantExpr(constant, &oldFunc, &newFunc);
    } else {
      llvm_unreachable(
          "UNHANDLED user for Function not a CallInst or ConstantExpr\n");
    }
  }

  // remove all the old instructions we no longer need
  for (llvm::CallInst *ci : callsToErase) {
    // then destroy the call
    ci->eraseFromParent();
  }
}

bool addParamToAllFunctions(llvm::Module &module,
                            llvm::Type *const newParamType,
                            const llvm::AttributeSet &newParamAttrs,
                            const UpdateMDCallbackFn &updateMetaDataCallback) {
  return cloneFunctionsAddArg(
      module,
      [newParamType, newParamAttrs](llvm::Module &) {
        return ParamTypeAttrsPair{newParamType, newParamAttrs};
      },
      [](const llvm::Function &func, bool &ClonedWithBody, bool &ClonedNoBody) {
        // don't clone and add arg to special functions starting with __llvm.
        // These are reserved for clang generated functions such as profile
        // related ones
        ClonedWithBody = !func.getName().startswith("__llvm");
        ClonedNoBody = false;
      },
      updateMetaDataCallback);
}

llvm::BasicBlock *createLoop(llvm::BasicBlock *entry, llvm::BasicBlock *exit,
                             llvm::Value *indexStart, llvm::Value *indexEnd,
                             llvm::ArrayRef<llvm::Value *> ivs,
                             const CreateLoopOpts &opts,
                             CreateLoopBodyFn body) {
  // If the index increment is null, we default to 1 as our index.
  auto indexInc = opts.indexInc
                      ? opts.indexInc
                      : llvm::ConstantInt::get(indexStart->getType(), 1);

  llvm::LLVMContext &ctx = entry->getContext();

  llvm::SmallVector<llvm::Value *, 4> currIVs(ivs.begin(), ivs.end());
  llvm::SmallVector<llvm::Value *, 4> nextIVs(ivs.size());

  // Check if indexStart, indexEnd, and indexInc are constants.
  if (llvm::isa<llvm::ConstantInt>(indexStart) &&
      llvm::isa<llvm::ConstantInt>(indexEnd) &&
      llvm::isa<llvm::ConstantInt>(indexInc)) {
    auto start = llvm::cast<llvm::ConstantInt>(indexStart)->getZExtValue();
    auto end = llvm::cast<llvm::ConstantInt>(indexEnd)->getZExtValue();
    auto inc = llvm::cast<llvm::ConstantInt>(indexInc)->getZExtValue();

    // If the loop requires no iteration at all, just return an empty block.
    if ((end - start) == 0) {
      // If no iteration is required, and we have already defined a block to
      // branch to, create a link between the entry block to that exit block and
      // return the latter.
      if (exit) {
        llvm::BranchInst::Create(exit, entry);
        return exit;
      }
      return entry;
    } else if ((end - start) == inc) {
      // If our loop would only actually contain one iteration, don't output the
      // loop body!
      // run the lamdba for the loop body, storing the block is finished at.
      auto *b = body(entry, indexStart, currIVs, nextIVs);
      if (exit) {
        llvm::BranchInst::Create(exit, b);
        return exit;
      }
      return b;
    } else if (opts.attemptUnroll) {
      // We've been asked to attempt to unroll the loop! We only unroll loops in
      // certain situations though. We only unroll loops that have
      // maxUnrollIterations or less iterations, as a higher number will
      // significantly increase code bloat.
      const unsigned maxUnrollIterations = 2;
      if (((end - start) / inc) <= maxUnrollIterations) {
        // We start at the entry block for our insertions.
        llvm::BasicBlock *last = entry;

        for (auto i = start; i < end; i += inc) {
          // Update last to the exit block from the body.
          last = body(last, llvm::ConstantInt::get(indexStart->getType(), i),
                      currIVs, nextIVs);
          // Pass the 'next' values on to the next iteration, reusing the
          // storage.
          std::swap(currIVs, nextIVs);
        }

        // Return the last basic block we wrote to (our exit block).
        if (exit) {
          llvm::BranchInst::Create(exit, last);
          return exit;
        }
        return last;
      }
    }
  }

  // the basic block that will link into our loop
  llvm::IRBuilder<> entryIR(entry);

  // the basic block that will form the start of our loop
  llvm::IRBuilder<> loopIR(
      llvm::BasicBlock::Create(ctx, opts.headerName, entry->getParent()));

  // branch into our loop to begin executing
  entryIR.CreateBr(loopIR.GetInsertBlock());

  // first thing in the loop is our phi node for the loop counter
  auto phi = loopIR.CreatePHI(indexInc->getType(), 2);

  // and make the phi node equal the start index when coming from our entry
  phi->addIncoming(indexStart, entryIR.GetInsertBlock());

  // Set up all of our user PHIs
  for (unsigned i = 0, e = currIVs.size(); i != e; i++) {
    auto *const phi = loopIR.CreatePHI(currIVs[i]->getType(), 2);
    llvm::cast<llvm::PHINode>(phi)->addIncoming(currIVs[i],
                                                entryIR.GetInsertBlock());
    currIVs[i] = phi;
  }

  // run the lamdba for the loop body, storing the block is finished at
  llvm::BasicBlock *const latch =
      body(loopIR.GetInsertBlock(), phi, currIVs, nextIVs);
  llvm::IRBuilder<> bodyIR(latch);

  // add to the phi node to increment our loop counter
  auto *const add = bodyIR.CreateAdd(phi, indexInc);

  // and set that if we loop back around, the phi node will be the increment
  phi->addIncoming(add, latch);

  // Update all of our PHIs
  for (unsigned i = 0, e = currIVs.size(); i != e; i++) {
    llvm::cast<llvm::PHINode>(currIVs[i])->addIncoming(nextIVs[i], latch);
  }

  if (!exit) {
    // the basic block to exit our loop when we are done
    llvm::IRBuilder<> exitIR(
        llvm::BasicBlock::Create(ctx, "exitIR", entry->getParent()));
    exit = exitIR.GetInsertBlock();
  }

  // last, branch condition either to the exit, or for another loop iteration
  auto *const termBR = bodyIR.CreateCondBr(bodyIR.CreateICmpULT(add, indexEnd),
                                           loopIR.GetInsertBlock(), exit);

  if (opts.disableVectorize) {
    auto *const vecDisable = llvm::MDNode::get(
        ctx, {llvm::MDString::get(ctx, "llvm.loop.vectorize.enable"),
              llvm::ConstantAsMetadata::get(
                  llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx), false))});
    // LLVM loop metadata -- for legacy reasons -- must have a reference to
    // itself as its first operand. See
    // https://llvm.org/docs/LangRef.html#llvm-loop.
    auto *loopID = llvm::MDNode::get(ctx, {nullptr, vecDisable});
    loopID->replaceOperandWith(0, loopID);
    termBR->setMetadata(llvm::LLVMContext::MD_loop, loopID);
  }

  // we stopped executing in the exit block, so return that
  return exit;
}

llvm::Argument *getLastArgument(llvm::Function *f) {
  assert(!f->arg_empty() &&
         "Can't get last argument if there are no arguments");
  return f->arg_end() - 1;
}

unsigned getSizeTypeBytes(const llvm::Module &m) {
  return m.getDataLayout().getPointerSize(0);
}

llvm::IntegerType *getSizeType(const llvm::Module &m) {
  const llvm::DataLayout &dataLayout = m.getDataLayout();
  return llvm::IntegerType::get(m.getContext(),
                                dataLayout.getPointerSizeInBits(0));
}

static llvm::Function *createKernelWrapperFunctionImpl(
    llvm::Function &F, llvm::Function &NewFunction, llvm::StringRef Suffix) {
  assert(!Suffix.empty() && "Suffix must not be empty");

  auto baseName = getOrSetBaseFnName(NewFunction, F);
  NewFunction.setName(baseName + Suffix);

  // we don't use exceptions
  NewFunction.addFnAttr(llvm::Attribute::NoUnwind);

  // copy the calling convention from the old function
  NewFunction.setCallingConv(F.getCallingConv());

  // copy the metadata into the new kernel ignoring any debug info.
  copyFunctionMetadata(F, NewFunction);

  // drop kernel (+ entry point) information from the old function: we've
  // copied it over to the new one.
  dropIsKernel(F);

  // move debug info for function over
  NewFunction.setSubprogram(F.getSubprogram());

  F.setSubprogram(nullptr);

  // set the function to always inline: 'noinline' takes precedence, though
  if (!F.hasFnAttribute(llvm::Attribute::NoInline)) {
    F.addFnAttr(llvm::Attribute::AlwaysInline);
  }

  // lastly set the linkage to internal
  F.setLinkage(llvm::GlobalValue::InternalLinkage);

  return &NewFunction;
}

llvm::Function *createKernelWrapperFunction(llvm::Function &F,
                                            llvm::StringRef Suffix) {
  assert(!Suffix.empty() && "Suffix must not be empty");
  // Create our new function
  llvm::Function *const NewFunction = llvm::Function::Create(
      F.getFunctionType(), llvm::Function::ExternalLinkage, "", F.getParent());

  // copy over function attributes, including parameter attributes
  copyFunctionAttrs(F, *NewFunction);

  // Copy over parameter names
  for (auto it : zip(NewFunction->args(), F.args())) {
    std::get<0>(it).setName(std::get<1>(it).getName());
  }

  return createKernelWrapperFunctionImpl(F, *NewFunction, Suffix);
}

llvm::Function *createKernelWrapperFunction(
    llvm::Module &M, llvm::Function &F, llvm::ArrayRef<llvm::Type *> ArgTypes,
    llvm::StringRef Suffix) {
  assert(!Suffix.empty() && "Suffix must not be empty");
  llvm::FunctionType *NewFunctionType =
      llvm::FunctionType::get(F.getReturnType(), ArgTypes, false);

  // create our new function
  llvm::Function *const NewFunction = llvm::Function::Create(
      NewFunctionType, llvm::Function::ExternalLinkage, "", &M);

  // copy over function attributes, ignoring all parameter attributes - we
  // don't know what the parameter mapping is.
  copyFunctionAttrs(F, *NewFunction, 0);

  return createKernelWrapperFunctionImpl(F, *NewFunction, Suffix);
}

}  // namespace utils
}  // namespace compiler