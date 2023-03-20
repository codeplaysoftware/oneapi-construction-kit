; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; REQUIRES: llvm-13+
; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -vecz-scalable -vecz-simd-width=4 -S < %s | %filecheck %t

target triple = "spir64-unknown-unknown"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

define spir_kernel void @do_shuffle_splat(i32* %aptr, <4 x i32>* %bptr, <4 x i32>* %zptr) {
  %idx = call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidxa = getelementptr inbounds i32, i32* %aptr, i64 %idx
  %arrayidxb = getelementptr inbounds <4 x i32>, <4 x i32>* %bptr, i64 %idx
  %a = load i32, i32* %arrayidxa, align 4
  %b = load <4 x i32>, <4 x i32>* %arrayidxb, align 16
  %insert = insertelement <4 x i32> undef, i32 %a, i32 0
  %splat = shufflevector <4 x i32> %insert, <4 x i32> undef, <4 x i32> zeroinitializer
  %arrayidxz = getelementptr inbounds <4 x i32>, <4 x i32>* %zptr, i64 %idx
  store <4 x i32> %splat, <4 x i32>* %arrayidxz
  ret void
; CHECK: define spir_kernel void @__vecz_nxv4_do_shuffle_splat
; CHECK: [[idx0:%.*]] = call <vscale x 16 x i32> @llvm.experimental.stepvector.nxv16i32()
; CHECK: [[idx1:%.*]] = lshr <vscale x 16 x i32> [[idx0]], shufflevector (<vscale x 16 x i32> insertelement (<vscale x 16 x i32> {{(undef|poison)}}, i32 2, {{(i32|i64)}} 0), <vscale x 16 x i32> {{(undef|poison)}}, <vscale x 16 x i32> zeroinitializer)
; CHECK: [[idx2:%.*]] = sext <vscale x 16 x i32> [[idx1]] to <vscale x 16 x i64>
; CHECK-GE15: [[alloc:%.*]] = getelementptr inbounds i32, ptr %{{.*}}, <vscale x 16 x i64> [[idx2]]
; CHECK-LT15: [[alloc:%.*]] = getelementptr inbounds i32, i32* %{{.*}}, <vscale x 16 x i64> [[idx2]]
; CHECK-GE15: [[splat:%.*]] = call <vscale x 16 x i32> @llvm.masked.gather.nxv16i32.nxv16p0(<vscale x 16 x ptr> [[alloc]],
; CHECK-LT15: [[splat:%.*]] = call <vscale x 16 x i32> @llvm.masked.gather.nxv16i32.nxv16p0i32(<vscale x 16 x i32*> [[alloc]],
; CHECK-GE15: store <vscale x 16 x i32> [[splat]], ptr
; CHECK-LT15: store <vscale x 16 x i32> [[splat]], <vscale x 16 x i32>*
}

define spir_kernel void @do_shuffle_splat_uniform(i32 %a, <4 x i32>* %bptr, <4 x i32>* %zptr) {
  %idx = call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidxb = getelementptr inbounds <4 x i32>, <4 x i32>* %bptr, i64 %idx
  %b = load <4 x i32>, <4 x i32>* %arrayidxb, align 16
  %insert = insertelement <4 x i32> undef, i32 %a, i32 0
  %splat = shufflevector <4 x i32> %insert, <4 x i32> undef, <4 x i32> zeroinitializer
  %arrayidxz = getelementptr inbounds <4 x i32>, <4 x i32>* %zptr, i64 %idx
  store <4 x i32> %splat, <4 x i32>* %arrayidxz
  ret void
; CHECK: define spir_kernel void @__vecz_nxv4_do_shuffle_splat_uniform
; CHECK: [[ins:%.*]] = insertelement <vscale x 16 x i32> poison, i32 %a, {{(i32|i64)}} 0
; CHECK: [[splat:%.*]] = shufflevector <vscale x 16 x i32> [[ins]], <vscale x 16 x i32> {{(undef|poison)}}, <vscale x 16 x i32> zeroinitializer
; CHECK-GE15: store <vscale x 16 x i32> [[splat]], ptr
; CHECK-LT15: store <vscale x 16 x i32> [[splat]], <vscale x 16 x i32>*
}

declare spir_func i64 @_Z13get_global_idj(i32)