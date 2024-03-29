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

; RUN: muxc --passes "encode-builtin-range-metadata<max-local-sizes=8:1>,verify" -S %s \
; RUN:   | FileCheck %s --check-prefixes RANGES,LOCAL-RANGES
; RUN: muxc --passes "encode-builtin-range-metadata<max-global-sizes=64;max-local-sizes=8:1>,verify" -S %s \
; RUN:   | FileCheck %s --check-prefixes RANGES,GLOBAL-RANGES
; RUN: muxc --passes "function(instcombine)" -S %s \
; RUN:   | FileCheck %s --check-prefix IC-NO-RANGES
; RUN: muxc --passes "encode-builtin-range-metadata<max-global-sizes=64;max-local-sizes=8:1>,function(instcombine,sccp)" -S %s \
; RUN:   | FileCheck %s --check-prefix IC-RANGES

target triple = "spir64-unknown-unknown"
target datalayout = "e-p:64:64:64-m:e-i64:64-f80:128-n8:16:32:64-S128"

; RANGES-LABEL: define spir_kernel void @foo
; RANGES: call i32 @__mux_get_work_dim(), !range ![[WORKDIM:[0-9]+]]
; RANGES: call i64 @__mux_get_local_size(i32 0), !range ![[LOCALSIZEX:[0-9]+]]
; RANGES: call i64 @__mux_get_local_size(i32 1), !range ![[LOCALSIZEY:[0-9]+]]
; RANGES-NOT: call i64 @__mux_get_local_size(i32 2), !range
; RANGES: call i64 @__mux_get_local_id(i32 0), !range ![[LOCALID:[0-9]+]]

; LOCAL-RANGES-NOT: call i64 @__mux_get_global_size(i32 0), !range
; GLOBAL-RANGES:    call i64 @__mux_get_global_size(i32 0), !range ![[GLOBALSIZE:[0-9]+]]

; Check that without ranges we can't optimize anything meaningful
; IC-NO-RANGES-LABEL: define spir_kernel void @foo
; IC-NO-RANGES: call void @ext(i1 %cmp_dims, i64 %tmp1, i64 %lsizey)

; Check we can:
; * constant-fold the dimension check
; * do away with the trunc/sext of the local size X
; * infer the local size Y is the constant 1 (SCCP does this)
; IC-RANGES-LABEL: define spir_kernel void @foo
; IC-RANGES: call void @ext(i1 true, i64 %lsizex, i64 1)
define spir_kernel void @foo() {
  %dims = call i32 @__mux_get_work_dim()
  %cmp_dims = icmp slt i32 %dims, 5

  %lsizex = call i64 @__mux_get_local_size(i32 0)
  %tmp0 = trunc i64 %lsizex to i32
  %tmp1 = sext i32 %tmp0 to i64

  %lsizey = call i64 @__mux_get_local_size(i32 1)
  %lsizez = call i64 @__mux_get_local_size(i32 2)

  %lid = call i64 @__mux_get_local_id(i32 0)

  %gsize = call i64 @__mux_get_global_size(i32 0)

  call void @ext(i1 %cmp_dims, i64 %tmp1, i64 %lsizey)
  ret void
}

declare void @ext(i1, i64, i64)

; RANGES: ![[WORKDIM]] = !{i32 1, i32 4}
; RANGES: ![[LOCALSIZEX]] = !{i64 1, i64 9}
; RANGES: ![[LOCALSIZEY]] = !{i64 1, i64 2}
; RANGES: ![[LOCALID]] = !{i64 0, i64 8}
; GLOBAL-RANGES: ![[GLOBALSIZE]] = !{i64 1, i64 65}

declare i32 @__mux_get_work_dim()
declare i64 @__mux_get_local_id(i32)
declare i64 @__mux_get_local_size(i32)
declare i64 @__mux_get_global_size(i32)
