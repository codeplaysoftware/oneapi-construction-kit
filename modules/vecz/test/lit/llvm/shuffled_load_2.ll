; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %veczc -S < %s | %filecheck %s

target triple = "spir64-unknown-unknown"
target datalayout = "e-p:64:64:64-m:e-i64:64-f80:128-n8:16:32:64-S128"

define spir_kernel void @load16(i8 addrspace(1)* %out, i8 addrspace(1)* %in, i32 %stride) {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %call to i32
  %call1 = tail call spir_func i64 @_Z13get_global_idj(i32 1)
  %conv2 = trunc i64 %call1 to i32
  %mul = mul nsw i32 %conv2, %stride
  %add = add nsw i32 %mul, %conv
  %mul3 = shl nsw i32 %add, 1
  %conv4 = sext i32 %mul3 to i64
  %arrayidx = getelementptr inbounds i8, i8 addrspace(1)* %in, i64 %conv4
  %0 = load i8, i8 addrspace(1)* %arrayidx, align 1
  %add5 = or i64 %conv4, 1
  %arrayidx6 = getelementptr inbounds i8, i8 addrspace(1)* %in, i64 %add5
  %1 = load i8, i8 addrspace(1)* %arrayidx6, align 1
  %add9 = add i8 %1, %0
  %idxprom = sext i32 %add to i64
  %arrayidx13 = getelementptr inbounds i8, i8 addrspace(1)* %out, i64 %idxprom
  store i8 %add9, i8 addrspace(1)* %arrayidx13, align 1
  ret void
}

declare spir_func i64 @_Z13get_global_idj(i32)

; CHECK: spir_kernel void @load16
; CHECK: load <4 x i8>
; CHECK: load <4 x i8>
; CHECK-NOT: load <4 x i8>
; CHECK-NOT: call <4 x i8> @__vecz_b_interleaved_load
; CHECK-NOT: call <4 x i8> @__vecz_b_gather_load
; CHECK: shufflevector <4 x i8>
; CHECK: shufflevector <4 x i8>
; CHECK-NOT: shufflevector <4 x i8>
; CHECK: ret void