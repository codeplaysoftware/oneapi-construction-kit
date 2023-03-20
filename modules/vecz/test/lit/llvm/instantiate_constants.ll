; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -k test -vecz-simd-width 4 -S < %s | %filecheck %t

; ModuleID = 'Unknown buffer'
source_filename = "kernel.opencl"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent nounwind
define spir_kernel void @test(half addrspace(1)* nocapture readonly %p, float addrspace(1)* nocapture %f) local_unnamed_addr #0 {
entry:
  %data = alloca [1 x i16], align 2
  %0 = bitcast [1 x i16]* %data to i8*
  %arraydecay = getelementptr inbounds [1 x i16], [1 x i16]* %data, i64 0, i64 0
  %1 = bitcast [1 x i16]* %data to half*
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0) #5
  %arrayidx7 = getelementptr inbounds half, half addrspace(1)* %p, i64 %call
  %arrayidx = bitcast half addrspace(1)* %arrayidx7 to i16 addrspace(1)*
  %2 = load i16, i16 addrspace(1)* %arrayidx, align 2, !tbaa !9
  store i16 %2, i16* %arraydecay, align 2, !tbaa !9
  %call2 = call spir_func float @_Z11vloada_halfmPKDh(i64 0, half* nonnull %1) #6
  %arrayidx3 = getelementptr inbounds float, float addrspace(1)* %f, i64 %call
  store float %call2, float addrspace(1)* %arrayidx3, align 4, !tbaa !13
  ret void
}

; Function Attrs: convergent nounwind readonly
declare spir_func i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

; Function Attrs: convergent nounwind
declare spir_func float @_Z11vloada_halfmPKDh(i64, half*) local_unnamed_addr #3

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="0" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="0" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="0" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { convergent nobuiltin nounwind readonly }
attributes #6 = { convergent nobuiltin nounwind }

!llvm.module.flags = !{!0}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.kernels = !{!2}
!host.build_options = !{!8}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{void (half addrspace(1)*, float addrspace(1)*)* @test, !3, !4, !5, !6, !7}
!3 = !{!"kernel_arg_addr_space", i32 1, i32 1}
!4 = !{!"kernel_arg_access_qual", !"none", !"none"}
!5 = !{!"kernel_arg_type", !"half*", !"float*"}
!6 = !{!"kernel_arg_base_type", !"half*", !"float*"}
!7 = !{!"kernel_arg_type_qual", !"const", !""}
!8 = !{!""}
!9 = !{!10, !10, i64 0}
!10 = !{!"short", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!14, !14, i64 0}
!14 = !{!"float", !11, i64 0}

; This test checks that an instantiated call with a constant operand gets
; that operand instantiated (packet-broadcast) correctly instead of causing the
; instantiation of the call to fail, thereby causing the packetization of the
; store to fail.
; CHECK: define spir_kernel void @__vecz_v4_test

; CHECK-GE15: %[[C0:.+]] = call spir_func float @_Z11vloada_halfmPKDh(i64 0, ptr nonnull %{{.+}})
; CHECK-LT15: %[[C0:.+]] = call spir_func float @_Z11vloada_halfmPKDh(i64 0, half* nonnull %{{.+}})
; CHECK-GE15: %[[C1:.+]] = call spir_func float @_Z11vloada_halfmPKDh(i64 0, ptr nonnull %{{.+}})
; CHECK-LT15: %[[C1:.+]] = call spir_func float @_Z11vloada_halfmPKDh(i64 0, half* nonnull %{{.+}})
; CHECK-GE15: %[[C2:.+]] = call spir_func float @_Z11vloada_halfmPKDh(i64 0, ptr nonnull %{{.+}})
; CHECK-LT15: %[[C2:.+]] = call spir_func float @_Z11vloada_halfmPKDh(i64 0, half* nonnull %{{.+}})
; CHECK-GE15: %[[C3:.+]] = call spir_func float @_Z11vloada_halfmPKDh(i64 0, ptr nonnull %{{.+}})
; CHECK-LT15: %[[C3:.+]] = call spir_func float @_Z11vloada_halfmPKDh(i64 0, half* nonnull %{{.+}})
; CHECK: %[[G0:.+]] = insertelement <4 x float> undef, float %[[C0]], {{(i32|i64)}} 0
; CHECK: %[[G1:.+]] = insertelement <4 x float> %[[G0]], float %[[C1]], {{(i32|i64)}} 1
; CHECK: %[[G2:.+]] = insertelement <4 x float> %[[G1]], float %[[C2]], {{(i32|i64)}} 2
; CHECK: %[[G3:.+]] = insertelement <4 x float> %[[G2]], float %[[C3]], {{(i32|i64)}} 3
; CHECK-GE15: store <4 x float> %[[G3]], ptr addrspace(1) %{{.+}}
; CHECK-LT15: store <4 x float> %[[G3]], <4 x float> addrspace(1)* %{{.+}}
; CHECK-NOT: store float

; CHECK: ret void