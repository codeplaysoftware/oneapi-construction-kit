; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -k test -w 4 -S < %s | %filecheck %t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir-unknown-unknown"

declare spir_func i32 @get_global_id(i32);

define spir_kernel void @test(i32 addrspace(1)* %in) {
entry:
  %gid = call i32 @get_global_id(i32 0)
  %and = and i32 %gid, 1
  %cmp = icmp eq i32 %and, 0
  br i1 %cmp, label %if, label %early_ret

early_ret:
; just to prevent ROSCC from sticking its oar in
  %gid1 = call i32 @get_global_id(i32 1)
  ret void

if:
  %single_load = load i32, i32 addrspace(1)* %in
  %single_add = add i32 %single_load, 42
  store i32 %single_add, i32 addrspace(1)* %in

  ret void
}

; CHECK: define spir_kernel void @__vecz_v4_test
; CHECK: %[[BITCAST:.*]] = bitcast <4 x i1> %cmp{{[0-9]*}} to i4
; CHECK: %[[MASK:.*]] = icmp ne i4 %[[BITCAST]], 0
; CHECK-GE15: %[[single_load:single_load[0-9]*]] = call i32 @__vecz_b_masked_load4_ju3ptrU3AS1b(ptr addrspace(1) %in, i1 %[[MASK]])
; CHECK-LT15: %[[single_load:single_load[0-9]*]] = call i32 @__vecz_b_masked_load4_jPU3AS1jb(i32 addrspace(1)* %in, i1 %[[MASK]])
; CHECK: %[[single_add:single_add[0-9]*]] = add i32 %[[single_load]], 42
; CHECK-GE15: call void @__vecz_b_masked_store4_ju3ptrU3AS1b(i32 %[[single_add]], ptr addrspace(1) %in, i1 %[[MASK]])
; CHECK-LT15: call void @__vecz_b_masked_store4_jPU3AS1jb(i32 %[[single_add]], i32 addrspace(1)* %in, i1 %[[MASK]])