; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; REQUIRES: llvm-13+
; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -vecz-scalable -vecz-simd-width=4 -vecz-choices=VectorPredication -S < %s | %filecheck %t

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown"

declare spir_func i64 @_Z13get_global_idj(i32)

; With VP all gathers become masked ones.
define spir_kernel void @unmasked_gather(i32 addrspace(1)* %a, i32 addrspace(1)* %b) {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 0)
  %rem = urem i64 %call, 3
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %rem
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %arrayidx3 = getelementptr inbounds i32, i32 addrspace(1)* %b, i64 %call
  store i32 %0, i32 addrspace(1)* %arrayidx3, align 4
  ret void
}

; CHECK: define spir_kernel void @__vecz_nxv4_vp_unmasked_gather(
; CHECK-GE15: [[v:%.*]] = call <vscale x 4 x i32> @__vecz_b_masked_gather_load4_vp_u5nxv4ju14nxv4u3ptrU3AS1u5nxv4bj(<vscale x 4 x ptr addrspace(1)> %{{.*}})
; CHECK-LT15: [[v:%.*]] = call <vscale x 4 x i32> @__vecz_b_masked_gather_load4_vp_u5nxv4ju11nxv4PU3AS1ju5nxv4bj(<vscale x 4 x i32 addrspace(1)*> %{{.*}})
; CHECK-GE15: call void @llvm.vp.store.nxv4i32.p1(<vscale x 4 x i32> [[v]],
; CHECK-LT15: call void @llvm.vp.store.nxv4i32.p1nxv4i32(<vscale x 4 x i32> [[v]],


; With VP all scatters become masked ones.
define spir_kernel void @unmasked_scatter(i32 addrspace(1)* %a, i32 addrspace(1)* %b) {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 0)
  %rem = urem i64 %call, 3
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %arrayidx3 = getelementptr inbounds i32, i32 addrspace(1)* %b, i64 %rem
  store i32 %0, i32 addrspace(1)* %arrayidx3, align 4
  ret void
}

; CHECK: define spir_kernel void @__vecz_nxv4_vp_unmasked_scatter(
; CHECK-GE15: [[v:%.*]] = call <vscale x 4 x i32> @llvm.vp.load.nxv4i32.p1(
; CHECK-LT15: [[v:%.*]] = call <vscale x 4 x i32> @llvm.vp.load.nxv4i32.p1nxv4i32(
; CHECK-GE15: call void @__vecz_b_masked_scatter_store4_vp_u5nxv4ju14nxv4u3ptrU3AS1u5nxv4bj(<vscale x 4 x i32> [[v]],
; CHECK-LT15: call void @__vecz_b_masked_scatter_store4_vp_u5nxv4ju11nxv4PU3AS1ju5nxv4bj(<vscale x 4 x i32> [[v]],

; CHECK-GE15: define <vscale x 4 x i32> @__vecz_b_masked_gather_load4_vp_u5nxv4ju14nxv4u3ptrU3AS1u5nxv4bj(<vscale x 4 x ptr addrspace(1)> %0, <vscale x 4 x i1> %1, i32 %2) {
; CHECK-LT15: define <vscale x 4 x i32> @__vecz_b_masked_gather_load4_vp_u5nxv4ju11nxv4PU3AS1ju5nxv4bj(<vscale x 4 x i32 addrspace(1)*> %0, <vscale x 4 x i1> %1, i32 %2) {
; CHECK-GE15:   %3 = call <vscale x 4 x i32> @llvm.vp.gather.nxv4i32.nxv4p1(<vscale x 4 x ptr addrspace(1)> %0, <vscale x 4 x i1> %1, i32 %2)
; CHECK-LT15:   %3 = call <vscale x 4 x i32> @llvm.vp.gather.nxv4i32.nxv4p1i32(<vscale x 4 x i32 addrspace(1)*> %0, <vscale x 4 x i1> %1, i32 %2)
; CHECK:   ret <vscale x 4 x i32> %3

; CHECK-GE15: define void @__vecz_b_masked_scatter_store4_vp_u5nxv4ju14nxv4u3ptrU3AS1u5nxv4bj(<vscale x 4 x i32> %0, <vscale x 4 x ptr addrspace(1)> %1, <vscale x 4 x i1> %2, i32 %3) {
; CHECK-LT15: define void @__vecz_b_masked_scatter_store4_vp_u5nxv4ju11nxv4PU3AS1ju5nxv4bj(<vscale x 4 x i32> %0, <vscale x 4 x i32 addrspace(1)*> %1, <vscale x 4 x i1> %2, i32 %3) {
; CHECK: entry:
; CHECK-GE15:   call void @llvm.vp.scatter.nxv4i32.nxv4p1(<vscale x 4 x i32> %0, <vscale x 4 x ptr addrspace(1)> %1, <vscale x 4 x i1> %2, i32 %3)
; CHECK-LT15:   call void @llvm.vp.scatter.nxv4i32.nxv4p1i32(<vscale x 4 x i32> %0, <vscale x 4 x i32 addrspace(1)*> %1, <vscale x 4 x i1> %2, i32 %3)
; CHECK:   ret void