; Copyright (C) Codeplay Software Limited
;
; Licensed under the Apache License, Version 2.0 (the "License") with LLVM
; Exceptions; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;     https://github.com/codeplaysoftware/oneapi-construction-kit/blob/main/LICENSE.txt
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
; WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
; License for the specific language governing permissions and limitations
; under the License.
;
; SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

; RUN: veczc -k dummy -vecz-simd-width=4 -vecz-passes=define-builtins -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown"

define spir_kernel void @dummy(i32 addrspace(2)* %in, i32 addrspace(1)* %out) {
  ; Dummy uses of the builtins, as we don't define any with zero uses.
  %a = call <4 x i32> @__vecz_b_sub_group_scan_inclusive_add_Dv4_j(<4 x i32> zeroinitializer)
  %b = call <4 x i32> @__vecz_b_sub_group_scan_exclusive_add_Dv4_j(<4 x i32> zeroinitializer)
  ret void
}

declare <4 x i32> @__vecz_b_sub_group_scan_inclusive_add_Dv4_j(<4 x i32>)
; CHECK-LABEL: define <4 x i32> @__vecz_b_sub_group_scan_inclusive_add_Dv4_j(<4 x i32> %0) {
; CHECK: entry:
; CHECK:  %[[SHUF1:.+]] = shufflevector <4 x i32> %0, <4 x i32> <i32 0, i32 undef, i32 undef, i32 undef>, <4 x i32> <i32 4, i32 0, i32 4, i32 2>
; CHECK:  %[[ADD1:.+]] = add <4 x i32> %0, %[[SHUF1]]
; CHECK:  %[[SHUF2:.+]] = shufflevector <4 x i32> %[[ADD1]], <4 x i32> <i32 0, i32 undef, i32 undef, i32 undef>, <4 x i32> <i32 4, i32 4, i32 1, i32 1>
; CHECK:  %[[RESULT:.+]] = add <4 x i32> %[[ADD1]], %[[SHUF2]]
; CHECK:  ret <4 x i32> %[[RESULT]]
; CHECK: }

declare <4 x i32> @__vecz_b_sub_group_scan_exclusive_add_Dv4_j(<4 x i32>)
; CHECK-LABEL: define <4 x i32> @__vecz_b_sub_group_scan_exclusive_add_Dv4_j(<4 x i32> %0) {
; CHECK: entry:
; CHECK:  %[[SHUF1:.+]] = shufflevector <4 x i32> %0, <4 x i32> <i32 0, i32 undef, i32 undef, i32 undef>, <4 x i32> <i32 4, i32 0, i32 4, i32 2>
; CHECK:  %[[ADD1:.+]] = add <4 x i32> %0, %[[SHUF1]]
; CHECK:  %[[SHUF2:.+]] = shufflevector <4 x i32> %[[ADD1]], <4 x i32> <i32 0, i32 undef, i32 undef, i32 undef>, <4 x i32> <i32 4, i32 4, i32 1, i32 1>
; CHECK:  %[[ADD2:.+]] = add <4 x i32> %[[ADD1]], %[[SHUF2]]
; CHECK:  %[[ROTATE:.+]] = shufflevector <4 x i32> %[[ADD2]], <4 x i32> undef, <4 x i32> <i32 {{.+}}, i32 0, i32 1, i32 2>
; CHECK:  %[[RESULT:.+]] = insertelement <4 x i32> %[[ROTATE]], i32 0, i64 0
; CHECK:  ret <4 x i32> %[[RESULT]]
; CHECK: }
