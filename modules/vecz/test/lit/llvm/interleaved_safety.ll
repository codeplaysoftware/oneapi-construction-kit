; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -k f -vecz-simd-width 4 -vecz-choices=FullScalarization -S < %s | %filecheck %t

; ModuleID = 'kernel.opencl'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown"

; Function Attrs: nounwind
define spir_kernel void @f(<4 x double> addrspace(1)* %a, <4 x double> addrspace(1)* %b, <4 x double> addrspace(1)* %c, <4 x double> addrspace(1)* %d, <4 x double> addrspace(1)* %e, i8 addrspace(1)* %flag) #0 {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 0) #3
  %add.ptr = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %b, i64 %call
  %.cast = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %add.ptr, i64 0, i64 0
  %0 = load <4 x double>, <4 x double> addrspace(1)* %add.ptr, align 32
  call spir_func void @_Z7barrierj(i32 2) #3
  store double 1.600000e+01, double addrspace(1)* %.cast, align 8
  %1 = load <4 x double>, <4 x double> addrspace(1)* %add.ptr, align 32
  %vecins5 = shufflevector <4 x double> %0, <4 x double> %1, <4 x i32> <i32 0, i32 1, i32 6, i32 undef>
  %vecins7 = shufflevector <4 x double> %vecins5, <4 x double> %1, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %arrayidx = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %c, i64 %call
  %2 = load <4 x double>, <4 x double> addrspace(1)* %arrayidx, align 32
  %arrayidx8 = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %d, i64 %call
  %3 = load <4 x double>, <4 x double> addrspace(1)* %arrayidx8, align 32
  %arrayidx9 = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %e, i64 %call
  %4 = load <4 x double>, <4 x double> addrspace(1)* %arrayidx9, align 32
  %div = fdiv <4 x double> %3, %4
  %5 = call <4 x double> @llvm.fmuladd.v4f64(<4 x double> %vecins7, <4 x double> %2, <4 x double> %div)
  %arrayidx10 = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %a, i64 %call
  %6 = load <4 x double>, <4 x double> addrspace(1)* %arrayidx10, align 32
  %sub = fsub <4 x double> %6, %5
  store <4 x double> %sub, <4 x double> addrspace(1)* %arrayidx10, align 32
  ret void
}

declare spir_func i64 @_Z13get_global_idj(i32) #1

declare spir_func void @_Z7barrierj(i32) #1

; Function Attrs: nounwind readnone
declare <4 x double> @llvm.fmuladd.v4f64(<4 x double>, <4 x double>, <4 x double>) #2

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="0" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="0" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }
attributes #3 = { nobuiltin nounwind }

!opencl.kernels = !{!0}
!llvm.ident = !{!6}

!0 = !{void (<4 x double> addrspace(1)*, <4 x double> addrspace(1)*, <4 x double> addrspace(1)*, <4 x double> addrspace(1)*, <4 x double> addrspace(1)*, i8 addrspace(1)*)* @f, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 1, i32 1, i32 1, i32 1}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"double4*", !"double4*", !"double4*", !"double4*", !"double4*", !"char*"}
!4 = !{!"kernel_arg_base_type", !"double __attribute__((ext_vector_type(4)))*", !"double __attribute__((ext_vector_type(4)))*", !"double __attribute__((ext_vector_type(4)))*", !"double __attribute__((ext_vector_type(4)))*", !"double __attribute__((ext_vector_type(4)))*", !"char*"}
!5 = !{!"kernel_arg_type_qual", !"", !"", !"", !"", !"", !""}
!6 = !{!"clang version 3.8.1 "}

; Function start
; CHECK: define spir_kernel void @__vecz_v4_f
; CHECK: call spir_func i64 @_Z13get_global_idj(i32 0)

; There should be exactly 4 interleaved loads and one store in the code
; CHECK-GE15: call <4 x double> @__vecz_b_interleaved_load8_4_Dv4_du3ptrU3AS1
; CHECK-LT15: call <4 x double> @__vecz_b_interleaved_load8_4_Dv4_dPU3AS1d
; CHECK-GE15: call <4 x double> @__vecz_b_interleaved_load8_4_Dv4_du3ptrU3AS1
; CHECK-LT15: call <4 x double> @__vecz_b_interleaved_load8_4_Dv4_dPU3AS1d

; And in between them there should be a barrier call
; CHECK: call spir_func void @_Z7barrierj
; CHECK-GE15: call void @__vecz_b_interleaved_store8_4_Dv4_du3ptrU3AS1(<4 x double> <double 1.600000e+01, double 1.600000e+01, double 1.600000e+01, double 1.600000e+01>
; CHECK-LT15: call void @__vecz_b_interleaved_store8_4_Dv4_dPU3AS1d(<4 x double> <double 1.600000e+01, double 1.600000e+01, double 1.600000e+01, double 1.600000e+01>
; CHECK-GE15: call <4 x double> @__vecz_b_interleaved_load8_4_Dv4_du3ptrU3AS1
; CHECK-LT15: call <4 x double> @__vecz_b_interleaved_load8_4_Dv4_dPU3AS1d
; CHECK-GE15: call <4 x double> @__vecz_b_interleaved_load8_4_Dv4_du3ptrU3AS1
; CHECK-LT15: call <4 x double> @__vecz_b_interleaved_load8_4_Dv4_dPU3AS1d

; There shouldn't be any more interleaved loads or stores left
; CHECK-NOT-GE15: call <4 x double> @__vecz_b_interleaved_load4_Dv4_du3ptrU3AS1
; CHECK-NOT-LT15: call <4 x double> @__vecz_b_interleaved_load4_Dv4_dPU3AS1d
; CHECK-NOT-GE15: call void @__vecz_b_interleaved_store8_4_Dv4_du3ptrU3AS1(<4 x double> <double 1.600000e+01, double 1.600000e+01, double 1.600000e+01, double 1.600000e+01>
; CHECK-NOT-LT15: call void @__vecz_b_interleaved_store8_4_Dv4_dPU3AS1d(<4 x double> <double 1.600000e+01, double 1.600000e+01, double 1.600000e+01, double 1.600000e+01>

; There should be some sufflevector instructions after the simplification
; CHECK: shufflevector

; Function end
; CHECK: ret void