# Copyright (C) Codeplay Software Limited
#
# Licensed under the Apache License, Version 2.0 (the "License") with LLVM
# Exceptions; you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://github.com/codeplaysoftware/oneapi-construction-kit/blob/main/LICENSE.txt
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# If kernel.cl files are added from external targets. Append them to this list.
foreach(NAME ${MUX_TARGET_LIBRARIES})
  set(CA_EXTERNAL_UNITVK_KERNEL_FILES
    ${CA_EXTERNAL_UNITVK_KERNEL_FILES}
    ${${NAME}_UNITVK_KERNEL_FILES})
endforeach()

set(UVK_CL_FILES
  task_01.01_copy.cl
  task_01.02_add.cl
  task_01.03_mul_fma.cl
  task_01.04_ternary.cl
  task_01.05_broadcast.cl
  task_01.06_broadcast_uniform.cl
  task_02.01_abs_builtin.cl
  task_02.02_dot_builtin.cl
  task_02.03_distance_builtin.cl
  task_02.04_fabs_builtin.cl
  task_02.05_clz_builtin.cl
  task_02.06_clamp_builtin.cl
  task_02.07_length_builtin.cl
  task_02.08_barrier_add.cl
  task_03.01_copy4.cl
  task_03.02_add4.cl
  task_03.03_abs4_builtin.cl
  task_03.04_dot4_builtin.cl
  task_03.05_distance4_builtin.cl
  task_03.06_ternary4.cl
  task_03.07_transpose4.cl
  task_03.08_clz4_builtin.cl
  task_03.09_clamp4_builtin.cl
  task_03.10_s2v_int.cl
  task_03.11_sum_reduce4.cl
  task_03.12_v2s2v2s.cl
  task_03.13_copy2.cl
  task_03.14_add2.cl
  task_03.17_length4_builtin.cl
  task_03.19_add4_i32_tid.cl
  task_03.27_atomic_inc_builtin.cl
  task_04.01_copy_constant_offset.cl
  task_04.02_copy_uniform_offset.cl
  task_04.03_mul_fma_uniform_offset_load.cl
  task_04.04_mul_fma_uniform_offset_store.cl
  task_04.05_scatter.cl
  task_04.06_gather.cl
  task_04.07_mul_fma_uniform_addr_load.cl
  task_04.08_mul_fma_uniform_addr_store.cl
  task_04.09_copy4_scalarized.cl
  task_04.10_alloca.cl
  task_04.11_byval_struct.cl
  task_04.13_struct_offset.cl
  task_04.14_alloca4.cl
  task_04.15_scatter_offset.cl
  task_04.16_gather_offset.cl
  task_04.17_local_array.cl
  task_04.18_private_array.cl
  task_05.01_sum_static_trip.cl
  task_05.02_saxpy_static_trip.cl
  task_05.03_sum_static_trip_uniform.cl
  task_05.04_saxpy_static_trip_uniform.cl
  task_06.01_copy_if_constant.cl
  task_06.02_copy_if_even_group.cl
  task_07.01_copy_if_even_item.cl
  task_07.02_copy_if_nested_item.cl
  task_07.03_add_no_nan.cl
  task_07.05_ternary_pointer.cl
  task_07.06_copy_if_even_item_phi.cl
  task_07.07_masked_loop_uniform.cl
  task_07.08_masked_loop_varying.cl
  task_07.09_control_dep_packetization.cl
  task_07.10_control_dep_scalarization.cl
  task_07.11_copy_if_even_item_early_return.cl
  task_07.12_scalar_masked_load.cl
  task_07.13_scalar_masked_store_uniform.cl
  task_07.14_scalar_masked_store_varying.cl
  task_07.15_normalize_range.cl
  task_07.16_normalize_range_while.cl
  task_07.17_if_in_loop.cl
  task_07.18_if_in_uniform_loop.cl
  task_07.19_nested_loops.cl
  task_07.20_sibling_loops.cl
  task_07.21_convert_half_to_float_impl.cl
  task_07.23_convert_half_to_float_nested_ifs.cl
  task_08.01_user_fn_identity.cl
  task_08.02_user_fn_sext.cl
  task_08.03_user_fn_two_contexts.cl
  task_09.01_masked_interleaved_store.cl
  task_09.02_masked_interleaved_load.cl
  task_09.03_masked_scatter.cl
  task_09.04_masked_gather.cl
  task_09.05_masked_argument_stride.cl
  task_09.06_masked_negative_stride.cl
  task_09.07_masked_negative_argument_stride.cl
  task_09.08_phi_memory.cl
  task_10.03_vector_loop.cl
  task_10.05_atomic_cmpxchg_builtin.cl
  task_10.07_break_loop.cl
  task_10.08_insertelement_constant_index.cl
  task_10.09_insertelement_runtime_index.cl
  task_10.10_extractelement_constant_index.cl
  task_10.11_extractelement_runtime_index.cl
  dma.01_direct.cl
  dma.06_auto_dma_convolution.cl
  dma.07_auto_dma_loop_convolution.cl
  dma.08_auto_dma_loop_convolution_cond_round_inner_loop.cl
  dma.09_auto_dma_loop_convolution_cond_not_global_id.cl
  regression.06_cross_elem4_zero.cl
  regression.10_dont_mask_workitem_builtins.cl
  regression.14_argument_stride.cl
  regression.15_negative_stride.cl
  regression.16_negative_argument_stride.cl
  regression.17_scalar_select_transform.cl
  regression.18_uniform_alloca.cl
  regression.19_memcpy_optimization.cl
  regression.28_uniform_atomics.cl
  regression.29_divergent_memfence.cl
  regression.34_codegen_1.cl
  regression.34_codegen_2.cl
  regression.37_cfc.cl
  regression.43_scatter_gather.cl
  regression.51_local_phi.cl
  regression.52_nested_loop_using_kernel_arg.cl
  regression.54_negative_comparison.cl
  ${CA_EXTERNAL_UNITVK_KERNEL_FILES})
