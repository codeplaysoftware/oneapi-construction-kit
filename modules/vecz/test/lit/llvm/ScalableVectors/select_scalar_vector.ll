; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; REQUIRES: llvm-13+
; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -k select_scalar_vector -vecz-scalable -vecz-simd-width=4 -S < %s | %filecheck %t

target triple = "spir64-unknown-unknown"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

declare spir_func i64 @_Z13get_global_idj(i32)

define spir_kernel void @select_scalar_vector(i32* %aptr, i32* %bptr, <2 x i32>* %cptr, <2 x i32>* %zptr) {
entry:
  %idx = call spir_func i64 @_Z13get_global_idj(i32 0)
  %arrayidxa = getelementptr inbounds i32, i32* %aptr, i64 %idx
  %arrayidxb = getelementptr inbounds i32, i32* %bptr, i64 %idx
  %arrayidxc = getelementptr inbounds <2 x i32>, <2 x i32>* %cptr, i64 %idx
  %arrayidxz = getelementptr inbounds <2 x i32>, <2 x i32>* %zptr, i64 %idx
  %a = load i32, i32* %arrayidxa, align 4
  %b = load i32, i32* %arrayidxb, align 4
  %c = load <2 x i32>, <2 x i32>* %arrayidxc, align 4
  %cmp = icmp slt i32 %a, %b
  %sel = select i1 %cmp, <2 x i32> %c, <2 x i32> <i32 4, i32 4>
  store <2 x i32> %sel, <2 x i32>* %arrayidxz, align 4
  ret void
}

; CHECK: define spir_kernel void @__vecz_nxv4_select_scalar_vector
; CHECK-GE15: [[rhs:%.*]] = load <vscale x 8 x i32>, ptr
; CHECK-LT15: [[rhs:%.*]] = load <vscale x 8 x i32>, <vscale x 8 x i32>*
; CHECK: [[cmp1:%.*]] = icmp slt <vscale x 4 x i32>
; CHECK: [[sext:%.*]] = sext <vscale x 4 x i1> [[cmp1]] to <vscale x 4 x i8>
; CHECK-GE15: store <vscale x 4 x i8> [[sext]], ptr [[alloc:%.*]], align 4
; CHECK-LT15: store <vscale x 4 x i8> [[sext]], <vscale x 4 x i8>* [[alloc:%.*]], align 4
; CHECK-LT15: [[bcast:%.*]] = getelementptr inbounds <vscale x 4 x i8>, <vscale x 4 x i8>* [[alloc]], i64 0, i64 0
; CHECK: [[idx0:%.*]] = call <vscale x 8 x i32> @llvm.experimental.stepvector.nxv8i32()
; CHECK: [[idx1:%.*]] = lshr <vscale x 8 x i32> [[idx0]], shufflevector (<vscale x 8 x i32> insertelement (<vscale x 8 x i32> {{(undef|poison)}}, i32 1, {{(i32|i64)}} 0), <vscale x 8 x i32> {{(undef|poison)}}, <vscale x 8 x i32> zeroinitializer)
; CHECK: [[sext2:%.*]] = sext <vscale x 8 x i32> [[idx1]] to <vscale x 8 x i64>
; CHECK-GE15: [[addrs:%.*]] = getelementptr inbounds i8, ptr [[alloc]], <vscale x 8 x i64> [[sext2]]
; CHECK-LT15: [[addrs:%.*]] = getelementptr inbounds i8, i8* [[bcast]], <vscale x 8 x i64> [[sext2]]
; CHECK-GE15: [[gather:%.*]] = call <vscale x 8 x i8> @llvm.masked.gather.nxv8i8.nxv8p0(<vscale x 8 x ptr> [[addrs]],
; CHECK-LT15: [[gather:%.*]] = call <vscale x 8 x i8> @llvm.masked.gather.nxv8i8.nxv8p0i8(<vscale x 8 x i8*> [[addrs]],
; CHECK: [[cmp:%.*]] = trunc <vscale x 8 x i8> [[gather]] to <vscale x 8 x i1>
; CHECK: [[sel:%.*]] = select <vscale x 8 x i1> [[cmp]], <vscale x 8 x i32> [[rhs]], <vscale x 8 x i32> shufflevector (<vscale x 8 x i32> insertelement (<vscale x 8 x i32> {{(undef|poison)}}, i32 4, {{(i32|i64)}} 0), <vscale x 8 x i32> {{(undef|poison)}}, <vscale x 8 x i32> zeroinitializer)
; CHECK: store <vscale x 8 x i32> [[sel]],