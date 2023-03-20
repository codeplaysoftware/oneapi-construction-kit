// Copyright (C) Codeplay Software Limited. All Rights Reserved.

#include <compiler/utils/metadata.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

using namespace llvm;

namespace compiler {
namespace utils {

static constexpr const char *ReqdWGSizeMD = "reqd_work_group_size";

static MDTuple *encodeVectorizationInfo(const VectorizationInfo &info,
                                        LLVMContext &Ctx) {
  auto *const i32Ty = Type::getInt32Ty(Ctx);

  return MDTuple::get(
      Ctx,
      {ConstantAsMetadata::get(ConstantInt::get(i32Ty, info.vf.getKnownMin())),
       ConstantAsMetadata::get(ConstantInt::get(i32Ty, info.vf.isScalable())),
       ConstantAsMetadata::get(ConstantInt::get(i32Ty, info.simdDimIdx)),
       ConstantAsMetadata::get(
           ConstantInt::get(i32Ty, info.IsVectorPredicated))});
}

static Optional<VectorizationInfo> extractVectorizationInfo(MDTuple *md) {
  if (md->getNumOperands() != 4) {
    return None;
  }
  auto *const widthMD = mdconst::extract<ConstantInt>(md->getOperand(0));
  auto *const isScalableMD = mdconst::extract<ConstantInt>(md->getOperand(1));
  auto *const simdDimIdxMD = mdconst::extract<ConstantInt>(md->getOperand(2));
  auto *const isVPMD = mdconst::extract<ConstantInt>(md->getOperand(3));

  VectorizationInfo info;

  info.vf.setKnownMin(widthMD->getZExtValue());
  info.vf.setIsScalable(isScalableMD->equalsInt(1));
  info.simdDimIdx = simdDimIdxMD->getZExtValue();
  info.IsVectorPredicated = isVPMD->equalsInt(1);

  return info;
};

static Optional<LinkMetadataResult> parseVectorLinkMD(MDNode *mdnode) {
  if (auto info =
          extractVectorizationInfo(dyn_cast<MDTuple>(mdnode->getOperand(0)))) {
    // The Function may well be null.
    Function *vecFn = mdconst::extract_or_null<Function>(mdnode->getOperand(1));
    return LinkMetadataResult(vecFn, *info);
  }
  return None;
}

void encodeVectorizationFailedMetadata(Function &f,
                                       const VectorizationInfo &info) {
  auto *veczInfo = encodeVectorizationInfo(info, f.getContext());
  f.addMetadata("codeplay_ca_vecz.base.fail", *veczInfo);
}

void linkOrigToVeczFnMetadata(Function &origF, Function &vectorF,
                              const VectorizationInfo &info) {
  auto *veczInfo = encodeVectorizationInfo(info, origF.getContext());
  auto *const mdTuple = MDTuple::get(
      origF.getContext(), {veczInfo, ValueAsMetadata::get(&vectorF)});
  origF.addMetadata("codeplay_ca_vecz.base", *mdTuple);
}

void linkVeczToOrigFnMetadata(Function &vectorizedF, Function &origF,
                              const VectorizationInfo &info) {
  auto *veczInfo = encodeVectorizationInfo(info, vectorizedF.getContext());
  auto *const mdTuple = MDTuple::get(origF.getContext(),
                                     {veczInfo, ValueAsMetadata::get(&origF)});
  vectorizedF.addMetadata("codeplay_ca_vecz.derived", *mdTuple);
}

static bool parseVectorizedFunctionLinkMetadata(
    Function &f, StringRef mdName,
    SmallVectorImpl<LinkMetadataResult> &results) {
  SmallVector<MDNode *, 1> nodes;

  f.getMetadata(mdName, nodes);
  if (nodes.empty()) {
    return false;
  }
  results.reserve(results.size() + nodes.size());
  for (auto *mdnode : nodes) {
    if (auto link = parseVectorLinkMD(mdnode)) {
      results.emplace_back(*link);
    } else {
      return false;
    }
  }
  return true;
}

bool parseOrigToVeczFnLinkMetadata(Function &f,
                                   SmallVectorImpl<LinkMetadataResult> &VFs) {
  return parseVectorizedFunctionLinkMetadata(f, "codeplay_ca_vecz.base", VFs);
}

Optional<LinkMetadataResult> parseVeczToOrigFnLinkMetadata(Function &f) {
  auto *mdnode = f.getMetadata("codeplay_ca_vecz.derived");
  if (!mdnode) {
    return None;
  }
  return parseVectorLinkMD(mdnode);
}

void dropVeczOrigMetadata(Function &f) {
  f.setMetadata("codeplay_ca_vecz.base", nullptr);
}

void dropVeczDerivedMetadata(Function &f) {
  f.setMetadata("codeplay_ca_vecz.derived", nullptr);
}

void encodeWrapperFnMetadata(Function &f, const VectorizationInfo &mainInfo,
                             Optional<VectorizationInfo> tailInfo) {
  MDTuple *tailInfoMD = nullptr;
  auto *mainInfoMD = encodeVectorizationInfo(mainInfo, f.getContext());

  if (tailInfo) {
    tailInfoMD = encodeVectorizationInfo(*tailInfo, f.getContext());
  }

  f.setMetadata("codeplay_ca_wrapper",
                MDTuple::get(f.getContext(), {mainInfoMD, tailInfoMD}));
}

Optional<std::pair<VectorizationInfo, Optional<VectorizationInfo>>>
parseWrapperFnMetadata(Function &f) {
  auto *const mdnode = f.getMetadata("codeplay_ca_wrapper");
  if (!mdnode || mdnode->getNumOperands() != 2) {
    return None;
  }

  auto *const mainTuple = dyn_cast_or_null<MDTuple>(mdnode->getOperand(0));
  if (!mainTuple) {
    return None;
  }

  VectorizationInfo mainInfo;
  Optional<VectorizationInfo> tailInfo;

  if (auto info = extractVectorizationInfo(mainTuple)) {
    mainInfo = *info;
  } else {
    return None;
  }

  if (auto *const tailTuple =
          dyn_cast_or_null<MDTuple>(mdnode->getOperand(1))) {
    if (auto info = extractVectorizationInfo(tailTuple)) {
      tailInfo = info;
    }
  }

  return std::make_pair(mainInfo, tailInfo);
}

void copyFunctionMetadata(Function &fromF, Function &toF, bool includeDebug) {
  if (includeDebug) {
    toF.copyMetadata(&fromF, 0);
    return;
  }
  // Copy the metadata into the new kernel ignoring any debug info.
  SmallVector<std::pair<unsigned, MDNode *>, 5> metadata;
  fromF.getAllMetadata(metadata);

  // Iterate through the metadata and only add nodes to the new one if they
  // are not debug info.
  for (const auto &pair : metadata) {
    if (auto *nonDebug = dyn_cast_or_null<MDTuple>(pair.second)) {
      toF.setMetadata(pair.first, nonDebug);
    }
  }
}

void encodeLocalSizeMetadata(Function &f, const std::array<uint64_t, 3> &size) {
  // We may be truncating i64 to i32 but we don't expect local sizes to ever
  // exceed 32 bits.
  auto *const i32Ty = Type::getInt32Ty(f.getContext());
  auto *const mdTuple =
      MDTuple::get(f.getContext(),
                   {ConstantAsMetadata::get(ConstantInt::get(i32Ty, size[0])),
                    ConstantAsMetadata::get(ConstantInt::get(i32Ty, size[1])),
                    ConstantAsMetadata::get(ConstantInt::get(i32Ty, size[2]))});
  f.setMetadata(ReqdWGSizeMD, mdTuple);
}

Optional<std::array<uint64_t, 3>> getLocalSizeMetadata(const Function &f) {
  if (auto *md = f.getMetadata(ReqdWGSizeMD)) {
    return std::array<uint64_t, 3>{
        mdconst::extract<ConstantInt>(md->getOperand(0))->getZExtValue(),
        mdconst::extract<ConstantInt>(md->getOperand(1))->getZExtValue(),
        mdconst::extract<ConstantInt>(md->getOperand(2))->getZExtValue()};
  }
  return None;
}

static constexpr const char *MuxScheduledFnMD = "mux_scheduled_fn";

void dropSchedulingParameterMetadata(Function &f) {
  f.setMetadata(MuxScheduledFnMD, nullptr);
}

SmallVector<int, 4> getSchedulingParameterFunctionMetadata(const Function &f) {
  SmallVector<int, 4> idxs;
  if (auto *md = f.getMetadata(MuxScheduledFnMD)) {
    for (auto &op : md->operands()) {
      idxs.push_back(mdconst::extract<ConstantInt>(op)->getSExtValue());
    }
  }
  return idxs;
}

void setSchedulingParameterFunctionMetadata(Function &f, ArrayRef<int> idxs) {
  if (idxs.empty()) {
    return;
  }
  SmallVector<Metadata *, 4> mdOps;
  auto *const i32Ty = Type::getInt32Ty(f.getContext());
  for (auto idx : idxs) {
    mdOps.push_back(ConstantAsMetadata::get(ConstantInt::get(i32Ty, idx)));
  }
  auto *const mdOpsTuple = MDTuple::get(f.getContext(), mdOps);
  f.setMetadata(MuxScheduledFnMD, mdOpsTuple);
}

static constexpr const char *MuxSchedulingParamsMD = "mux-scheduling-params";

void setSchedulingParameterModuleMetadata(Module &m,
                                          ArrayRef<std::string> names) {
  SmallVector<Metadata *, 4> paramDebugNames;
  for (const auto &name : names) {
    paramDebugNames.push_back(MDString::get(m.getContext(), name));
  }
  auto *const md = m.getOrInsertNamedMetadata(MuxSchedulingParamsMD);
  md->clearOperands();
  md->addOperand(MDNode::get(m.getContext(), paramDebugNames));
}

NamedMDNode *getSchedulingParameterModuleMetadata(const Module &m) {
  return m.getNamedMetadata(MuxSchedulingParamsMD);
}

Optional<unsigned> isSchedulingParameter(const Function &f, unsigned idx) {
  if (auto *md = f.getMetadata(MuxScheduledFnMD)) {
    for (const auto &op : enumerate(md->operands())) {
      auto paramIdx = mdconst::extract<ConstantInt>(op.value())->getSExtValue();
      if (paramIdx >= 0 && (unsigned)paramIdx == idx) {
        return op.index();
      }
    }
  }
  return None;
}

// Uses the format of a metadata node directly applied to a function.
Optional<std::array<uint64_t, 3>> parseRequiredWGSMetadata(const Function &f) {
  if (auto mdnode = f.getMetadata(ReqdWGSizeMD)) {
    auto *const op0 = mdconst::extract<ConstantInt>(mdnode->getOperand(0));
    auto *const op1 = mdconst::extract<ConstantInt>(mdnode->getOperand(1));
    auto *const op2 = mdconst::extract<ConstantInt>(mdnode->getOperand(2));

    // KLOCWORK "UNINIT.STACK.ARRAY.MUST" possible false positive
    // This is normal std::array initialization
    std::array<uint64_t, 3> wgs = {
        {op0->getZExtValue(), op1->getZExtValue(), op2->getZExtValue()}};
    return wgs;
  }
  return None;
}

// Uses the format of a metadata node that's a part of the opencl.kernels node.
Optional<std::array<uint64_t, 3>> parseRequiredWGSMetadata(const MDNode &node) {
  for (uint32_t i = 1; i < node.getNumOperands(); ++i) {
    MDNode *const subNode = cast<MDNode>(node.getOperand(i));
    MDString *const operandName = cast<MDString>(subNode->getOperand(0));
    if (operandName->getString() == ReqdWGSizeMD) {
      auto *const op0 = mdconst::extract<ConstantInt>(subNode->getOperand(1));
      auto *const op1 = mdconst::extract<ConstantInt>(subNode->getOperand(2));
      auto *const op2 = mdconst::extract<ConstantInt>(subNode->getOperand(3));
      // KLOCWORK "UNINIT.STACK.ARRAY.MUST" possible false positive
      // Initialization of looks like an uninitialized access to Klocwork
      std::array<uint64_t, 3> wgs = {
          {op0->getZExtValue(), op1->getZExtValue(), op2->getZExtValue()}};
      return wgs;
    }
  }
  return None;
}

Optional<uint32_t> parseMaxWorkDimMetadata(const Function &f) {
  if (auto *mdnode = f.getMetadata("max_work_dim")) {
    auto *op0 = mdconst::extract<ConstantInt>(mdnode->getOperand(0));
    return op0->getZExtValue();
  }

  return None;
}

void populateKernelList(Module &m, SmallVectorImpl<KernelInfo> &results) {
  // Construct list of kernels from metadata, if present.
  if (auto *md = m.getNamedMetadata("opencl.kernels")) {
    for (uint32_t i = 0, e = md->getNumOperands(); i < e; ++i) {
      MDNode *const kernelNode = md->getOperand(i);
      ValueAsMetadata *vmdKernel =
          cast<ValueAsMetadata>(kernelNode->getOperand(0));
      KernelInfo info{vmdKernel->getValue()->getName()};
      if (auto wgs = parseRequiredWGSMetadata(*kernelNode)) {
        info.ReqdWGSize = *wgs;
      }
      results.push_back(info);
    }
    return;
  }

  // No metadata - assume all functions with the SPIR_KERNEL calling
  // convention are kernels.
  for (auto &f : m) {
    if (f.hasName() && f.getCallingConv() == CallingConv::SPIR_KERNEL) {
      KernelInfo info(f.getName());
      if (auto wgs = parseRequiredWGSMetadata(f)) {
        info.ReqdWGSize = *wgs;
      }
      results.push_back(info);
    }
  }
}

}  // namespace utils
}  // namespace compiler