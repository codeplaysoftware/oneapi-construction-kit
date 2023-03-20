; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -k test_fn -vecz-simd-width=4 -S < %s | %filecheck %t

; ModuleID = 'kernel.opencl'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown"

; Function Attrs: nounwind
define spir_kernel void @test_fn(i32 addrspace(1)* %results) #0 {
entry:
  %results.addr = alloca i32 addrspace(1)*, align 8
  %tid = alloca i32, align 4
  store i32 addrspace(1)* %results, i32 addrspace(1)** %results.addr, align 8
  %call = call spir_func i64 @_Z13get_global_idj(i32 0) #2
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* %tid, align 4
  %0 = load i32, i32* %tid, align 4
  %cmp = icmp sgt i32 3, %0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = load i32, i32* %tid, align 4
  %mul = mul nsw i32 2, %1
  %add = add nsw i32 %mul, 2
  %idxprom = sext i32 %add to i64
  %2 = load i32 addrspace(1)*, i32 addrspace(1)** %results.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %2, i64 %idxprom
  store i32 5, i32 addrspace(1)* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

declare spir_func i64 @_Z13get_global_idj(i32) #1

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="0" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="0" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nobuiltin }

!opencl.kernels = !{!0}
!llvm.ident = !{!6}

!0 = !{void (i32 addrspace(1)*)* @test_fn, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"int*"}
!4 = !{!"kernel_arg_base_type", !"int*"}
!5 = !{!"kernel_arg_type_qual", !""}
!6 = !{!"clang version 3.8.0 "}


; CHECK-GE15: define void @__vecz_b_masked_interleaved_store4_2_Dv4_ju3ptrU3AS1Dv4_b(<4 x i32>{{( %0)?}}, ptr addrspace(1){{( %1)?}}, <4 x i1>{{( %2)?}}) {
; CHECK-LT15: define void @__vecz_b_masked_interleaved_store4_2_Dv4_jPU3AS1jDv4_b(<4 x i32>{{( %0)?}}, i32 addrspace(1)*{{( %1)?}}, <4 x i1>{{( %2)?}}) {

; Check for the address splat
; CHECK-GE15: %[[BROADCASTADDRSPLATINSERT:.+]] = insertelement <4 x ptr addrspace(1)> {{poison|undef}}, ptr addrspace(1) %{{.+}}, i32 0
; CHECK-LT15: %[[BROADCASTADDRSPLATINSERT:.+]] = insertelement <4 x i32 addrspace(1)*> {{poison|undef}}, i32 addrspace(1)* %{{.+}}, i32 0
; CHECK-GE15: %[[BROADCASTADDRSPLAT:.+]] = shufflevector <4 x ptr addrspace(1)> %[[BROADCASTADDRSPLATINSERT]], <4 x ptr addrspace(1)> {{poison|undef}}, <4 x i32> zeroinitializer
; CHECK-LT15: %[[BROADCASTADDRSPLAT:.+]] = shufflevector <4 x i32 addrspace(1)*> %[[BROADCASTADDRSPLATINSERT]], <4 x i32 addrspace(1)*> {{poison|undef}}, <4 x i32> zeroinitializer
; CHECK-GE15: getelementptr i32, <4 x ptr addrspace(1)> %[[BROADCASTADDRSPLAT]], <4 x i64> <i64 0, i64 2, i64 4, i64 6>
; CHECK-LT15: getelementptr i32, <4 x i32 addrspace(1)*> %[[BROADCASTADDRSPLAT]], <4 x i64> <i64 0, i64 2, i64 4, i64 6>

; CHECK: ret void