; Copyright (C) Codeplay Software Limited. All Rights Reserved.
; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -k bar -vecz-simd-width=4 -S -o - %s | %filecheck %t

target triple = "spir64-unknown-unknown"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

declare spir_func i64 @_Z13get_global_idj(i32)

define void @bar(i64** %ptrptrs, i64 %val) {
  %idx = call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidxa = getelementptr inbounds i64*, i64** %ptrptrs, i64 %idx
  %ptrs = load i64*, i64** %arrayidxa, align 4
  %addr = getelementptr inbounds i64, i64* %ptrs, <4 x i32> <i32 2, i32 2, i32 2, i32 2>

  %elt0 = extractelement <4 x i64*> %addr, i32 0
  %elt1 = extractelement <4 x i64*> %addr, i32 1
  %elt2 = extractelement <4 x i64*> %addr, i32 2
  %elt3 = extractelement <4 x i64*> %addr, i32 3

  store i64 %val, i64* %elt0
  store i64 %val, i64* %elt1
  store i64 %val, i64* %elt2
  store i64 %val, i64* %elt3
  ret void
}

; it checks that the GEP with mixed scalar/vector operands in the kernel
; gets scalarized/re-packetized correctly

; CHECK: define void @__vecz_v4_bar
; CHECK-GE15: %[[ADDR:.+]] = getelementptr inbounds i64, <4 x ptr> %{{.+}}, i64 2
; CHECK-LT15: %[[ADDR:.+]] = getelementptr inbounds i64, <4 x i64*> %{{.+}}, i64 2
; CHECK-GE15: call void @__vecz_b_scatter_store8_Dv4_mDv4_u3ptr(<4 x i64> %.splat{{.*}}, <4 x ptr> %[[ADDR]])
; CHECK-LT15: call void @__vecz_b_scatter_store8_Dv4_mDv4_Pm(<4 x i64> %.splat{{.*}}, <4 x i64*> %[[ADDR]])