// Copyright (C) Codeplay Software Limited. All Rights Reserved.
#ifndef MULTI_LLVM_CREATION_APIS_HELPER_H_INCLUDED
#define MULTI_LLVM_CREATION_APIS_HELPER_H_INCLUDED

#include <llvm/ADT/None.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/TypeSize.h>
#include <multi_llvm/llvm_version.h>
#include <multi_llvm/vector_type_helper.h>

namespace multi_llvm {

// LLVM 11 updates the LoadInst constructors and creation APIs to consistently
// accept a return-type argument. The IRBuilder CreateLoad() APIs are still
// backwards compatible in LLVM 11, so no need to handle them for now, but
// the public constructors that we use internally in CA do need handling.

inline llvm::LoadInst *newLoadInst(llvm::Type *Ty, llvm::Value *Ptr,
                                   const llvm::Twine &NameStr,
                                   llvm::Instruction *InsertBefore) {
  return new llvm::LoadInst(Ty, Ptr, NameStr, InsertBefore);
}

inline llvm::LoadInst *newLoadInst(llvm::Type *Ty, llvm::Value *Ptr,
                                   const llvm::Twine &NameStr,
                                   llvm::BasicBlock *InsertAtEnd) {
  return new llvm::LoadInst(Ty, Ptr, NameStr, InsertAtEnd);
}

inline llvm::LoadInst *newLoadInst(llvm::Type *Ty, llvm::Value *Ptr,
                                   const llvm::Twine &NameStr, bool isVolatile,
                                   llvm::Instruction *InsertBefore) {
  return new llvm::LoadInst(Ty, Ptr, NameStr, isVolatile, InsertBefore);
}

inline llvm::LoadInst *newLoadInst(llvm::Type *Ty, llvm::Value *Ptr,
                                   const llvm::Twine &NameStr, bool isVolatile,
                                   llvm::BasicBlock *InsertAtEnd) {
  return new llvm::LoadInst(Ty, Ptr, NameStr, isVolatile, InsertAtEnd);
}

inline llvm::LoadInst *newLoadInst(llvm::Type *Ty, llvm::Value *Ptr,
                                   const llvm::Twine &NameStr, bool isVolatile,
                                   unsigned Alignment,
                                   llvm::Instruction *InsertBefore = nullptr) {
  auto Align = llvm::MaybeAlign(Alignment).valueOrOne();
  return new llvm::LoadInst(Ty, Ptr, NameStr, isVolatile, Align, InsertBefore);
}

inline llvm::LoadInst *newLoadInst(llvm::Type *Ty, llvm::Value *Ptr,
                                   const llvm::Twine &NameStr, bool isVolatile,
                                   unsigned Alignment,
                                   llvm::BasicBlock *InsertAtEnd) {
  auto Align = llvm::MaybeAlign(Alignment).valueOrOne();
  return new llvm::LoadInst(Ty, Ptr, NameStr, isVolatile, Align, InsertAtEnd);
}

inline llvm::LoadInst *newLoadInst(
    llvm::Type *Ty, llvm::Value *Ptr, const llvm::Twine &NameStr,
    bool isVolatile, unsigned Alignment, llvm::AtomicOrdering Order,
    llvm::SyncScope::ID SSID = llvm::SyncScope::System,
    llvm::Instruction *InsertBefore = nullptr) {
  auto Align = llvm::MaybeAlign(Alignment).valueOrOne();
  return new llvm::LoadInst(Ty, Ptr, NameStr, isVolatile, Align, Order, SSID,
                            InsertBefore);
}

inline llvm::LoadInst *newLoadInst(llvm::Type *Ty, llvm::Value *Ptr,
                                   const llvm::Twine &NameStr, bool isVolatile,
                                   unsigned Alignment,
                                   llvm::AtomicOrdering Order,
                                   llvm::SyncScope::ID SSID,
                                   llvm::BasicBlock *InsertAtEnd) {
  auto Align = llvm::MaybeAlign(Alignment).valueOrOne();
  return new llvm::LoadInst(Ty, Ptr, NameStr, isVolatile, Align, Order, SSID,
                            InsertAtEnd);
}

// LLVM 11 updates the CallInst constructors and creation APIs to consistently
// accept a return-type argument. The constructors are private, so we only
// handle the differences in the public Creation APIs for CallInst.

static inline llvm::CallInst *createCallInst(llvm::Value *Callee,
                                             llvm::FunctionType *FTy,
                                             llvm::ArrayRef<llvm::Value *> Args,
                                             const llvm::Twine &NameStr,
                                             llvm::BasicBlock *InsertAtEnd) {
  return llvm::CallInst::Create(llvm::FunctionCallee(FTy, Callee), Args,
                                NameStr, InsertAtEnd);
}

inline llvm::CallInst *createCall(llvm::IRBuilder<> &Builder, llvm::Value *Func,
                                  llvm::FunctionType *FTy,
                                  llvm::ArrayRef<llvm::Value *> Args,
                                  const llvm::Twine &Name = "",
                                  llvm::MDNode *FPMathTag = nullptr) {
  return Builder.CreateCall(FTy, Func, Args, Name, FPMathTag);
}

inline llvm::Value *createVectorSplat(llvm::IRBuilder<> &Builder,
                                      llvm::ElementCount EC, llvm::Value *V,
                                      const llvm::Twine &Name = "") {
  return Builder.CreateVectorSplat(EC, V, Name);
}

inline llvm::Value *createAllTrueMask(llvm::IRBuilder<> &B,
                                      llvm::ElementCount EC) {
  return llvm::ConstantInt::getTrue(llvm::VectorType::get(B.getInt1Ty(), EC));
}

inline llvm::Value *createIndexSequence(llvm::IRBuilder<> &Builder,
                                        llvm::Type *Ty, llvm::ElementCount EC,
                                        const llvm::Twine &Name = "") {
  (void)Builder;
  (void)Name;
  if (EC.isScalable()) {
    // FIXME: This intrinsic works on fixed-length types too: should we migrate
    // to using it starting from LLVM 13?
    return Builder.CreateStepVector(Ty, Name);
  }

  llvm::SmallVector<llvm::Constant *, 16> Indices;
  unsigned SimdWidth = EC.getFixedValue();
  for (unsigned i = 0; i < SimdWidth; i++) {
    Indices.push_back(llvm::ConstantInt::get(getVectorElementType(Ty), i));
  }
  return llvm::ConstantVector::get(Indices);
}

inline llvm::CallInst *createRISCVMaskedIntrinsic(
    llvm::IRBuilder<> &B, llvm::Intrinsic::ID ID,
    llvm::ArrayRef<llvm::Type *> Types, llvm::ArrayRef<llvm::Value *> Args,
    unsigned TailPolicy, llvm::Instruction *FMFSource = nullptr,
    const llvm::Twine &Name = "") {
  llvm::SmallVector<llvm::Value *> InArgs(Args.begin(), Args.end());
  InArgs.push_back(
      B.getIntN(Args.back()->getType()->getIntegerBitWidth(), TailPolicy));
  return B.CreateIntrinsic(ID, Types, InArgs, FMFSource, Name);
}

}  // namespace multi_llvm

#endif  // MULTI_LLVM_CREATION_APIS_HELPER_H_INCLUDED