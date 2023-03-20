// Copyright (C) Codeplay Software Limited. All Rights Reserved.

#include <compiler/utils/attributes.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

namespace compiler {
namespace utils {
using namespace llvm;

static constexpr const char *MuxKernelAttrName = "mux-kernel";

void setIsKernel(Function &F) { F.addFnAttr(MuxKernelAttrName, ""); }

void setIsKernelEntryPt(Function &F) {
  F.addFnAttr(MuxKernelAttrName, "entry-point");
}

bool isKernel(const Function &F) {
  return F.getFnAttribute(MuxKernelAttrName).isValid();
}

bool isKernelEntryPt(const Function &F) {
  Attribute Attr = F.getFnAttribute(MuxKernelAttrName);
  if (Attr.isValid()) {
    return Attr.getValueAsString() == "entry-point";
  }
  return false;
}

void dropIsKernel(Function &F) { F.removeFnAttr(MuxKernelAttrName); }

void takeIsKernel(Function &ToF, Function &FromF) {
  if (!isKernel(FromF)) {
    return;
  }
  // Check whether we need to add entry-point data.
  bool IsEntryPt = isKernelEntryPt(FromF);
  // Drop all data for simplicity
  dropIsKernel(ToF);
  dropIsKernel(FromF);
  // Add the new data
  IsEntryPt ? setIsKernelEntryPt(ToF) : setIsKernel(ToF);
}

static StringRef getFnNameFromAttr(const Function &F, StringRef AttrName) {
  Attribute Attr = F.getFnAttribute(AttrName);
  if (Attr.isValid()) {
    return Attr.getValueAsString();
  }
  return "";
}

static constexpr const char *OrigFnNameAttr = "mux-orig-fn";

void setOrigFnName(Function &F) { F.addFnAttr(OrigFnNameAttr, F.getName()); }

StringRef getOrigFnName(const Function &F) {
  return getFnNameFromAttr(F, OrigFnNameAttr);
}

StringRef getOrigFnNameOrFnName(const Function &F) {
  auto N = getFnNameFromAttr(F, OrigFnNameAttr);
  return N.empty() ? F.getName() : N;
}

static constexpr const char *BaseFnNameAttr = "mux-base-fn-name";

void setBaseFnName(Function &F, StringRef N) { F.addFnAttr(BaseFnNameAttr, N); }

StringRef getBaseFnName(const Function &F) {
  return getFnNameFromAttr(F, BaseFnNameAttr);
}

StringRef getBaseFnNameOrFnName(const Function &F) {
  auto N = getFnNameFromAttr(F, BaseFnNameAttr);
  return N.empty() ? F.getName() : N;
}

StringRef getOrSetBaseFnName(Function &F, const Function &SetFromF) {
  Attribute Attr = F.getFnAttribute(BaseFnNameAttr);
  if (Attr.isValid()) {
    return Attr.getValueAsString();
  }

  // Try and peer through the original function's name
  StringRef BaseFnName = getBaseFnNameOrFnName(SetFromF);
  F.addFnAttr(BaseFnNameAttr, BaseFnName);
  setBaseFnName(F, BaseFnName);
  return BaseFnName;
}

static Optional<int> getStringFnAttrAsInt(const Attribute &Attr) {
  if (Attr.isValid()) {
    int AttrValue = 0;
    if (!Attr.getValueAsString().getAsInteger(10, AttrValue)) {
      return AttrValue;
    }
  }
  return None;
}

static constexpr const char *LocalMemUsageAttrName = "mux-local-mem-usage";

void setLocalMemoryUsage(Function &F, uint64_t LocalMemUsage) {
  Attribute Attr = Attribute::get(F.getContext(), LocalMemUsageAttrName,
                                  itostr(LocalMemUsage));
  F.addFnAttr(Attr);
}

Optional<uint64_t> getLocalMemoryUsage(const Function &F) {
  Attribute Attr = F.getFnAttribute(LocalMemUsageAttrName);
  auto Val = getStringFnAttrAsInt(Attr);
  // Only return non-negative integers
  return Val && Val >= 0 ? Optional<uint64_t>(*Val) : None;
}

static constexpr const char *DMAReqdSizeBytesAttrName = "mux-dma-reqd-size";

void setDMAReqdSizeBytes(Function &F, uint32_t DMASizeBytes) {
  Attribute Attr = Attribute::get(F.getContext(), DMAReqdSizeBytesAttrName,
                                  itostr(DMASizeBytes));
  F.addFnAttr(Attr);
}

Optional<uint32_t> getDMAReqdSizeBytes(const Function &F) {
  Attribute Attr = F.getFnAttribute(DMAReqdSizeBytesAttrName);
  auto Val = getStringFnAttrAsInt(Attr);
  // Only return non-negative integers
  return Val && Val >= 0 ? Optional<uint32_t>(*Val) : None;
}

static constexpr const char *BarrierScheduleAttrName = "mux-barrier-schedule";

void setBarrierSchedule(CallInst &CI, BarrierSchedule Sched) {
  StringRef Val;
  switch (Sched) {
    default:
    case BarrierSchedule::Unordered:
      Val = "unordered";
      break;
    case BarrierSchedule::Once:
      Val = "once";
      break;
    case BarrierSchedule::ScalarTail:
      Val = "scalar-tail";
      break;
    case BarrierSchedule::Linear:
      Val = "linear";
      break;
  }

  Attribute Attr =
      Attribute::get(CI.getContext(), BarrierScheduleAttrName, Val);
  CI.addFnAttr(Attr);
}

BarrierSchedule getBarrierSchedule(CallInst const &CI) {
  Attribute Attr = CI.getFnAttr(BarrierScheduleAttrName);
  if (Attr.isValid()) {
    return StringSwitch<BarrierSchedule>(Attr.getValueAsString())
        .Case("once", BarrierSchedule::Once)
        .Case("scalar-tail", BarrierSchedule::ScalarTail)
        .Case("linear", BarrierSchedule::Linear)
        .Default(BarrierSchedule::Unordered);
  }
  return BarrierSchedule::Unordered;
}

static constexpr const char *MuxDegenerateSubgroupsAttrName =
    "mux-degenerate-subgroups";

void setHasDegenerateSubgroups(Function &F) {
  F.addFnAttr(MuxDegenerateSubgroupsAttrName);
}

bool hasDegenerateSubgroups(const Function &F) {
  Attribute Attr = F.getFnAttribute(MuxDegenerateSubgroupsAttrName);
  return Attr.isValid();
}

}  // namespace utils
}  // namespace compiler