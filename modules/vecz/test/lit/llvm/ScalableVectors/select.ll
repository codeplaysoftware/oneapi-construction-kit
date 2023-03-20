; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -vecz-scalable -vecz-simd-width=4 -S < %s | %filecheck %t

target triple = "spir64-unknown-unknown"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

define spir_kernel void @select_scalar_scalar(i32* %aptr, i32* %bptr, i32* %zptr) {
entry:
  %idx = call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidxa = getelementptr inbounds i32, i32* %aptr, i64 %idx
  %arrayidxb = getelementptr inbounds i32, i32* %bptr, i64 %idx
  %arrayidxz = getelementptr inbounds i32, i32* %zptr, i64 %idx
  %a = load i32, i32* %arrayidxa, align 4
  %b = load i32, i32* %arrayidxb, align 4
  %cmp = icmp slt i32 %a, %b
  %sel = select i1 %cmp, i32 %b, i32 4
  store i32 %sel, i32* %arrayidxz, align 4
  ret void
}

define spir_kernel void @select_vector_vector(<2 x i32>* %aptr, <2 x i32>* %bptr, <2 x i32>* %cptr, <2 x i32>* %zptr) {
entry:
  %idx = call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidxa = getelementptr inbounds <2 x i32>, <2 x i32>* %aptr, i64 %idx
  %arrayidxb = getelementptr inbounds <2 x i32>, <2 x i32>* %bptr, i64 %idx
  %arrayidxc = getelementptr inbounds <2 x i32>, <2 x i32>* %cptr, i64 %idx
  %arrayidxz = getelementptr inbounds <2 x i32>, <2 x i32>* %zptr, i64 %idx
  %a = load <2 x i32>, <2 x i32>* %arrayidxa, align 4
  %b = load <2 x i32>, <2 x i32>* %arrayidxb, align 4
  %c = load <2 x i32>, <2 x i32>* %arrayidxc, align 4
  %cmp = icmp slt <2 x i32> %a, %b
  %sel = select <2 x i1> %cmp, <2 x i32> %c, <2 x i32> <i32 4, i32 4>
  store <2 x i32> %sel, <2 x i32>* %arrayidxz, align 4
  ret void
}

declare spir_func i64 @_Z13get_global_idj(i32)

; CHECK: define spir_kernel void @__vecz_nxv4_select_scalar_scalar
; CHECK-GE15: [[lhs:%[0-9a-z]+]] = load <vscale x 4 x i32>, ptr
; CHECK-LT15: [[lhs:%[0-9a-z]+]] = load <vscale x 4 x i32>, <vscale x 4 x i32>*
; CHECK-GE15: [[rhs:%[0-9a-z]+]] = load <vscale x 4 x i32>, ptr
; CHECK-LT15: [[rhs:%[0-9a-z]+]] = load <vscale x 4 x i32>, <vscale x 4 x i32>*
; CHECK: [[cmp:%[0-9a-z]+]] = icmp slt <vscale x 4 x i32> [[lhs]], [[rhs]]
; CHECK: [[sel:%[0-9a-z]+]] = select <vscale x 4 x i1> [[cmp]], <vscale x 4 x i32> [[rhs]], <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> {{(undef|poison)}}, i32 4, {{(i32|i64)}} 0), <vscale x 4 x i32> {{(undef|poison)}}, <vscale x 4 x i32> zeroinitializer)
; CHECK: store <vscale x 4 x i32> [[sel]],

; CHECK: define spir_kernel void @__vecz_nxv4_select_vector_vector
; CHECK-GE15: [[x:%[0-9a-z]+]] = load <vscale x 8 x i32>, ptr
; CHECK-LT15: [[x:%[0-9a-z]+]] = load <vscale x 8 x i32>, <vscale x 8 x i32>*
; CHECK-GE15: [[y:%[0-9a-z]+]] = load <vscale x 8 x i32>, ptr
; CHECK-LT15: [[y:%[0-9a-z]+]] = load <vscale x 8 x i32>, <vscale x 8 x i32>*
; CHECK-GE15: [[z:%[0-9a-z]+]] = load <vscale x 8 x i32>, ptr
; CHECK-LT15: [[z:%[0-9a-z]+]] = load <vscale x 8 x i32>, <vscale x 8 x i32>*
; CHECK: [[cmp:%[0-9a-z]+]] = icmp slt <vscale x 8 x i32> [[x]], [[y]]
; CHECK: [[sel:%[0-9a-z]+]] = select <vscale x 8 x i1> [[cmp]], <vscale x 8 x i32> [[z]], <vscale x 8 x i32> shufflevector (<vscale x 8 x i32> insertelement (<vscale x 8 x i32> {{(undef|poison)}}, i32 4, {{(i32|i64)}} 0), <vscale x 8 x i32> {{(undef|poison)}}, <vscale x 8 x i32> zeroinitializer)
; CHECK: store <vscale x 8 x i32> [[sel]],