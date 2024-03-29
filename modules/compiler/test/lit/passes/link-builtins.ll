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

; RUN: muxc --passes link-builtins,verify -S %s | FileCheck %s

target triple = "spir64-unknown-unknown"
target datalayout = "e-p:64:64:64-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Without a target, there's no builtins to link
; CHECK: declare spir_func i32 @_Z3absi(i32)

declare spir_func i32 @_Z3absi(i32)
declare i64 @__mux_get_global_id(i32)

define spir_kernel void @foo(i32 addrspace(1)* %in) {
  %gid = call i64 @__mux_get_global_id(i32 0)
  %addr = getelementptr i32, i32 addrspace(1)* %in, i64 %gid
  %x = load i32, i32 addrspace(1)* %addr
  %abs = call i32 @_Z3absi(i32 %x)
  store i32 %abs, i32 addrspace(1)* %addr
  ret void
}
