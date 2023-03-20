; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -k test -vecz-simd-width=4 -vecz-passes=cfg-convert,packetizer -S < %s | %filecheck %t

; ModuleID = 'kernel.opencl'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown"

; Function Attrs: nounwind
define spir_kernel void @test(i32* %in, i32* %out) {
entry:
  %in.addr = alloca i32*, align 8
  %out.addr = alloca i32*, align 8
  %gid = alloca i64, align 8
  store i32* %in, i32** %in.addr, align 8
  store i32* %out, i32** %out.addr, align 8
  %call = call spir_func i64 @_Z13get_global_idj(i32 0)
  store i64 %call, i64* %gid, align 8
  %0 = load i64, i64* %gid, align 8
  %rem = urem i64 %0, 16
  %cmp = icmp eq i64 %rem, 1
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %1 = load i64, i64* %gid, align 8
  %2 = load i32*, i32** %in.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32* %2, i64 %1
  %3 = load i32, i32* %arrayidx, align 4
  %4 = load i64, i64* %gid, align 8
  %5 = load i32*, i32** %in.addr, align 8
  %arrayidx1 = getelementptr inbounds i32, i32* %5, i64 %4
  %call2 = call spir_func i32 bitcast (i32 (i32, i32 addrspace(1)*)* @foo to i32 (i32, i32*)*)(i32 %3, i32* %arrayidx1)
  %6 = load i64, i64* %gid, align 8
  %7 = load i32*, i32** %out.addr, align 8
  %arrayidx3 = getelementptr inbounds i32, i32* %7, i64 %6
  store i32 %call2, i32* %arrayidx3, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %8 = load i64, i64* %gid, align 8
  %9 = load i32*, i32** %in.addr, align 8
  %arrayidx4 = getelementptr inbounds i32, i32* %9, i64 %8
  %10 = load i32, i32* %arrayidx4, align 4
  %11 = load i64, i64* %gid, align 8
  %12 = load i32*, i32** %out.addr, align 8
  %arrayidx5 = getelementptr inbounds i32, i32* %12, i64 %11
  store i32 %10, i32* %arrayidx5, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

declare spir_func i64 @_Z13get_global_idj(i32)
declare spir_func i32 @foo(i32, i32 addrspace(1)*)

; CHECK: define spir_kernel void @__vecz_v4_test(
; CHECK: call spir_func i32 @__vecz_b_masked_foo(
; CHECK: call spir_func i32 @__vecz_b_masked_foo(
; CHECK: call spir_func i32 @__vecz_b_masked_foo(
; CHECK: call spir_func i32 @__vecz_b_masked_foo(
; CHECK: ret void

; CHECK-GE15: define private spir_func i32 @__vecz_b_masked_foo(i32{{( %0)?}}, ptr{{( %1)?}}, i1{{( %2)?}}
; CHECK-LT15: define private spir_func i32 @__vecz_b_masked_foo(i32{{( %0)?}}, i32*{{( %1)?}}, i1{{( %2)?}}
; CHECK-GE15: call spir_func i32 @foo(i32 %0, ptr %1)
; CHECK-LT15: call spir_func i32 bitcast (i32 (i32, i32 addrspace(1)*)* @foo to i32 (i32, i32*)*)(i32 %0, i32* %1)