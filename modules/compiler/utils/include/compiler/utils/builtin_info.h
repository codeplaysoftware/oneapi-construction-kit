// Copyright (C) Codeplay Software Limited
//
// Licensed under the Apache License, Version 2.0 (the "License") with LLVM
// Exceptions; you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://github.com/codeplaysoftware/oneapi-construction-kit/blob/main/LICENSE.txt
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

/// @file
///
/// @brief Information about compiler builtins.

#ifndef COMPILER_UTILS_BUILTIN_INFO_H_INCLUDED
#define COMPILER_UTILS_BUILTIN_INFO_H_INCLUDED

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/ConstantRange.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>
#include <multi_llvm/multi_llvm.h>
#include <multi_llvm/optional_helper.h>
#include <multi_llvm/vector_type_helper.h>

namespace compiler {
namespace utils {
/// @addtogroup utils
/// @{

using BuiltinID = int32_t;

enum BaseBuiltinID {
  eBuiltinUnknown,
  eBuiltinInvalid,

  // Mux builtins
  eMuxBuiltinIsFTZ,
  eMuxBuiltinUseFast,
  eMuxBuiltinIsEmbeddedProfile,
  eMuxBuiltinGetGlobalSize,
  eMuxBuiltinGetGlobalId,
  eMuxBuiltinGetGlobalOffset,
  eMuxBuiltinGetLocalSize,
  eMuxBuiltinGetLocalId,
  eMuxBuiltinSetLocalId,
  eMuxBuiltinGetSubGroupId,
  eMuxBuiltinSetSubGroupId,
  eMuxBuiltinGetNumGroups,
  eMuxBuiltinGetNumSubGroups,
  eMuxBuiltinSetNumSubGroups,
  eMuxBuiltinGetMaxSubGroupSize,
  eMuxBuiltinSetMaxSubGroupSize,
  eMuxBuiltinGetGroupId,
  eMuxBuiltinGetWorkDim,
  eMuxBuiltinDMARead1D,
  eMuxBuiltinDMARead2D,
  eMuxBuiltinDMARead3D,
  eMuxBuiltinDMAWrite1D,
  eMuxBuiltinDMAWrite2D,
  eMuxBuiltinDMAWrite3D,
  eMuxBuiltinDMAWait,
  eMuxBuiltinGetGlobalLinearId,
  eMuxBuiltinGetLocalLinearId,
  eMuxBuiltinGetEnqueuedLocalSize,
  // Synchronization builtins
  eMuxBuiltinMemBarrier,
  eMuxBuiltinSubGroupBarrier,
  eMuxBuiltinWorkGroupBarrier,

