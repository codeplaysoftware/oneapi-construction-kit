
; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %veczc -vecz-passes=builtin-inlining,verify -vecz-simd-width=4 %flag -S < %s | %filecheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown"

define spir_kernel void @test_memset_i8(i64* %z) {
  %dst = bitcast i64* %z to i8*
  call void @llvm.memset.p0i8.i64(i8* %dst, i8 42, i64 18, i32 8, i1 false)
  ret void
}

; CHECK-LABEL: define spir_kernel void @__vecz_v4_test_memset_i8(ptr %z)
; CHECK:  %dst = bitcast ptr %z to ptr
; CHECK:  %1 = getelementptr inbounds i8, ptr %dst, i64 0
; CHECK:  store i64 3038287259199220266, ptr %1, align 8
; CHECK:  %2 = getelementptr inbounds i8, ptr %dst, i64 8
; CHECK:  store i64 3038287259199220266, ptr %2, align 8
; CHECK:  %dst1 = getelementptr inbounds i8, ptr %dst, i64 16
; CHECK:  store i8 42, ptr %dst1, align 1
; CHECK:  %dst2 = getelementptr inbounds i8, ptr %dst, i64 17
; CHECK:  store i8 42, ptr %dst2, align 1
; CHECK:  ret void
; CHECK: }

define spir_kernel void @test_memset_i16(i64* %z) {
  %dst = bitcast i64* %z to i16*
  call void @llvm.memset.p0i16.i64(i16* %dst, i8 42, i64 18, i32 8, i1 false)
  ret void
}

; CHECK-LABEL: define spir_kernel void @__vecz_v4_test_memset_i16(ptr %z)
; CHECK:  %dst = bitcast ptr %z to ptr
; CHECK:  %1 = getelementptr inbounds i8, ptr %dst, i64 0
; CHECK:  store i64 3038287259199220266, ptr %1, align 8
; CHECK:  %2 = getelementptr inbounds i8, ptr %dst, i64 8
; CHECK:  store i64 3038287259199220266, ptr %2, align 8
; CHECK:  %dst1 = getelementptr inbounds i8, ptr %dst, i64 16
; CHECK:  store i8 42, ptr %dst1, align 1
; CHECK:  %dst2 = getelementptr inbounds i8, ptr %dst, i64 17
; CHECK:  store i8 42, ptr %dst2, align 1
; CHECK:  ret void
; CHECK: }

define spir_kernel void @test_memcpy_i8(i64* %a, i64* %z) {
  %src = bitcast i64* %a to i8*
  %dst = bitcast i64* %z to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dst, i8* %src, i64 18, i32 8, i1 false)
  ret void
}

; CHECK-LABEL: define spir_kernel void @__vecz_v4_test_memcpy_i8(ptr %a, ptr %z)
; CHECK:  %src = bitcast ptr %a to ptr
; CHECK:  %dst = bitcast ptr %z to ptr
; CHECK:  %1 = getelementptr inbounds i8, ptr %src, i64 0
; CHECK:  %2 = getelementptr inbounds i8, ptr %dst, i64 0
; CHECK:  %src1 = load i64, ptr %1, align 8
; CHECK:  store i64 %src1, ptr %2, align 8
; CHECK:  %3 = getelementptr inbounds i8, ptr %src, i64 8
; CHECK:  %4 = getelementptr inbounds i8, ptr %dst, i64 8
; CHECK:  %src2 = load i64, ptr %3, align 8
; CHECK:  store i64 %src2, ptr %4, align 8
; CHECK:  %5 = getelementptr inbounds i8, ptr %src, i64 16
; CHECK:  %dst3 = getelementptr inbounds i8, ptr %dst, i64 16
; CHECK:  %src4 = load i8, ptr %5, align 1
; CHECK:  store i8 %src4, ptr %dst3, align 1
; CHECK:  %6 = getelementptr inbounds i8, ptr %src, i64 17
; CHECK:  %dst5 = getelementptr inbounds i8, ptr %dst, i64 17
; CHECK:  %src6 = load i8, ptr %6, align 1
; CHECK:  store i8 %src6, ptr %dst5, align 1
; CHECK:  ret void
; CHECK: }

define spir_kernel void @test_memcpy_i16(i64* %a, i64* %z) {
  %src = bitcast i64* %a to i16*
  %dst = bitcast i64* %z to i16*
  call void @llvm.memcpy.p0i16.p0i16.i64(i16* %dst, i16* %src, i64 18, i32 8, i1 false)
  ret void
}

; CHECK-LABEL: define spir_kernel void @__vecz_v4_test_memcpy_i16(ptr %a, ptr %z)
; CHECK:  %src = bitcast ptr %a to ptr
; CHECK:  %dst = bitcast ptr %z to ptr
; CHECK:  %1 = getelementptr inbounds i8, ptr %src, i64 0
; CHECK:  %2 = getelementptr inbounds i8, ptr %dst, i64 0
; CHECK:  %src1 = load i64, ptr %1, align 8
; CHECK:  store i64 %src1, ptr %2, align 8
; CHECK:  %3 = getelementptr inbounds i8, ptr %src, i64 8
; CHECK:  %4 = getelementptr inbounds i8, ptr %dst, i64 8
; CHECK:  %src2 = load i64, ptr %3, align 8
; CHECK:  store i64 %src2, ptr %4, align 8
; CHECK:  %5 = getelementptr inbounds i8, ptr %src, i64 16
; CHECK:  %dst3 = getelementptr inbounds i8, ptr %dst, i64 16
; CHECK:  %src4 = load i8, ptr %5, align 1
; CHECK:  store i8 %src4, ptr %dst3, align 1
; CHECK:  %6 = getelementptr inbounds i8, ptr %src, i64 17
; CHECK:  %dst5 = getelementptr inbounds i8, ptr %dst, i64 17
; CHECK:  %src6 = load i8, ptr %6, align 1
; CHECK:  store i8 %src6, ptr %dst5, align 1
; CHECK:  ret void
; CHECK: }

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i32, i1)
declare void @llvm.memset.p0i16.i64(i16*, i8, i64, i32, i1)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i32, i1)
declare void @llvm.memcpy.p0i16.p0i16.i64(i16*, i16*, i64, i32, i1)