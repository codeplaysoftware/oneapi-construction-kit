; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; REQUIRES: llvm-12+
; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -vecz-simd-width=4 -S < %s | %filecheck %t

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown"

declare spir_func i64 @_Z13get_global_idj(i32)

declare i32 @llvm.abs.i32(i32, i1)
declare <2 x i32> @llvm.abs.v2i32(<2 x i32>, i1)

define spir_kernel void @absff(i32* %pa, i32* %pb) {
entry:
  %idx = call spir_func i64 @_Z13get_global_idj(i32 0)
  %a = getelementptr i32, i32* %pa, i64 %idx
  %b = getelementptr i32, i32* %pb, i64 %idx
  %la = load i32, i32* %a, align 16
  %res = call spir_func i32 @llvm.abs.i32(i32 %la, i1 true)
  store i32 %res, i32* %b, align 16
  ret void
}

define spir_kernel void @absvf(<2 x i32>* %pa, <2 x i32>* %pb) {
entry:
  %idx = call spir_func i64 @_Z13get_global_idj(i32 0)
  %a = getelementptr <2 x i32>, <2 x i32>* %pa, i64 %idx
  %b = getelementptr <2 x i32>, <2 x i32>* %pb, i64 %idx
  %la = load <2 x i32>, <2 x i32>* %a, align 16
  %res = call spir_func <2 x i32> @llvm.abs.v2i32(<2 x i32> %la, i1 true)
  store <2 x i32> %res, <2 x i32>* %b, align 16
  ret void
}

; CHECK-GE15: define spir_kernel void @__vecz_v4_absff(ptr %pa, ptr %pb)
; CHECK-LT15: define spir_kernel void @__vecz_v4_absff(i32* %pa, i32* %pb)
; CHECK: entry:
; CHECK: %idx = call spir_func i64 @_Z13get_global_idj(i32 0)
; CHECK-GE15: %a = getelementptr i32, ptr %pa, i64 %idx
; CHECK-LT15: %a = getelementptr i32, i32* %pa, i64 %idx
; CHECK-GE15: %b = getelementptr i32, ptr %pb, i64 %idx
; CHECK-LT15: %b = getelementptr i32, i32* %pb, i64 %idx
; CHECK-LT15: %0 = bitcast i32* %a to <4 x i32>*
; CHECK-GE15: %[[T0:.*]] = load <4 x i32>, ptr %a, align 4
; CHECK-LT15: %[[T0:.*]] = load <4 x i32>, <4 x i32>* %0, align 4
; CHECK: %[[RES1:.+]] = call <4 x i32> @llvm.abs.v4i32(<4 x i32> %[[T0]], i1 true)
; CHECK-GE15: store <4 x i32> %[[RES1]], ptr %b, align 4
; CHECK-LT15: %2 = bitcast i32* %b to <4 x i32>*
; CHECK-LT15: store <4 x i32> %[[RES1]], <4 x i32>* %2, align 4
; CHECK: ret void

; CHECK-GE15: define spir_kernel void @__vecz_v4_absvf(ptr %pa, ptr %pb)
; CHECK-LT15: define spir_kernel void @__vecz_v4_absvf(<2 x i32>* %pa, <2 x i32>* %pb)
; CHECK: entry:
; CHECK: %idx = call spir_func i64 @_Z13get_global_idj(i32 0)
; CHECK-GE15: %a = getelementptr <2 x i32>, ptr %pa, i64 %idx
; CHECK-LT15: %a = getelementptr <2 x i32>, <2 x i32>* %pa, i64 %idx
; CHECK-GE15: %b = getelementptr <2 x i32>, ptr %pb, i64 %idx
; CHECK-LT15: %b = getelementptr <2 x i32>, <2 x i32>* %pb, i64 %idx
; CHECK-LT15: %0 = bitcast <2 x i32>* %a to <8 x i32>*
; CHECK-GE15: %[[T0:.*]] = load <8 x i32>, ptr %a, align 4
; CHECK-LT15: %[[T0:.*]] = load <8 x i32>, <8 x i32>* %0, align 4
; CHECK: %[[RES2:.+]] = call <8 x i32> @llvm.abs.v8i32(<8 x i32> %[[T0]], i1 true)
; CHECK-LT15: %2 = bitcast <2 x i32>* %b to <8 x i32>*
; CHECK-GE15: store <8 x i32> %[[RES2]], ptr %b, align 8
; CHECK-LT15: store <8 x i32> %[[RES2]], <8 x i32>* %2, align 8
; CHECK: ret void