  // Marker - target builtins should start from here.
  eFirstTargetBuiltin,
};

/// @brief Describes the uniformity of a builtin's return values. An uniform
/// value is the same for all instances (e.g. SIMD lanes).
enum BuiltinUniformity : int32_t {
  /// @brief The uniformity of the builtin's return value cannot be determined.
  eBuiltinUniformityUnknown,
  /// @brief The builtin never returns uniform values.
  eBuiltinUniformityNever,
  /// @brief The builtin always returns uniform values.
  eBuiltinUniformityAlways,
  /// @brief The builtin returns uniform values if its inputs are uniform.
  eBuiltinUniformityLikeInputs,
  /// @brief The builtin returns a sequential instance ID value
  /// (e.g. get_local_id in OpenCL).
  eBuiltinUniformityInstanceID,
  /// @brief The builtin might return a sequential instance ID value,
  /// if its argument can be zero (e.g. get_local_id(x)).
  eBuiltinUniformityMaybeInstanceID
};

/// @brief Describes certain properties of builtin functions that the vectorizer
/// needs to know about.
enum BuiltinProperties : int32_t {
  /// @brief The builtin has no special propery.
  eBuiltinPropertyNone = 0,
  /// @brief The builtin returns a value related to the geometry of the work
  /// space, such as its dimension or an index into that dimensions.
  eBuiltinPropertyWorkItem = (1 << 0),
  /// @brief The builtin can affect the execution flow (e.g. barrier).
  eBuiltinPropertyExecutionFlow = (1 << 1),
  /// @brief The builtin implements a reduction, that is, it takes vector
  /// arguments and returns a scalar value.
  eBuiltinPropertyReduction = (1 << 2),
  /// @brief The builtin has known side-effects.
  eBuiltinPropertySideEffects = (1 << 3),
  /// @brief The builtin is known to have no runtime side-effects. This is
  /// equivalent to 'readonly' or 'readnone' in IR. The return value depends
  /// only on the values of the arguments.
  eBuiltinPropertyNoSideEffects = (1 << 4),
  /// @brief The builtin can be instantiated, even if it has side-effects.
  /// Builtins with 'NoSideEffects' should not be instantiated unless they
  /// also have this flag, because of the 'noduplicate' IR attribute.
  eBuiltinPropertySupportsInstantiation = (1 << 5),
  /// @brief The builtin has no vector equivalent. There may be functions that
  /// have the same signature that a vector equivalent function would have,
  /// but these functions should not be used for that purpose. This can also
  /// mean that a vector builtin has no scalar equivalent.
  eBuiltinPropertyNoVectorEquivalent = (1 << 6),
  /// @brief The builtin has a vector equivalent. This is used for the LLVM
  /// intrinsics, since for the OpenCL builtins we can determine that
  /// programmatically. It can also mean that a builtin has a scalar equivalent.
  eBuiltinPropertyVectorEquivalent = (1 << 7),
  /// @brief The builtin can be emitted inline.
  eBuiltinPropertyCanEmitInline = (1 << 8),
  /// @brief The builtin returns a value through its pointer argument. The
  /// returned type is equal to the function return type.
  eBuiltinPropertyPointerReturnEqualRetTy = (1 << 9),
  /// @brief The builtin wants to be inlined post vectorization
  eBuiltinPropertyInlinePostVectorization = (1 << 10),
  /// @brief The builtin returns a value through its pointer argument. The
  /// returned value is an i32 scalar or vector, matching the function return
  /// type: float -> i32, <4 x double> -> <4 x i32>, etc
  eBuiltinPropertyPointerReturnEqualIntRetTy = (1 << 11),
  /// @brief The builtin returns local work item ID.
  eBuiltinPropertyLocalID = (1 << 12),
  /// @brief The builtin is atomic
  eBuiltinPropertyAtomic = (1 << 13),
  /// @brief The builtin is rematerializable on the other side of a barrier
  ///
  /// The HandleBarriersPass queries this property to prune the number of live
  /// variables that are stored and passed between barrier regions. Calls to
  /// rematerializable builtins are removed from the live variable structure,
  /// and are re-inserted into each barrier region that requires their results.
  eBuiltinPropertyRematerializable = (1 << 14),
  /// @brief The builtin should be mapped to a mux synchronization builtin.
  ///
  /// This mapping takes place in BuiltiInfo::mapSyncBuiltinToMuxSyncBuiltin.
  eBuiltinPropertyMapToMuxSyncBuiltin = (1 << 15),
  /// @brief The builtin is known not be be convergent, i.e., it does not
  /// depend on any other work-item in any way.
  eBuiltinPropertyKnownNonConvergent = (1 << 16),
};

/// @brief struct to hold information about a builtin function
struct Builtin {
  /// @brief the builtin Function
  llvm::Function const &function;
  /// @brief ID for internal use
  BuiltinID const ID;
  /// @brief the Builtin Properties
  BuiltinProperties const properties;

  /// @brief returns whether the builtin is valid
  bool isValid() const { return ID != eBuiltinInvalid; }

  /// @brief returns whether the builtin is unknown
  bool isUnknown() const { return ID == eBuiltinUnknown; }
};

/// @brief struct to hold information about a builtin function call
struct BuiltinCall : public Builtin {
  /// @brief the call instruction
  llvm::CallInst const &call;
  /// @brief the uniformity of the builtin call
  BuiltinUniformity const uniformity;

