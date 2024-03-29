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

; RUN: muxc --passes work-item-loops,verify -S %s  | FileCheck %s

target triple = "spir64-unknown-unknown"
target datalayout = "e-p:64:64:64-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK: @foo.mux-barrier-region(
; CHECK: call i64 @__mux_get_global_id(i32 0)

; Check we've rematerialized the call to __mux_get_global_id, rather than
; storing it in the live variables structure.
; CHECK: @foo.mux-barrier-region.1(
; CHECK: call i64 @__mux_get_global_id(i32 0)

define internal void @foo(ptr addrspace(1) %d, ptr addrspace(1) %a, ptr addrspace(1) %b) #0 {
entry:
  %call = tail call i64 @__mux_get_global_id(i32 0)
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %a, i64 %call
  %0 = load i32, ptr addrspace(1) %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, ptr addrspace(1) %b, i64 %call
  %1 = load i32, ptr addrspace(1) %arrayidx1, align 4
  %add = add nsw i32 %1, %0
  tail call void @__mux_work_group_barrier(i32 0, i32 1, i32 272)
  %arrayidx2 = getelementptr inbounds i32, ptr addrspace(1) %d, i64 %call
  store i32 %add, ptr addrspace(1) %arrayidx2, align 4
  ret void
}

declare i64 @__mux_get_global_id(i32)
declare void @__mux_work_group_barrier(i32, i32, i32)

attributes #0 = { "mux-kernel"="entry-point" }
