; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; REQUIRES: llvm-13+
; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -k f -vecz-scalable -vecz-simd-width=4 -vecz-choices=VectorPredication:FullScalarization -S < %s | %filecheck %t

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown"

define spir_kernel void @f(<4 x double> addrspace(1)* %a, <4 x double> addrspace(1)* %b, <4 x double> addrspace(1)* %c, <4 x double> addrspace(1)* %d, <4 x double> addrspace(1)* %e, i8 addrspace(1)* %flag) {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 0)
  %add.ptr = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %b, i64 %call
  %.cast = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %add.ptr, i64 0, i64 0
  %0 = load <4 x double>, <4 x double> addrspace(1)* %add.ptr, align 32
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

; Test if the interleaved load is defined correctly
; Vector-predicated interleaved loads are always masked
; CHECK-GE15: define <vscale x 4 x double> @__vecz_b_masked_interleaved_load8_vp_4_u5nxv4du3ptrU3AS1u5nxv4bj(ptr addrspace(1){{( %0)?}}, <vscale x 4 x i1>{{( %1)?}}, i32{{( %2)?}}) {
; CHECK-LT15: define <vscale x 4 x double> @__vecz_b_masked_interleaved_load8_vp_4_u5nxv4dPU3AS1du5nxv4bj(double addrspace(1)*{{( %0)?}}, <vscale x 4 x i1>{{( %1)?}}, i32{{( %2)?}}) {
; CHECK: entry:
; CHECK-GE15:   %BroadcastAddr.splatinsert = insertelement <vscale x 4 x ptr addrspace(1)> poison, ptr addrspace(1) %0, i32 0
; CHECK-LT15:   %BroadcastAddr.splatinsert = insertelement <vscale x 4 x double addrspace(1)*> poison, double addrspace(1)* %0, i32 0
; CHECK-GE15:   %BroadcastAddr.splat = shufflevector <vscale x 4 x ptr addrspace(1)> %BroadcastAddr.splatinsert, <vscale x 4 x ptr addrspace(1)> poison, <vscale x 4 x i32> zeroinitializer
; CHECK-LT15:   %BroadcastAddr.splat = shufflevector <vscale x 4 x double addrspace(1)*> %BroadcastAddr.splatinsert, <vscale x 4 x double addrspace(1)*> poison, <vscale x 4 x i32> zeroinitializer
; CHECK:   %3 = call <vscale x 4 x i64> @llvm.experimental.stepvector.nxv4i64()
; CHECK:   %4 = mul <vscale x 4 x i64> shufflevector (<vscale x 4 x i64> insertelement (<vscale x 4 x i64> poison, i64 4, i32 0), <vscale x 4 x i64> poison, <vscale x 4 x i32> zeroinitializer), %3
; CHECK-GE15:   %5 = getelementptr double, <vscale x 4 x ptr addrspace(1)> %BroadcastAddr.splat, <vscale x 4 x i64> %4
; CHECK-LT15:   %5 = getelementptr double, <vscale x 4 x double addrspace(1)*> %BroadcastAddr.splat, <vscale x 4 x i64> %4
; CHECK-GE15:   %6 = call <vscale x 4 x double> @llvm.vp.gather.nxv4f64.nxv4p1(<vscale x 4 x ptr addrspace(1)> %5, <vscale x 4 x i1> %1, i32 %2)
; CHECK-LT15:   %6 = call <vscale x 4 x double> @llvm.vp.gather.nxv4f64.nxv4p1f64(<vscale x 4 x double addrspace(1)*> %5, <vscale x 4 x i1> %1, i32 %2)
; CHECK:   ret <vscale x 4 x double> %6
; CHECK: }


; Test if the interleaved store is defined correctly
; Vector-predicated interleaved stores are always masked
; CHECK-GE15: define void @__vecz_b_masked_interleaved_store8_vp_4_u5nxv4du3ptrU3AS1u5nxv4bj(<vscale x 4 x double>{{( %0)?}}, ptr addrspace(1){{( %1)?}}, <vscale x 4 x i1>{{( %2)?}}, i32{{( %3)?}})
; CHECK-LT15: define void @__vecz_b_masked_interleaved_store8_vp_4_u5nxv4dPU3AS1du5nxv4bj(<vscale x 4 x double>{{( %0)?}}, double addrspace(1)*{{( %1)?}}, <vscale x 4 x i1>{{( %2)?}}, i32{{( %3)?}})
; CHECK: entry:
; CHECK-GE15:  %BroadcastAddr.splatinsert = insertelement <vscale x 4 x ptr addrspace(1)> poison, ptr addrspace(1) %1, i32 0
; CHECK-LT15:  %BroadcastAddr.splatinsert = insertelement <vscale x 4 x double addrspace(1)*> poison, double addrspace(1)* %1, i32 0
; CHECK-GE15:  %BroadcastAddr.splat = shufflevector <vscale x 4 x ptr addrspace(1)> %BroadcastAddr.splatinsert, <vscale x 4 x ptr addrspace(1)> poison, <vscale x 4 x i32> zeroinitializer
; CHECK-LT15:  %BroadcastAddr.splat = shufflevector <vscale x 4 x double addrspace(1)*> %BroadcastAddr.splatinsert, <vscale x 4 x double addrspace(1)*> poison, <vscale x 4 x i32> zeroinitializer
; CHECK:  %4 = call <vscale x 4 x i64> @llvm.experimental.stepvector.nxv4i64()
; CHECK:  %5 = mul <vscale x 4 x i64> shufflevector (<vscale x 4 x i64> insertelement (<vscale x 4 x i64> poison, i64 4, i32 0), <vscale x 4 x i64> poison, <vscale x 4 x i32> zeroinitializer), %4
; CHECK-GE15:  %6 = getelementptr double, <vscale x 4 x ptr addrspace(1)> %BroadcastAddr.splat, <vscale x 4 x i64> %5
; CHECK-LT15:  %6 = getelementptr double, <vscale x 4 x double addrspace(1)*> %BroadcastAddr.splat, <vscale x 4 x i64> %5
; CHECK-GE15:  call void @llvm.vp.scatter.nxv4f64.nxv4p1(<vscale x 4 x double> %0, <vscale x 4 x ptr addrspace(1)> %6, <vscale x 4 x i1> %2, i32 %3)
; CHECK-LT15:  call void @llvm.vp.scatter.nxv4f64.nxv4p1f64(<vscale x 4 x double> %0, <vscale x 4 x double addrspace(1)*> %6, <vscale x 4 x i1> %2, i32 %3)
; CHECK:  ret void
; CHECK: }