  /// @brief constructor
  BuiltinCall(Builtin const &B, llvm::CallInst const &CI, BuiltinUniformity U)
      : Builtin(B), call(CI), uniformity(U) {}
};

namespace MuxBuiltins {
constexpr const char isftz[] = "__mux_isftz";
constexpr const char usefast[] = "__mux_usefast";
constexpr const char isembeddedprofile[] = "__mux_isembeddedprofile";
constexpr const char get_global_size[] = "__mux_get_global_size";
constexpr const char get_global_id[] = "__mux_get_global_id";
constexpr const char get_global_offset[] = "__mux_get_global_offset";
constexpr const char get_local_size[] = "__mux_get_local_size";
constexpr const char get_local_id[] = "__mux_get_local_id";
constexpr const char get_sub_group_id[] = "__mux_get_sub_group_id";
constexpr const char get_num_groups[] = "__mux_get_num_groups";
constexpr const char get_num_sub_groups[] = "__mux_get_num_sub_groups";
constexpr const char get_max_sub_group_size[] = "__mux_get_max_sub_group_size";
constexpr const char get_group_id[] = "__mux_get_group_id";
constexpr const char get_work_dim[] = "__mux_get_work_dim";
constexpr const char dma_read_1d[] = "__mux_dma_read_1D";
constexpr const char dma_read_2d[] = "__mux_dma_read_2D";
constexpr const char dma_read_3d[] = "__mux_dma_read_3D";
constexpr const char dma_write_1d[] = "__mux_dma_write_1D";
constexpr const char dma_write_2d[] = "__mux_dma_write_2D";
constexpr const char dma_write_3d[] = "__mux_dma_write_3D";
constexpr const char dma_wait[] = "__mux_dma_wait";
constexpr const char get_global_linear_id[] = "__mux_get_global_linear_id";
constexpr const char get_local_linear_id[] = "__mux_get_local_linear_id";
constexpr const char get_enqueued_local_size[] =
    "__mux_get_enqueued_local_size";

// Barriers
constexpr const char mem_barrier[] = "__mux_mem_barrier";
constexpr const char sub_group_barrier[] = "__mux_sub_group_barrier";
constexpr const char work_group_barrier[] = "__mux_work_group_barrier";

// DMA Event Type
constexpr const char dma_event_type[] = "__mux_dma_event_t";

// Internal Mux Functions
constexpr const char set_local_id[] = "__mux_set_local_id";
constexpr const char set_sub_group_id[] = "__mux_set_sub_group_id";
constexpr const char set_num_sub_groups[] = "__mux_set_num_sub_groups";
constexpr const char set_max_sub_group_size[] = "__mux_set_max_sub_group_size";
}  // namespace MuxBuiltins

static inline llvm::Type *getPointerReturnPointeeTy(const llvm::Function &F,
                                                    BuiltinProperties Props) {
  if (Props & eBuiltinPropertyPointerReturnEqualRetTy) {
    return F.getReturnType();
  }
  if (Props & eBuiltinPropertyPointerReturnEqualIntRetTy) {
    llvm::Type *I32Ty = llvm::IntegerType::getInt32Ty(F.getContext());
    if (auto *VTy = llvm::dyn_cast<llvm::VectorType>(F.getReturnType())) {
      return llvm::VectorType::get(I32Ty,
                                   multi_llvm::getVectorElementCount(VTy));
    }
    return I32Ty;
  }
  return nullptr;
}

/// @brief Describes how builtins should be materialized.
enum BuiltinMatFlags : int32_t {
  /// @brief Use default materialization options.
  eBuiltinMatDefault = 0,
  /// @brief The body of the builtin should be materialized.
  eBuiltinMatDefinition = (1 << 0)
};

/// @brief Describes the kind of subgroup reduction corresponding to a builtin.
enum BuiltinSubgroupReduceKind : int32_t {
  /// @brief Not a subgroup reduction
  eBuiltinSubgroupReduceInvalid,
  /// @brief sub_group_all
  eBuiltinSubgroupAll,
  /// @brief sub_group_any
  eBuiltinSubgroupAny,
  /// @brief sub_group_reduce_add
  eBuiltinSubgroupReduceAdd,
  /// @brief sub_group_reduce_min
  eBuiltinSubgroupReduceMin,
  /// @brief sub_group_reduce_max
  eBuiltinSubgroupReduceMax,
  /// @brief sub_group_reduce_mul
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupReduceMul,
  /// @brief sub_group_reduce_and
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupReduceAnd,
  /// @brief sub_group_reduce_or
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupReduceOr,
  /// @brief sub_group_reduce_xor
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupReduceXor,
  /// @brief sub_group_reduce_logical_and
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupReduceLogicalAnd,
  /// @brief sub_group_reduce_logical_or
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupReduceLogicalOr,
  /// @brief sub_group_reduce_logical_xor
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupReduceLogicalXor,
};

/// @brief Describes the kind of subgroup scan corresponding to a builtin.
enum BuiltinSubgroupScanKind : int32_t {
  /// @brief Not a subgroup reduction
  eBuiltinSubgroupScanInvalid,
  /// @brief sub_group_scan_add_inclusive
  eBuiltinSubgroupScanAddIncl,
  /// @brief sub_group_scan_add_exclusive
  eBuiltinSubgroupScanAddExcl,
  /// @brief sub_group_scan_min_inclusive
  eBuiltinSubgroupScanMinIncl,
  /// @brief sub_group_scan_min_exclusive
  eBuiltinSubgroupScanMinExcl,
  /// @brief sub_group_scan_max_inclusive
  eBuiltinSubgroupScanMaxIncl,
  /// @brief sub_group_scan_max_exclusive
  eBuiltinSubgroupScanMaxExcl,
  /// @brief sub_group_scan_mul_inclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanMulIncl,
  /// @brief sub_group_scan_mul_exclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanMulExcl,
  /// @brief sub_group_scan_and_inclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanAndIncl,
  /// @brief sub_group_scan_and_exclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanAndExcl,
  /// @brief sub_group_scan_or_inclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanOrIncl,
  /// @brief sub_group_scan_or_exclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanOrExcl,
  /// @brief sub_group_scan_xor_inclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanXorIncl,
  /// @brief sub_group_scan_xor_exclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanXorExcl,
  /// @brief sub_group_scan_logical_and_inclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanLogicalAndIncl,
  /// @brief sub_group_scan_logical_and_exclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanLogicalAndExcl,
  /// @brief sub_group_scan_logical_or_inclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanLogicalOrIncl,
  /// @brief sub_group_scan_logical_or_exclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanLogicalOrExcl,
  /// @brief sub_group_scan_logical_xor_inclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanLogicalXorIncl,
  /// @brief sub_group_scan_logical_xor_exclusive
  ///
  /// Provided by SPV_KHR_uniform_group_instructions.
  eBuiltinSubgroupScanLogicalXorExcl,
};

class BIMuxInfoConcept;
class BILangInfoConcept;

/// @brief A class that encapsulates information and transformations concerning
/// compiler builtin functions.
///
/// It provides methods for querying data about builtin functions, methods for
/// emitting bodies of builtins "inline", and methods for materializing
/// builtins from an external source.
///
/// It contains a BIMuxInfoConcept implementation to provide mux builtin
/// information on a target-by-target basis.
///
/// It contains an optional BILangInfoConcept implementation to provide builtin
/// information on a target-by-target basis.
class BuiltinInfo {
 public:
  // Default-construct a BuiltinInfo without a concrete set of language-level
  // builtins.
  BuiltinInfo() : MuxImpl(std::make_unique<BIMuxInfoConcept>()) {}

