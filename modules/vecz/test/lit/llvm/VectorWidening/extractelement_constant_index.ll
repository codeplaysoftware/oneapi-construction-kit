; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %veczc -k extract_constant_index -vecz-simd-width=4 -vecz-passes=packetizer -vecz-choices=TargetIndependentPacketization -S < %s | %filecheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown"

; Function Attrs: nounwind
define spir_kernel void @extract_constant_index(<4 x i64> addrspace(1)* %in, i32 %x, i64 addrspace(1)* %out) #0 {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 0) #2
  %arrayidx = getelementptr inbounds <4 x i64>, <4 x i64> addrspace(1)* %in, i64 %call
  %0 = load <4 x i64>, <4 x i64> addrspace(1)* %arrayidx, align 4
  %vecext = extractelement <4 x i64> %0, i32 0;
  %arrayidx1 = getelementptr inbounds i64, i64 addrspace(1)* %out, i64 %call
  store i64 %vecext, i64 addrspace(1)* %arrayidx1, align 1
  ret void
}

declare spir_func i64 @_Z13get_global_idj(i32) #1

; CHECK: define spir_kernel void @__vecz_v4_extract_constant_index
; CHECK: %[[LD:.+]] = load <16 x i64>
; CHECK: %[[EXT:.+]] = shufflevector <16 x i64> %[[LD]], <16 x i64> undef, <4 x i32> <i32 0, i32 4, i32 8, i32 12>
; CHECK: store <4 x i64> %[[EXT]]
; CHECK: ret void