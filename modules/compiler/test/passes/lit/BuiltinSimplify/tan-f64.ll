; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %muxc --passes builtin-simplify,verify -S %s | %filecheck %s

target triple = "spir64-unknown-unknown"
target datalayout = "e-p:64:64:64-m:e-i64:64-f80:128-n8:16:32:64-S128"

define spir_kernel void @tan_no_fold(double addrspace(1)* %d) {
entry:
  %0 = load double, double addrspace(1)* %d, align 8
  %call = tail call spir_func double @_Z3tand(double %0)
  store double %call, double addrspace(1)* %d, align 8
  ret void
}

declare spir_func double @_Z3tand(double)

define spir_kernel void @tan_fold(double addrspace(1)* %d, <2 x double> addrspace(1)* %d2) {
entry:
  %call = tail call spir_func double @_Z3tand(double 1.000000e+00)
  store double %call, double addrspace(1)* %d, align 8
  %call1 = tail call spir_func <2 x double> @_Z3tanDv2_d(<2 x double> <double 1.000000e+00, double 2.000000e+00>)
  store <2 x double> %call1, <2 x double> addrspace(1)* %d2, align 16
  ret void
}

declare spir_func <2 x double> @_Z3tanDv2_d(<2 x double>)

define spir_kernel void @half_tan_no_fold(double addrspace(1)* %d) {
entry:
  %0 = load double, double addrspace(1)* %d, align 8
  %call = tail call spir_func double @_Z8half_tand(double %0)
  store double %call, double addrspace(1)* %d, align 8
  ret void
}

declare spir_func double @_Z8half_tand(double)

define spir_kernel void @native_tan_no_fold(double addrspace(1)* %d) {
entry:
  %0 = load double, double addrspace(1)* %d, align 8
  %call = tail call spir_func double @_Z10native_tand(double %0)
  store double %call, double addrspace(1)* %d, align 8
  ret void
}

declare spir_func double @_Z10native_tand(double)

define spir_kernel void @tan_double_call(double addrspace(1)* %d) {
entry:
  %call = tail call spir_func double @_Z3tand(double 1.000000e+00)
  %call1 = tail call spir_func double @_Z3tand(double %call)
  store double %call1, double addrspace(1)* %d, align 8
  ret void
}

; CHECK: define {{(dso_local )?}}spir_kernel void @tan_no_fold
; CHECK: call spir_func double @_Z3tand
; CHECK: define {{(dso_local )?}}spir_kernel void @tan_fold
; CHECK-NOT: call spir_func double @_Z3tand
; CHECK-NOT: call spir_func <2 x double> @_Z3tanDv2_d
; CHECK: define {{(dso_local )?}}spir_kernel void @half_tan_no_fold
; CHECK: call spir_func double @_Z8half_tand
; CHECK: define {{(dso_local )?}}spir_kernel void @native_tan_no_fold
; CHECK: call spir_func double @_Z10native_tand
; CHECK: define {{(dso_local )?}}spir_kernel void @tan_double_call
; CHECK-NOT: call spir_func double @_Z3tand