  BuiltinInfo(std::unique_ptr<BILangInfoConcept> &&LangImpl)
      : MuxImpl(std::make_unique<BIMuxInfoConcept>()),
        LangImpl(std::move(LangImpl)) {}

  BuiltinInfo(std::unique_ptr<BIMuxInfoConcept> &&MuxImpl,
              std::unique_ptr<BILangInfoConcept> &&LangImpl)
      : MuxImpl(std::move(MuxImpl)), LangImpl(std::move(LangImpl)) {}

  BuiltinInfo(BuiltinInfo &&) = default;
  BuiltinInfo &operator=(BuiltinInfo &&RHS) = default;

  /// @brief Retrieves the optional module containing builtin definitions.
  llvm::Module *getBuiltinsModule();

  /// @brief Determine general properties for the given builtin function.
  /// @param[in] F Function to analyze.
  /// @return Analyzed properties for the builtin.
  Builtin analyzeBuiltin(llvm::Function const &F) const;

  /// @brief Determine general properties for the given builtin function.
  /// @param[in] CI Call instruction to analyze.
  /// @return Analyzed properties for the builtin call.
  BuiltinCall analyzeBuiltinCall(llvm::CallInst const &CI,
                                 unsigned SimdDimIdx) const;

  /// @brief Try to find a builtin function that is a vector equivalent of the
  /// given function with the given vector width, if it exists.
  /// @param[in] B Builtin to query for a vector equivalent.
  /// @param[in] Width Vector width.
  /// @param[in] M Optional module where the vector equivalent should be
  /// declared.
  /// @return Equivalent vector builtin function on success.
  llvm::Function *getVectorEquivalent(Builtin const &B, unsigned Width,
                                      llvm::Module *M = nullptr);

  /// @brief Try to find a builtin function that is a scalar equivalent of the
  /// given function, if it exists.
  /// @param[in] B Builtin to query for a scalar equivalent.
  /// @param[in] M Optional module where the vector equivalent should be
  /// declared.
  /// @return Equivalent scalar builtin function on success.
  llvm::Function *getScalarEquivalent(Builtin const &B, llvm::Module *M);

  /// @brief Determine the kind of subgroup reduction pertaining to the
  /// specified builtin.
  /// @param[in] B Builtin to query.
  /// @return The BuiltinSubgroupReduceKind corresponding to the argument,
  /// returning eBuiltinSubgroupReduceInvalid if the argument is not a
  /// subgroup reduction.
  BuiltinSubgroupReduceKind getBuiltinSubgroupReductionKind(
      Builtin const &B) const;

  /// @brief Determine the kind of subgroup scan pertaining to the specified
  /// builtin.
  /// @param[in] B Builtin to query.
  /// @return The BuiltinSubgroupScanKind corresponding to the argument,
  /// returning eBuiltinSubgroupScanInvalid if the argument is not a subgroup
  /// reduction.
  BuiltinSubgroupScanKind getBuiltinSubgroupScanKind(Builtin const &B) const;

