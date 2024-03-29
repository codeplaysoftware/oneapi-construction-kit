// Copyright (C) Codeplay Software Limited
//
// Licensed under the Apache License, Version 2.0 (the "License") with LLVM
// Exceptions you may not use this file except in compliance with the License.
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

/// @file Provides utilities for generating human-readable names from SPIR-V
/// data, using X macros.

#ifndef CAPABILITY
#define CAPABILITY(ENUM, NAME)
#endif

CAPABILITY(spv::CapabilityMatrix, "Matrix")
CAPABILITY(spv::CapabilityShader, "Shader")
CAPABILITY(spv::CapabilityGeometry, "Geometry")
CAPABILITY(spv::CapabilityTessellation, "Tessellation")
CAPABILITY(spv::CapabilityAddresses, "Addresses")
CAPABILITY(spv::CapabilityLinkage, "Linkage")
CAPABILITY(spv::CapabilityKernel, "Kernel")
CAPABILITY(spv::CapabilityVector16, "Vector16")
CAPABILITY(spv::CapabilityFloat16Buffer, "Float16Buffer")
CAPABILITY(spv::CapabilityFloat16, "Float16")
CAPABILITY(spv::CapabilityFloat64, "Float64")
CAPABILITY(spv::CapabilityInt64, "Int64")
CAPABILITY(spv::CapabilityInt64Atomics, "Int64Atomics")
CAPABILITY(spv::CapabilityImageBasic, "ImageBasic")
CAPABILITY(spv::CapabilityImageReadWrite, "ImageReadWrite")
CAPABILITY(spv::CapabilityImageMipmap, "ImageMipmap")
CAPABILITY(spv::CapabilityPipes, "Pipes")
CAPABILITY(spv::CapabilityGroups, "Groups")
CAPABILITY(spv::CapabilityDeviceEnqueue, "DeviceEnqueue")
CAPABILITY(spv::CapabilityLiteralSampler, "LiteralSampler")
CAPABILITY(spv::CapabilityAtomicStorage, "AtomicStorage")
CAPABILITY(spv::CapabilityInt16, "Int16")
CAPABILITY(spv::CapabilityTessellationPointSize, "TessellationPointSize")
CAPABILITY(spv::CapabilityGeometryPointSize, "GeometryPointSize")
CAPABILITY(spv::CapabilityImageGatherExtended, "ImageGatherExtended")
CAPABILITY(spv::CapabilityStorageImageMultisample, "StorageImageMultisample")
CAPABILITY(spv::CapabilityUniformBufferArrayDynamicIndexing, "UniformBufferArrayDynamicIndexing")
CAPABILITY(spv::CapabilitySampledImageArrayDynamicIndexing, "SampledImageArrayDynamicIndexing")
CAPABILITY(spv::CapabilityStorageBufferArrayDynamicIndexing, "StorageBufferArrayDynamicIndexing")
CAPABILITY(spv::CapabilityStorageImageArrayDynamicIndexing, "StorageImageArrayDynamicIndexing")
CAPABILITY(spv::CapabilityClipDistance, "ClipDistance")
CAPABILITY(spv::CapabilityCullDistance, "CullDistance")
CAPABILITY(spv::CapabilityImageCubeArray, "ImageCubeArray")
CAPABILITY(spv::CapabilitySampleRateShading, "SampleRateShading")
CAPABILITY(spv::CapabilityImageRect, "ImageRect")
CAPABILITY(spv::CapabilitySampledRect, "SampledRect")
CAPABILITY(spv::CapabilityGenericPointer, "GenericPointer")
CAPABILITY(spv::CapabilityInt8, "Int8")
CAPABILITY(spv::CapabilityInputAttachment, "InputAttachment")
CAPABILITY(spv::CapabilitySparseResidency, "SparseResidency")
CAPABILITY(spv::CapabilityMinLod, "MinLod")
CAPABILITY(spv::CapabilitySampled1D, "Sampled1D")
CAPABILITY(spv::CapabilityImage1D, "Image1D")
CAPABILITY(spv::CapabilitySampledCubeArray, "SampledCubeArray")
CAPABILITY(spv::CapabilitySampledBuffer, "SampledBuffer")
CAPABILITY(spv::CapabilityImageBuffer, "ImageBuffer")
CAPABILITY(spv::CapabilityImageMSArray, "ImageMSArray")
CAPABILITY(spv::CapabilityStorageImageExtendedFormats, "StorageImageExtendedFormats")
CAPABILITY(spv::CapabilityImageQuery, "ImageQuery")
CAPABILITY(spv::CapabilityDerivativeControl, "DerivativeControl")
CAPABILITY(spv::CapabilityInterpolationFunction, "InterpolationFunction")
CAPABILITY(spv::CapabilityTransformFeedback, "TransformFeedback")
CAPABILITY(spv::CapabilityGeometryStreams, "GeometryStreams")
CAPABILITY(spv::CapabilityStorageImageReadWithoutFormat, "StorageImageReadWithoutFormat")
CAPABILITY(spv::CapabilityStorageImageWriteWithoutFormat, "StorageImageWriteWithoutFormat")
CAPABILITY(spv::CapabilityMultiViewport, "MultiViewport")
CAPABILITY(spv::CapabilitySubgroupDispatch, "SubgroupDispatch")
CAPABILITY(spv::CapabilityNamedBarrier, "NamedBarrier")
CAPABILITY(spv::CapabilityPipeStorage, "PipeStorage")
CAPABILITY(spv::CapabilityGroupNonUniform, "GroupNonUniform")
CAPABILITY(spv::CapabilityGroupNonUniformVote, "GroupNonUniformVote")
CAPABILITY(spv::CapabilityGroupNonUniformArithmetic, "GroupNonUniformArithmetic")
CAPABILITY(spv::CapabilityGroupNonUniformBallot, "GroupNonUniformBallot")
CAPABILITY(spv::CapabilityGroupNonUniformShuffle, "GroupNonUniformShuffle")
CAPABILITY(spv::CapabilityGroupNonUniformShuffleRelative, "GroupNonUniformShuffleRelative")
CAPABILITY(spv::CapabilityGroupNonUniformClustered, "GroupNonUniformClustered")
CAPABILITY(spv::CapabilityGroupNonUniformQuad, "GroupNonUniformQuad")
CAPABILITY(spv::CapabilityShaderLayer, "ShaderLayer")
CAPABILITY(spv::CapabilityShaderViewportIndex, "ShaderViewportIndex")
CAPABILITY(spv::CapabilityUniformDecoration, "UniformDecoration")
CAPABILITY(spv::CapabilityCoreBuiltinsARM, "CoreBuiltinsARM")
CAPABILITY(spv::CapabilityFragmentShadingRateKHR, "FragmentShadingRateKHR")
CAPABILITY(spv::CapabilitySubgroupBallotKHR, "SubgroupBallotKHR")
CAPABILITY(spv::CapabilityDrawParameters, "DrawParameters")
CAPABILITY(spv::CapabilityWorkgroupMemoryExplicitLayoutKHR, "WorkgroupMemoryExplicitLayoutKHR")
CAPABILITY(spv::CapabilityWorkgroupMemoryExplicitLayout8BitAccessKHR, "WorkgroupMemoryExplicitLayout8BitAccessKHR")
CAPABILITY(spv::CapabilityWorkgroupMemoryExplicitLayout16BitAccessKHR, "WorkgroupMemoryExplicitLayout16BitAccessKHR")
CAPABILITY(spv::CapabilitySubgroupVoteKHR, "SubgroupVoteKHR")
CAPABILITY(spv::CapabilityStorageBuffer16BitAccess, "StorageBuffer16BitAccess")
CAPABILITY(spv::CapabilityStorageUniformBufferBlock16, "StorageUniformBufferBlock16")
CAPABILITY(spv::CapabilityStorageUniform16, "StorageUniform16")
CAPABILITY(spv::CapabilityUniformAndStorageBuffer16BitAccess, "UniformAndStorageBuffer16BitAccess")
CAPABILITY(spv::CapabilityStoragePushConstant16, "StoragePushConstant16")
CAPABILITY(spv::CapabilityStorageInputOutput16, "StorageInputOutput16")
CAPABILITY(spv::CapabilityDeviceGroup, "DeviceGroup")
CAPABILITY(spv::CapabilityMultiView, "MultiView")
CAPABILITY(spv::CapabilityVariablePointersStorageBuffer, "VariablePointersStorageBuffer")
CAPABILITY(spv::CapabilityVariablePointers, "VariablePointers")
CAPABILITY(spv::CapabilityAtomicStorageOps, "AtomicStorageOps")
CAPABILITY(spv::CapabilitySampleMaskPostDepthCoverage, "SampleMaskPostDepthCoverage")
CAPABILITY(spv::CapabilityStorageBuffer8BitAccess, "StorageBuffer8BitAccess")
CAPABILITY(spv::CapabilityUniformAndStorageBuffer8BitAccess, "UniformAndStorageBuffer8BitAccess")
CAPABILITY(spv::CapabilityStoragePushConstant8, "StoragePushConstant8")
CAPABILITY(spv::CapabilityDenormPreserve, "DenormPreserve")
CAPABILITY(spv::CapabilityDenormFlushToZero, "DenormFlushToZero")
CAPABILITY(spv::CapabilitySignedZeroInfNanPreserve, "SignedZeroInfNanPreserve")
CAPABILITY(spv::CapabilityRoundingModeRTE, "RoundingModeRTE")
CAPABILITY(spv::CapabilityRoundingModeRTZ, "RoundingModeRTZ")
CAPABILITY(spv::CapabilityRayQueryProvisionalKHR, "RayQueryProvisionalKHR")
CAPABILITY(spv::CapabilityRayQueryKHR, "RayQueryKHR")
CAPABILITY(spv::CapabilityRayTraversalPrimitiveCullingKHR, "RayTraversalPrimitiveCullingKHR")
CAPABILITY(spv::CapabilityRayTracingKHR, "RayTracingKHR")
CAPABILITY(spv::CapabilityFloat16ImageAMD, "Float16ImageAMD")
CAPABILITY(spv::CapabilityImageGatherBiasLodAMD, "ImageGatherBiasLodAMD")
CAPABILITY(spv::CapabilityFragmentMaskAMD, "FragmentMaskAMD")
CAPABILITY(spv::CapabilityStencilExportEXT, "StencilExportEXT")
CAPABILITY(spv::CapabilityImageReadWriteLodAMD, "ImageReadWriteLodAMD")
CAPABILITY(spv::CapabilityInt64ImageEXT, "Int64ImageEXT")
CAPABILITY(spv::CapabilityShaderClockKHR, "ShaderClockKHR")
CAPABILITY(spv::CapabilitySampleMaskOverrideCoverageNV, "SampleMaskOverrideCoverageNV")
CAPABILITY(spv::CapabilityGeometryShaderPassthroughNV, "GeometryShaderPassthroughNV")
CAPABILITY(spv::CapabilityShaderViewportIndexLayerEXT, "ShaderViewportIndexLayerEXT")
CAPABILITY(spv::CapabilityShaderViewportIndexLayerNV, "ShaderViewportIndexLayerNV")
CAPABILITY(spv::CapabilityShaderViewportMaskNV, "ShaderViewportMaskNV")
CAPABILITY(spv::CapabilityShaderStereoViewNV, "ShaderStereoViewNV")
CAPABILITY(spv::CapabilityPerViewAttributesNV, "PerViewAttributesNV")
CAPABILITY(spv::CapabilityFragmentFullyCoveredEXT, "FragmentFullyCoveredEXT")
CAPABILITY(spv::CapabilityMeshShadingNV, "MeshShadingNV")
CAPABILITY(spv::CapabilityImageFootprintNV, "ImageFootprintNV")
CAPABILITY(spv::CapabilityMeshShadingEXT, "MeshShadingEXT")
CAPABILITY(spv::CapabilityFragmentBarycentricKHR, "FragmentBarycentricKHR")
CAPABILITY(spv::CapabilityFragmentBarycentricNV, "FragmentBarycentricNV")
CAPABILITY(spv::CapabilityComputeDerivativeGroupQuadsNV, "ComputeDerivativeGroupQuadsNV")
CAPABILITY(spv::CapabilityFragmentDensityEXT, "FragmentDensityEXT")
CAPABILITY(spv::CapabilityShadingRateNV, "ShadingRateNV")
CAPABILITY(spv::CapabilityGroupNonUniformPartitionedNV, "GroupNonUniformPartitionedNV")
CAPABILITY(spv::CapabilityShaderNonUniform, "ShaderNonUniform")
CAPABILITY(spv::CapabilityShaderNonUniformEXT, "ShaderNonUniformEXT")
CAPABILITY(spv::CapabilityRuntimeDescriptorArray, "RuntimeDescriptorArray")
CAPABILITY(spv::CapabilityRuntimeDescriptorArrayEXT, "RuntimeDescriptorArrayEXT")
CAPABILITY(spv::CapabilityInputAttachmentArrayDynamicIndexing, "InputAttachmentArrayDynamicIndexing")
CAPABILITY(spv::CapabilityInputAttachmentArrayDynamicIndexingEXT, "InputAttachmentArrayDynamicIndexingEXT")
CAPABILITY(spv::CapabilityUniformTexelBufferArrayDynamicIndexing, "UniformTexelBufferArrayDynamicIndexing")
CAPABILITY(spv::CapabilityUniformTexelBufferArrayDynamicIndexingEXT, "UniformTexelBufferArrayDynamicIndexingEXT")
CAPABILITY(spv::CapabilityStorageTexelBufferArrayDynamicIndexing, "StorageTexelBufferArrayDynamicIndexing")
CAPABILITY(spv::CapabilityStorageTexelBufferArrayDynamicIndexingEXT, "StorageTexelBufferArrayDynamicIndexingEXT")
CAPABILITY(spv::CapabilityUniformBufferArrayNonUniformIndexing, "UniformBufferArrayNonUniformIndexing")
CAPABILITY(spv::CapabilityUniformBufferArrayNonUniformIndexingEXT, "UniformBufferArrayNonUniformIndexingEXT")
CAPABILITY(spv::CapabilitySampledImageArrayNonUniformIndexing, "SampledImageArrayNonUniformIndexing")
CAPABILITY(spv::CapabilitySampledImageArrayNonUniformIndexingEXT, "SampledImageArrayNonUniformIndexingEXT")
CAPABILITY(spv::CapabilityStorageBufferArrayNonUniformIndexing, "StorageBufferArrayNonUniformIndexing")
CAPABILITY(spv::CapabilityStorageBufferArrayNonUniformIndexingEXT, "StorageBufferArrayNonUniformIndexingEXT")
CAPABILITY(spv::CapabilityStorageImageArrayNonUniformIndexing, "StorageImageArrayNonUniformIndexing")
CAPABILITY(spv::CapabilityStorageImageArrayNonUniformIndexingEXT, "StorageImageArrayNonUniformIndexingEXT")
CAPABILITY(spv::CapabilityInputAttachmentArrayNonUniformIndexing, "InputAttachmentArrayNonUniformIndexing")
CAPABILITY(spv::CapabilityInputAttachmentArrayNonUniformIndexingEXT, "InputAttachmentArrayNonUniformIndexingEXT")
CAPABILITY(spv::CapabilityUniformTexelBufferArrayNonUniformIndexing, "UniformTexelBufferArrayNonUniformIndexing")
CAPABILITY(spv::CapabilityUniformTexelBufferArrayNonUniformIndexingEXT, "UniformTexelBufferArrayNonUniformIndexingEXT")
CAPABILITY(spv::CapabilityStorageTexelBufferArrayNonUniformIndexing, "StorageTexelBufferArrayNonUniformIndexing")
CAPABILITY(spv::CapabilityStorageTexelBufferArrayNonUniformIndexingEXT, "StorageTexelBufferArrayNonUniformIndexingEXT")
CAPABILITY(spv::CapabilityRayTracingNV, "RayTracingNV")
CAPABILITY(spv::CapabilityRayTracingMotionBlurNV, "RayTracingMotionBlurNV")
CAPABILITY(spv::CapabilityVulkanMemoryModel, "VulkanMemoryModel")
CAPABILITY(spv::CapabilityVulkanMemoryModelKHR, "VulkanMemoryModelKHR")
CAPABILITY(spv::CapabilityVulkanMemoryModelDeviceScope, "VulkanMemoryModelDeviceScope")
CAPABILITY(spv::CapabilityVulkanMemoryModelDeviceScopeKHR, "VulkanMemoryModelDeviceScopeKHR")
CAPABILITY(spv::CapabilityPhysicalStorageBufferAddresses, "PhysicalStorageBufferAddresses")
CAPABILITY(spv::CapabilityPhysicalStorageBufferAddressesEXT, "PhysicalStorageBufferAddressesEXT")
CAPABILITY(spv::CapabilityComputeDerivativeGroupLinearNV, "ComputeDerivativeGroupLinearNV")
CAPABILITY(spv::CapabilityRayTracingProvisionalKHR, "RayTracingProvisionalKHR")
CAPABILITY(spv::CapabilityCooperativeMatrixNV, "CooperativeMatrixNV")
CAPABILITY(spv::CapabilityFragmentShaderSampleInterlockEXT, "FragmentShaderSampleInterlockEXT")
CAPABILITY(spv::CapabilityFragmentShaderShadingRateInterlockEXT, "FragmentShaderShadingRateInterlockEXT")
CAPABILITY(spv::CapabilityShaderSMBuiltinsNV, "ShaderSMBuiltinsNV")
CAPABILITY(spv::CapabilityFragmentShaderPixelInterlockEXT, "FragmentShaderPixelInterlockEXT")
CAPABILITY(spv::CapabilityDemoteToHelperInvocation, "DemoteToHelperInvocation")
CAPABILITY(spv::CapabilityDemoteToHelperInvocationEXT, "DemoteToHelperInvocationEXT")
CAPABILITY(spv::CapabilityRayTracingOpacityMicromapEXT, "RayTracingOpacityMicromapEXT")
CAPABILITY(spv::CapabilityShaderInvocationReorderNV, "ShaderInvocationReorderNV")
CAPABILITY(spv::CapabilityBindlessTextureNV, "BindlessTextureNV")
CAPABILITY(spv::CapabilitySubgroupShuffleINTEL, "SubgroupShuffleINTEL")
CAPABILITY(spv::CapabilitySubgroupBufferBlockIOINTEL, "SubgroupBufferBlockIOINTEL")
CAPABILITY(spv::CapabilitySubgroupImageBlockIOINTEL, "SubgroupImageBlockIOINTEL")
CAPABILITY(spv::CapabilitySubgroupImageMediaBlockIOINTEL, "SubgroupImageMediaBlockIOINTEL")
CAPABILITY(spv::CapabilityRoundToInfinityINTEL, "RoundToInfinityINTEL")
CAPABILITY(spv::CapabilityFloatingPointModeINTEL, "FloatingPointModeINTEL")
CAPABILITY(spv::CapabilityIntegerFunctions2INTEL, "IntegerFunctions2INTEL")
CAPABILITY(spv::CapabilityFunctionPointersINTEL, "FunctionPointersINTEL")
CAPABILITY(spv::CapabilityIndirectReferencesINTEL, "IndirectReferencesINTEL")
CAPABILITY(spv::CapabilityAsmINTEL, "AsmINTEL")
CAPABILITY(spv::CapabilityAtomicFloat32MinMaxEXT, "AtomicFloat32MinMaxEXT")
CAPABILITY(spv::CapabilityAtomicFloat64MinMaxEXT, "AtomicFloat64MinMaxEXT")
CAPABILITY(spv::CapabilityAtomicFloat16MinMaxEXT, "AtomicFloat16MinMaxEXT")
CAPABILITY(spv::CapabilityVectorComputeINTEL, "VectorComputeINTEL")
CAPABILITY(spv::CapabilityVectorAnyINTEL, "VectorAnyINTEL")
CAPABILITY(spv::CapabilityExpectAssumeKHR, "ExpectAssumeKHR")
CAPABILITY(spv::CapabilitySubgroupAvcMotionEstimationINTEL, "SubgroupAvcMotionEstimationINTEL")
CAPABILITY(spv::CapabilitySubgroupAvcMotionEstimationIntraINTEL, "SubgroupAvcMotionEstimationIntraINTEL")
CAPABILITY(spv::CapabilitySubgroupAvcMotionEstimationChromaINTEL, "SubgroupAvcMotionEstimationChromaINTEL")
CAPABILITY(spv::CapabilityVariableLengthArrayINTEL, "VariableLengthArrayINTEL")
CAPABILITY(spv::CapabilityFunctionFloatControlINTEL, "FunctionFloatControlINTEL")
CAPABILITY(spv::CapabilityFPGAMemoryAttributesINTEL, "FPGAMemoryAttributesINTEL")
CAPABILITY(spv::CapabilityFPFastMathModeINTEL, "FPFastMathModeINTEL")
CAPABILITY(spv::CapabilityArbitraryPrecisionIntegersINTEL, "ArbitraryPrecisionIntegersINTEL")
CAPABILITY(spv::CapabilityArbitraryPrecisionFloatingPointINTEL, "ArbitraryPrecisionFloatingPointINTEL")
CAPABILITY(spv::CapabilityUnstructuredLoopControlsINTEL, "UnstructuredLoopControlsINTEL")
CAPABILITY(spv::CapabilityFPGALoopControlsINTEL, "FPGALoopControlsINTEL")
CAPABILITY(spv::CapabilityKernelAttributesINTEL, "KernelAttributesINTEL")
CAPABILITY(spv::CapabilityFPGAKernelAttributesINTEL, "FPGAKernelAttributesINTEL")
CAPABILITY(spv::CapabilityFPGAMemoryAccessesINTEL, "FPGAMemoryAccessesINTEL")
CAPABILITY(spv::CapabilityFPGAClusterAttributesINTEL, "FPGAClusterAttributesINTEL")
CAPABILITY(spv::CapabilityLoopFuseINTEL, "LoopFuseINTEL")
CAPABILITY(spv::CapabilityFPGADSPControlINTEL, "FPGADSPControlINTEL")
CAPABILITY(spv::CapabilityMemoryAccessAliasingINTEL, "MemoryAccessAliasingINTEL")
CAPABILITY(spv::CapabilityFPGAInvocationPipeliningAttributesINTEL, "FPGAInvocationPipeliningAttributesINTEL")
CAPABILITY(spv::CapabilityFPGABufferLocationINTEL, "FPGABufferLocationINTEL")
CAPABILITY(spv::CapabilityArbitraryPrecisionFixedPointINTEL, "ArbitraryPrecisionFixedPointINTEL")
CAPABILITY(spv::CapabilityUSMStorageClassesINTEL, "USMStorageClassesINTEL")
CAPABILITY(spv::CapabilityRuntimeAlignedAttributeINTEL, "RuntimeAlignedAttributeINTEL")
CAPABILITY(spv::CapabilityIOPipesINTEL, "IOPipesINTEL")
CAPABILITY(spv::CapabilityBlockingPipesINTEL, "BlockingPipesINTEL")
CAPABILITY(spv::CapabilityFPGARegINTEL, "FPGARegINTEL")
CAPABILITY(spv::CapabilityDotProductInputAll, "DotProductInputAll")
CAPABILITY(spv::CapabilityDotProductInputAllKHR, "DotProductInputAllKHR")
CAPABILITY(spv::CapabilityDotProductInput4x8Bit, "DotProductInput4x8Bit")
CAPABILITY(spv::CapabilityDotProductInput4x8BitKHR, "DotProductInput4x8BitKHR")
CAPABILITY(spv::CapabilityDotProductInput4x8BitPacked, "DotProductInput4x8BitPacked")
CAPABILITY(spv::CapabilityDotProductInput4x8BitPackedKHR, "DotProductInput4x8BitPackedKHR")
CAPABILITY(spv::CapabilityDotProduct, "DotProduct")
CAPABILITY(spv::CapabilityDotProductKHR, "DotProductKHR")
CAPABILITY(spv::CapabilityRayCullMaskKHR, "RayCullMaskKHR")
CAPABILITY(spv::CapabilityBitInstructions, "BitInstructions")
CAPABILITY(spv::CapabilityGroupNonUniformRotateKHR, "GroupNonUniformRotateKHR")
CAPABILITY(spv::CapabilityAtomicFloat32AddEXT, "AtomicFloat32AddEXT")
CAPABILITY(spv::CapabilityAtomicFloat64AddEXT, "AtomicFloat64AddEXT")
CAPABILITY(spv::CapabilityLongCompositesINTEL, "LongCompositesINTEL")
CAPABILITY(spv::CapabilityOptNoneINTEL, "OptNoneINTEL")
CAPABILITY(spv::CapabilityAtomicFloat16AddEXT, "AtomicFloat16AddEXT")
CAPABILITY(spv::CapabilityDebugInfoModuleINTEL, "DebugInfoModuleINTEL")
CAPABILITY(spv::CapabilityBFloat16ConversionINTEL, "BFloat16ConversionINTEL")
CAPABILITY(spv::CapabilitySplitBarrierINTEL, "SplitBarrierINTEL")
CAPABILITY(spv::CapabilityFPGAClusterAttributesV2INTEL, "FPGAClusterAttributesV2INTEL")
CAPABILITY(spv::CapabilityFPGAKernelAttributesv2INTEL, "FPGAKernelAttributesv2INTEL")
CAPABILITY(spv::CapabilityFPMaxErrorINTEL, "FPMaxErrorINTEL")
CAPABILITY(spv::CapabilityFPGALatencyControlINTEL, "FPGALatencyControlINTEL")
CAPABILITY(spv::CapabilityFPGAArgumentInterfacesINTEL, "FPGAArgumentInterfacesINTEL")
CAPABILITY(spv::CapabilityGlobalVariableHostAccessINTEL, "GlobalVariableHostAccessINTEL")
CAPABILITY(spv::CapabilityGlobalVariableFPGADecorationsINTEL, "GlobalVariableFPGADecorationsINTEL")
CAPABILITY(spv::CapabilityGroupUniformArithmeticKHR, "GroupUniformArithmeticKHR")
CAPABILITY(spv::CapabilityCacheControlsINTEL, "CacheControlsINTEL")

#undef CAPABILITY
