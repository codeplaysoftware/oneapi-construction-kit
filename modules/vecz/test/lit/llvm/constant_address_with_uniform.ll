; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -k test -w 4 -S < %s | %filecheck %t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir-unknown-unknown"

declare spir_func i32 @get_global_id(i32);

define spir_kernel void @test(i32 addrspace(1)* %out, i32 addrspace(1)* addrspace(1)* %out2) {
entry:
  %gid = call i32 @get_global_id(i32 0)
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 3
  store i32 %gid, i32 addrspace(1)* %arrayidx, align 4

  %arrayidx2 = getelementptr inbounds i32 addrspace(1)*, i32 addrspace(1)* addrspace(1)* %out2, i32 %gid
  store i32 addrspace(1)* %arrayidx, i32 addrspace(1)* addrspace(1)* %arrayidx2, align 4

  ret void
}

; CHECK: define spir_kernel void @__vecz_v4_test
; CHECK-NEXT: entry:
; CHECK-NEXT: %gid = call i32 @get_global_id(i32 0)
; CHECK-NEXT-GE15: %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %out, i32 3
; CHECK-NEXT-LT15: %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 3
; CHECK-GE15: store i32 %gid, ptr addrspace(1) %arrayidx, align 4
; CHECK-LT15: store i32 %gid, i32 addrspace(1)* %arrayidx, align 4
; CHECK-GE15: store <4 x ptr addrspace(1)> %{{.+}}, ptr addrspace(1) %{{.+}}
; CHECK-LT15: store <4 x i32 addrspace(1)*> %{{.+}}, <4 x i32 addrspace(1)*> addrspace(1)* %{{.+}}