  /// @brief Emit an inline implementation of the builtin function F.
  /// @param[in] Builtin Builtin function to emit an implementation for.
  /// @param[in] B Insertion point for the implementation.
  /// @param[in] Args Arguments to the builtin function.
  /// @return A value that implements the builtin function or null.
  llvm::Value *emitBuiltinInline(llvm::Function *Builtin, llvm::IRBuilder<> &B,
                                 llvm::ArrayRef<llvm::Value *> Args);

  /// @brief Return a known range of values this call may return.
  /// @param[in] CI Call instruction to analyze.
  /// @param[in] MaxLocalSizes The maximum local work-group sizes in each of
  /// the 3 dimensions that this target supports.
  /// @param[in] MaxGlobalSizes The maximum global work-group sizes in each of
  /// the 3 dimensions that this target supports.
  multi_llvm::Optional<llvm::ConstantRange> getBuiltinRange(
      llvm::CallInst &CI,
      std::array<multi_llvm::Optional<uint64_t>, 3> MaxLocalSizes,
      std::array<multi_llvm::Optional<uint64_t>, 3> MaxGlobalSizes) const;

  /// @brief Remaps a call instruction to a call calling a mux synchronization
  /// builtin.
  ///
  /// For a call to a builtin for which the property
  /// eBuiltinPropertyMapToMuxSyncBuiltin is set, the target must then remap
  /// the call to a new call to the correct mux builtin, remapping any
  /// arguments as required.
  llvm::CallInst *mapSyncBuiltinToMuxSyncBuiltin(llvm::CallInst &CI);

  /// @brief Get a builtin for printf.
  /// @return An identifier for the builtin, or the invalid builtin if there
  /// is none. This builtin should have a signature of `<void type | integer
  /// type> <builtin name>(<char*>, ...)`.
  BuiltinID getPrintfBuiltin() const;

  /// @brief Get a builtin for a subgroup per-lane identifier.
  /// @return An identifier for the builtin, or the invalid builtin if there
  /// is none. This builtin should have a signature of `i32 <builtin name>()`.
  BuiltinID getSubgroupLocalIdBuiltin() const;

  /// @brief Get a builtin for a subgroup broadcast.
  /// @return An identifier for the builtin, or the invalid builtin if there
  /// is none. This builtin should have a signature of `<scalar ty> <builtin
  /// name>(<scalar ty>, i32)`.
  BuiltinID getSubgroupBroadcastBuiltin() const;

  /// @brief Returns true if the given ID is a ComputeMux builtin ID.
  static bool isMuxBuiltinID(BuiltinID ID) {
    return ID > eBuiltinInvalid && ID < eFirstTargetBuiltin;
  }

  /// @brief Returns true if the given ID is a ComputeMux barrier builtin ID.
  static bool isMuxControlBarrierID(BuiltinID ID) {
    return ID == eMuxBuiltinSubGroupBarrier ||
           ID == eMuxBuiltinWorkGroupBarrier;
  }

  /// @brief Returns true if the given ID is a ComputeMux DMA builtin ID.
  static bool isMuxDmaBuiltinID(BuiltinID ID) {
    return ID == eMuxBuiltinDMAWait || ID == eMuxBuiltinDMARead1D ||
           ID == eMuxBuiltinDMARead2D || ID == eMuxBuiltinDMARead3D ||
           ID == eMuxBuiltinDMAWrite1D || ID == eMuxBuiltinDMAWrite2D ||
           ID == eMuxBuiltinDMAWrite3D;
  }

  /// @brief Maps a ComputeMux builtin ID to its function name.
  static llvm::StringRef getMuxBuiltinName(BuiltinID ID);

  /// @brief Defines the body of a ComputeMux builtin declaration
  ///
  /// If the Module already has a function definition with the corresponding
  /// function name, it is left alone and returned.
  ///
  /// Will declare any builtins it requires as transitive dependencies.
  llvm::Function *defineMuxBuiltin(BuiltinID, llvm::Module &M);

  /// @brief Gets a ComputeMux builtin from the module, or declares it
  ///
  /// Only work-item builtins are supported.
  llvm::Function *getOrDeclareMuxBuiltin(BuiltinID, llvm::Module &M);

