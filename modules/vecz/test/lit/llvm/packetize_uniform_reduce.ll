; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -k reduce -vecz-choices=PacketizeUniform -vecz-simd-width=4 -S < %s | %filecheck %t

; ModuleID = 'kernel.opencl'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown"

declare spir_func i64 @_Z12get_local_idj(i32)
declare spir_func i64 @_Z13get_global_idj(i32)
declare spir_func i64 @_Z14get_local_sizej(i32)

; Function Attrs: nounwind
define spir_kernel void @reduce(i32 addrspace(3)* %in, i32 addrspace(3)* %out) {
entry:
  %call = call spir_func i64 @_Z12get_local_idj(i32 0)
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %storemerge = phi i32 [ 1, %entry ], [ %mul6, %for.inc ]
  %conv = zext i32 %storemerge to i64
  %call1 = call spir_func i64 @_Z14get_local_sizej(i32 0)
  %cmp = icmp ult i64 %conv, %call1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %mul = mul i32 %storemerge, 3
  %conv3 = zext i32 %mul to i64
  %0 = icmp eq i32 %mul, 0
  %1 = select i1 %0, i64 1, i64 %conv3
  %rem = urem i64 %call, %1
  %cmp4 = icmp eq i64 %rem, 0
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32 addrspace(3)* %out, i64 %call
  store i32 5, i32 addrspace(3)* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %mul6 = shl i32 %storemerge, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

; This test checks if the "packetize uniform" Vecz choice works on uniform
; values used by varying values, but not on uniform values used by other uniform
; values only.

; CHECK-GE15: define spir_kernel void @__vecz_v4_reduce(ptr addrspace(3) %in, ptr addrspace(3) %out)
; CHECK-LT15: define spir_kernel void @__vecz_v4_reduce(i32 addrspace(3)* %in, i32 addrspace(3)* %out)
; CHECK: insertelement <4 x i64> {{poison|undef}}, i64 %{{.+}}, {{(i32|i64)}} 0
; CHECK: shufflevector <4 x i64> %{{.+}}, <4 x i64> {{poison|undef}}, <4 x i32> zeroinitializer
; CHECK: phi <4 x i32>
; CHECK: mul <4 x i32> %{{.+}}, <i32 3, i32 3, i32 3, i32 3>
; CHECK: urem <4 x i64>
; CHECK: icmp eq <4 x i64> %{{.+}}, zeroinitializer

; The branch condition is actually Uniform, despite the divergence analysis
; CHECK: icmp ugt i64
; CHECK: ret void