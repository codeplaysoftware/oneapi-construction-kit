; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %veczc -w 4 -S -vecz-passes=packetizer -vecz-choices=VectorPredication < %s | %filecheck %s 

; Tests the use of the VectorPredication choice. However, note that this option
; currently makes no difference on fixed length vectors.

target triple = "spir64-unknown-unknown"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

declare spir_func i64 @_Z13get_global_idj(i32)

declare spir_func i32 @_Z28sub_group_scan_inclusive_addi(i32)
declare spir_func i64 @_Z28sub_group_scan_inclusive_addl(i64)
declare spir_func float @_Z28sub_group_scan_inclusive_addf(float)

declare spir_func i32 @_Z28sub_group_scan_inclusive_mini(i32)
declare spir_func i32 @_Z28sub_group_scan_inclusive_minj(i32)
declare spir_func i32 @_Z28sub_group_scan_inclusive_maxi(i32)
declare spir_func i32 @_Z28sub_group_scan_inclusive_maxj(i32)
declare spir_func float @_Z28sub_group_scan_inclusive_minf(float)
declare spir_func float @_Z28sub_group_scan_inclusive_maxf(float)

define spir_kernel void @reduce_scan_incl_add_i32(i32 addrspace(1)* %in, i32 addrspace(1)* %out) {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %in, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %call1 = tail call spir_func i32 @_Z28sub_group_scan_inclusive_addi(i32 %0)
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %out, i64 %call
  store i32 %call1, i32 addrspace(1)* %arrayidx2, align 4
  ret void
; CHECK-LABEL: @__vecz_v4_vp_reduce_scan_incl_add_i32(
; CHECK: call <4 x i32> @__vecz_b_sub_group_scan_inclusive_add_Dv4_j(<4 x i32> %{{.*}})
}

define spir_kernel void @reduce_scan_incl_add_i64(i64 addrspace(1)* %in, i64 addrspace(1)* %out) {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidx = getelementptr inbounds i64, i64 addrspace(1)* %in, i64 %call
  %0 = load i64, i64 addrspace(1)* %arrayidx, align 4
  %call1 = tail call spir_func i64 @_Z28sub_group_scan_inclusive_addl(i64 %0)
  %arrayidx2 = getelementptr inbounds i64, i64 addrspace(1)* %out, i64 %call
  store i64 %call1, i64 addrspace(1)* %arrayidx2, align 4
  ret void
; CHECK-LABEL: @__vecz_v4_vp_reduce_scan_incl_add_i64(
; CHECK: call <4 x i64> @__vecz_b_sub_group_scan_inclusive_add_Dv4_m(<4 x i64> %{{.*}})
}

define spir_kernel void @reduce_scan_incl_add_f32(float addrspace(1)* %in, float addrspace(1)* %out) {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %in, i64 %call
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %call1 = tail call spir_func float @_Z28sub_group_scan_inclusive_addf(float %0)
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %out, i64 %call
  store float %call1, float addrspace(1)* %arrayidx2, align 4
  ret void
; CHECK-LABEL: @__vecz_v4_vp_reduce_scan_incl_add_f32(
; CHECK: call <4 x float> @__vecz_b_sub_group_scan_inclusive_add_Dv4_f(<4 x float> %{{.*}})
}

define spir_kernel void @reduce_scan_incl_smin_i32(i32 addrspace(1)* %in, i32 addrspace(1)* %out) {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %in, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %call1 = tail call spir_func i32 @_Z28sub_group_scan_inclusive_mini(i32 %0)
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %out, i64 %call
  store i32 %call1, i32 addrspace(1)* %arrayidx2, align 4
  ret void
; CHECK-LABEL: @__vecz_v4_vp_reduce_scan_incl_smin_i32(
; CHECK: call <4 x i32> @__vecz_b_sub_group_scan_inclusive_smin_Dv4_i(<4 x i32> %{{.*}})
}

define spir_kernel void @reduce_scan_incl_umin_i32(i32 addrspace(1)* %in, i32 addrspace(1)* %out) {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %in, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %call1 = tail call spir_func i32 @_Z28sub_group_scan_inclusive_minj(i32 %0)
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %out, i64 %call
  store i32 %call1, i32 addrspace(1)* %arrayidx2, align 4
  ret void
; CHECK-LABEL: @__vecz_v4_vp_reduce_scan_incl_umin_i32(
; CHECK: call <4 x i32> @__vecz_b_sub_group_scan_inclusive_umin_Dv4_j(<4 x i32> %{{.*}})
}

define spir_kernel void @reduce_scan_incl_smax_i32(i32 addrspace(1)* %in, i32 addrspace(1)* %out) {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %in, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %call1 = tail call spir_func i32 @_Z28sub_group_scan_inclusive_maxi(i32 %0)
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %out, i64 %call
  store i32 %call1, i32 addrspace(1)* %arrayidx2, align 4
  ret void
; CHECK-LABEL: @__vecz_v4_vp_reduce_scan_incl_smax_i32(
; CHECK: call <4 x i32> @__vecz_b_sub_group_scan_inclusive_smax_Dv4_i(<4 x i32> %{{.*}})
}

define spir_kernel void @reduce_scan_incl_umax_i32(i32 addrspace(1)* %in, i32 addrspace(1)* %out) {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %in, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %call1 = tail call spir_func i32 @_Z28sub_group_scan_inclusive_maxj(i32 %0)
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %out, i64 %call
  store i32 %call1, i32 addrspace(1)* %arrayidx2, align 4
  ret void
; CHECK-LABEL: @__vecz_v4_vp_reduce_scan_incl_umax_i32(
; CHECK: call <4 x i32> @__vecz_b_sub_group_scan_inclusive_umax_Dv4_j(<4 x i32> %{{.*}})
}

define spir_kernel void @reduce_scan_incl_fmin_f32(float addrspace(1)* %in, float addrspace(1)* %out) {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %in, i64 %call
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %call1 = tail call spir_func float @_Z28sub_group_scan_inclusive_minf(float %0)
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %out, i64 %call
  store float %call1, float addrspace(1)* %arrayidx2, align 4
  ret void
; CHECK-LABEL: @__vecz_v4_vp_reduce_scan_incl_fmin_f32(
; CHECK: call <4 x float> @__vecz_b_sub_group_scan_inclusive_min_Dv4_f(<4 x float> %{{.*}})
}

define spir_kernel void @reduce_scan_incl_fmax_f32(float addrspace(1)* %in, float addrspace(1)* %out) {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %in, i64 %call
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %call1 = tail call spir_func float @_Z28sub_group_scan_inclusive_maxf(float %0)
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %out, i64 %call
  store float %call1, float addrspace(1)* %arrayidx2, align 4
  ret void
; CHECK-LABEL: @__vecz_v4_vp_reduce_scan_incl_fmax_f32(
; CHECK: call <4 x float> @__vecz_b_sub_group_scan_inclusive_max_Dv4_f(<4 x float> %{{.*}})
}