  struct SchedParamInfo {
    /// @brief An identifier providing resolution for targets to identify
    /// specific scheduling parameters.
    ///
    /// By default, will be the index into the list returned by
    /// getMuxSchedulingParameters.
    unsigned ID;
    /// @brief The parameter type
    llvm::Type *ParamTy;
    /// @brief A (possibly empty) set of parameter attributes to apply to all
    /// functions featuring this parameter.
    llvm::AttributeSet ParamAttrs;
    /// @brief The name of the parameter, to aid debugging. May be empty.
    std::string ParamName;
    /// @brief A human-readable name to be emitted in !mux-scheduling-params
    std::string ParamDebugName;
    /// @brief True if the parameter is passed externally by the driver to the
    /// kernel entry point, else false if this parameter is initialized by the
    /// kernel at the top level.
    ///
    /// This provides an interface to passes such as AddKernelWrapperPass.
    ///
    /// If true, the parameter is passed through every layer of kernels. If
    /// false, the parameter must be initialized by
    /// initializeSchedulingParamForWrappedKernel.
    bool PassedExternally;
    /// @brief An optional type to aid targets in remembering the underlying
    /// parameter type, if the parameter is a pointer.
    llvm::Type *ParamPointeeTy = nullptr;
    /// @brief An optional value specifying the concrete function argument.
    llvm::Argument *ArgVal = nullptr;
  };

  /// @brief Returns a target-specific list of scheduling parameters to be
  /// applied to all builtins for which requiresSchedulingParameters returns
  /// true.
  ///
  /// This list of parameters that dictates the order of parameters added to
  /// each builtin. As such it must be constant and immutable for each Module.
  ///
  /// This list is emitted into the module as metadata by the
  /// AddSchedulingParametersPass for user reference.
  ///
  /// This function does not have to fill in SchedParamInfo::ArgVal, as this
  /// query is not specific to one function.
  llvm::SmallVector<SchedParamInfo, 4> getMuxSchedulingParameters(
      llvm::Module &);

  /// @brief Returns target-specific scheduling parameters from a concrete
  /// function.
  ///
  /// Uses metadata returned via
  /// compiler::utils::getSchedulingParameterFunctionMetadata to determine
  /// whether the function contains scheduling parameters.
  ///
  /// If set, this function should return the same result as
  /// getMuxSchedulingParameters, but with SchedParamInfo::ArgVal filled in to
  /// correspond to the actual concrete llvm::Argument values of the given
  /// function. Note that not all ArgVals are guaranteed to be populated, as a
  /// function may contain only a subset of the target's list of scheduling
  /// parameters.
  ///
  /// If not set, this function returns an empty list.
  llvm::SmallVector<SchedParamInfo, 4> getFunctionSchedulingParameters(
      llvm::Function &);

  /// @brief Responsible for initializing a scheduling parameter for which
  /// PassedExternally is 'false'.
  ///
  /// This is conceptually used to initialize scheduling parameters which are
  /// used for scheduling "internally" and do not make up the driver-facing
  /// kernel ABI.
  ///
  /// @param Info The SchedParamInfo dictating which kind of scheduling
  /// parameter to initialize.
  /// @param B An IRBuilder providing the insertion point at which to insert
  /// initialization instructions.
  /// @param IntoF The function into which initialization instructions are to be
  /// inserted.
  /// @param CalleeF The function for which the initialization is taking place.
  /// CalleeF will be called by IntoF.
  llvm::Value *initializeSchedulingParamForWrappedKernel(
      const SchedParamInfo &Info, llvm::IRBuilder<> &B, llvm::Function &IntoF,
      llvm::Function &CalleeF);

  /// @brief Returns true if the builtin ID requires extra scheduling
  /// parameters to function.
  ///
  /// This function only handles mux builtins, and does not to defer any of
  /// BuiltinInfo's implementation instances.
  ///
  /// These parameters will to be added to the function (and its callers) by
  /// the AddSchedulingParametersPass.
  bool requiresSchedulingParameters(BuiltinID ID);

  /// Handle the invalidation of this information.
  ///
  /// When used as a result of BuiltinInfoAnalysis this method will be called
  /// when the function this was computed for changes. When it returns false,
  /// the information is preserved across those changes.
  bool invalidate(llvm::Module &, const llvm::PreservedAnalyses &,
                  llvm::ModuleAnalysisManager::Invalidator &) {
    return false;
  }

 private:
  /// @brief Try to identify a builtin function.
  /// @param[in] F The function to identify.
  /// @return Valid builtin ID if the name was identified.
  BuiltinID identifyMuxBuiltin(llvm::Function const &F) const;

  /// @brief Determine whether the given builtin function returns uniform values
  /// or not. An optional call instruction can be passed for more accuracy.
  /// @param[in] B the builtin to analyze uniformity.
  /// @param[in] CI Optional argument list from a call instruction.
  /// @param[in] SimdDimIdx Index of current vectorization dimension.
  /// @return Uniformity value for the builtin.
  BuiltinUniformity isBuiltinUniform(Builtin const &B, const llvm::CallInst *CI,
                                     unsigned SimdDimIdx) const;

  std::unique_ptr<BIMuxInfoConcept> MuxImpl;
  std::unique_ptr<BILangInfoConcept> LangImpl;
};

/// @brief An interface class that provides mux- and target-specific
/// information and transformations to an instance of BuiltinInfo. All methods
/// are to be called through from the equivalent methods in BuiltinInfo.
class BIMuxInfoConcept {
 public:
  virtual ~BIMuxInfoConcept() = default;

  /// @brief See BuiltinInfo::defineMuxBuiltin.
  virtual llvm::Function *defineMuxBuiltin(BuiltinID, llvm::Module &M);

  /// @brief See BuiltinInfo::getOrDeclareMuxBuiltin.
  virtual llvm::Function *getOrDeclareMuxBuiltin(BuiltinID, llvm::Module &M);

  /// @brief See BuiltinInfo::getMuxSchedulingParameters
  virtual llvm::SmallVector<BuiltinInfo::SchedParamInfo, 4>
  getMuxSchedulingParameters(llvm::Module &);

  /// @brief See BuiltinInfo::getFunctionSchedulingParameters
  virtual llvm::SmallVector<BuiltinInfo::SchedParamInfo, 4>
  getFunctionSchedulingParameters(llvm::Function &);

  /// @brief See BuiltinInfo::initializeSchedulingParamForWrappedKernel
  virtual llvm::Value *initializeSchedulingParamForWrappedKernel(
      const BuiltinInfo::SchedParamInfo &Info, llvm::IRBuilder<> &B,
      llvm::Function &IntoF, llvm::Function &CalleeF);

  /// @brief Sets default builtin attributes on the given function.
  static void setDefaultBuiltinAttributes(llvm::Function &F,
                                          bool AlwaysInline = true);

  /// @brief Returns true if the mux builtin requires scheduling parameters to
  /// function.
  virtual bool requiresSchedulingParameters(BuiltinID);

  enum MemScope : uint32_t {
    MemScopeCrossDevice = 0,
    MemScopeDevice = 1,
    MemScopeWorkGroup = 2,
    MemScopeSubGroup = 3,
    MemScopeWorkItem = 4,
  };

  enum MemSemantics : uint32_t {
    // Only set one of the following bits at a time:
    MemSemanticsRelaxed = 0x0,
    MemSemanticsAcquire = 0x2,
    MemSemanticsRelease = 0x4,
    MemSemanticsAcquireRelease = 0x8,
    MemSemanticsSequentiallyConsistent = 0x10,
    MemSemanticsMask = 0x1F,
    // What kind of memory is controlled by a barrier
    MemSemanticsSubGroupMemory = 0x80,
    MemSemanticsWorkGroupMemory = 0x100,
    MemSemanticsCrossWorkGroupMemory = 0x200,
  };

 protected:
  llvm::Function *defineGetGlobalId(llvm::Module &M);
  llvm::Function *defineGetGlobalSize(llvm::Module &M);
  llvm::Function *defineGetLocalLinearId(llvm::Module &M);
  llvm::Function *defineGetGlobalLinearId(llvm::Module &M);
  llvm::Function *defineGetEnqueuedLocalSize(llvm::Module &M);
  llvm::Function *defineMemBarrier(llvm::Function &F, unsigned ScopeIdx,
                                   unsigned SemanticsIdx);
  /// @brief Provides a default implementation for `__mux_dma_read_1D` and
  /// `__mux_dma_write_1D`.
  ///
  /// These routines are not intended to be efficient for a
  /// particular architecture and are really a placeholder for customers until
  /// they are ready to define these functions with DMA calls. They are
  /// essentially a memcpy.
  llvm::Function *defineDMA1D(llvm::Function &F);
  /// @brief Provides a default implementation for `__mux_dma_read_2D`
  /// and `__mux_dma_write_2D`.
  ///
  /// These routines are not intended to be efficient for a
  /// particular architecture and are really a placeholder for customers until
  /// they are ready to define these functions with DMA calls. They are
  /// essentially a memcpy.
  llvm::Function *defineDMA2D(llvm::Function &F);
  /// @brief Provides a default implementation for `__mux_dma_read_3D`
  /// and `__mux_dma_write_3D`.
  ///
  /// These routines are not intended to be efficient for a
  /// particular architecture and are really a placeholder for customers until
  /// they are ready to define these functions with DMA calls. They are
  /// essentially a memcpy.
  llvm::Function *defineDMA3D(llvm::Function &F);
  /// @brief Provides a default implementation for `__mux_dma_wait`.
  ///
  /// This routine is not intended to be efficient for a
  /// particular architecture and are really a placeholder for customers until
  /// they are ready to define these functions with DMA calls. This
  /// implementation does nothing and simply returns.
  llvm::Function *defineDMAWait(llvm::Function &F);
};

/// @brief An interface class that provides language-specific information and
/// transformations to an instance of BuiltinInfo. All methods are to be called
/// through from the equivalent methods in BuiltinInfo.
class BILangInfoConcept {
 public:
  virtual ~BILangInfoConcept() = default;

  /// @see BuiltinInfo::getBuiltinsModule
  virtual llvm::Module *getBuiltinsModule() { return nullptr; }
  /// @see BuiltinInfo::analyzeBuiltin
  virtual Builtin analyzeBuiltin(llvm::Function const &F) const = 0;
  /// @see BuiltinInfo::isBuiltinUniform
  virtual BuiltinUniformity isBuiltinUniform(Builtin const &B,
                                             const llvm::CallInst *,
                                             unsigned) const = 0;
  /// @see BuiltinInfo::getVectorEquivalent
  virtual llvm::Function *getVectorEquivalent(Builtin const &B, unsigned Width,
                                              llvm::Module *M = nullptr) = 0;
  /// @see BuiltinInfo::getScalarEquivalent
  virtual llvm::Function *getScalarEquivalent(Builtin const &B,
                                              llvm::Module *M) = 0;
  /// @see BuiltinInfo::getBuiltinSubgroupReductionKind
  virtual BuiltinSubgroupReduceKind getBuiltinSubgroupReductionKind(
      Builtin const &) const {
    return eBuiltinSubgroupReduceInvalid;
  }
  /// @see BuiltinInfo::getBuiltinSubgroupScanKind
  virtual BuiltinSubgroupScanKind getBuiltinSubgroupScanKind(
      Builtin const &) const {
    return eBuiltinSubgroupScanInvalid;
  }
  /// @see BuiltinInfo::emitBuiltinInline
  virtual llvm::Value *emitBuiltinInline(
      llvm::Function *Builtin, llvm::IRBuilder<> &B,
      llvm::ArrayRef<llvm::Value *> Args) = 0;
  /// @see BuiltinInfo::getBuiltinRange
  virtual multi_llvm::Optional<llvm::ConstantRange> getBuiltinRange(
      llvm::CallInst &, std::array<multi_llvm::Optional<uint64_t>, 3>,
      std::array<multi_llvm::Optional<uint64_t>, 3>) const {
    return multi_llvm::None;
  }
  /// @see BuiltinInfo::requiresMapToMuxSyncBuiltin
  virtual bool requiresMapToMuxSyncBuiltin(BuiltinID) const { return false; }

  /// @see BuiltinInfo::mapSyncBuiltinToMuxSyncBuiltin
  virtual llvm::CallInst *mapSyncBuiltinToMuxSyncBuiltin(llvm::CallInst &,
                                                         BIMuxInfoConcept &) {
    return nullptr;
  }
  /// @see BuiltinInfo::getPrintfBuiltin
  virtual BuiltinID getPrintfBuiltin() const = 0;
  /// @see BuiltinInfo::getSubgroupLocalIdBuiltin
  virtual BuiltinID getSubgroupLocalIdBuiltin() const {
    return eBuiltinInvalid;
  }
  /// @see BuiltinInfo::getSubgroupBroadcastBuiltin
  virtual BuiltinID getSubgroupBroadcastBuiltin() const {
    return eBuiltinInvalid;
  }
};

/// @brief Caches and returns the BuiltinInfo for a Module.
class BuiltinInfoAnalysis
    : public llvm::AnalysisInfoMixin<BuiltinInfoAnalysis> {
  friend AnalysisInfoMixin<BuiltinInfoAnalysis>;

 public:
  using Result = BuiltinInfo;
  using CallbackFn = std::function<Result(const llvm::Module &)>;

  BuiltinInfoAnalysis();

  BuiltinInfoAnalysis(CallbackFn BICallback) : BICallback(BICallback) {}

  /// @brief Retrieve the BuiltinInfo for the requested module.
  Result run(llvm::Module &M, llvm::ModuleAnalysisManager &) {
    return BICallback(M);
  }

  /// @brief Return the name of the pass.
  static llvm::StringRef name() { return "BuiltinInfo analysis"; }

 private:
  /// @brief Unique pass identifier.
  static llvm::AnalysisKey Key;

  /// @brief Callback function producing a BuiltinInfo on demand.
  CallbackFn BICallback;
};

/// @}
}  // namespace utils
}  // namespace compiler

#endif  // COMPILER_UTILS_BUILTIN_INFO_H_INCLUDED
