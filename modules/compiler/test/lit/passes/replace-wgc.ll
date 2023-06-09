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

; RUN: muxc --passes replace-wgc,verify -S %s | FileCheck %s

; Check that the replace-wgc correctly defines the work-group collective functions

target triple = "spir64-unknown-unknown"
target datalayout = "e-p:64:64:64-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK: @[[MINI_ACCUM:.+]] = internal addrspace(3) global i32 undef
; CHECK: @_Z29work_group_scan_inclusive_maxj.accumulator = internal addrspace(3) global i32 undef
; CHECK: @_Z29work_group_scan_exclusive_addf.accumulator = internal addrspace(3) global float undef
; CHECK: @_Z20work_group_broadcastij.accumulator = internal addrspace(3) global i32 undef

; CHECK: define spir_func i32 @_Z21work_group_reduce_mini(i32 [[PARAM:%.*]])
declare spir_func i32 @_Z21work_group_reduce_mini(i32 %x)
; CHECK-LABEL: entry:
; CHECK: %[[SUBGROUP:.+]] = call i32 @_Z20sub_group_reduce_mini(i32 %{{.+}})
; CHECK: call void @__mux_work_group_barrier(i32 0, i32 2, i32 272) [[SCHEDULE_ONCE:#[0-9]+]]
; CHECK: store i32 2147483647, [[PTR_i32:(i32 addrspace\(3\)\*)|(ptr addrspace\(3\))]] @[[MINI_ACCUM]]
; CHECK: call void @__mux_work_group_barrier(i32 0, i32 2, i32 272)
; CHECK: %[[CURRVAL:.+]] = load i32, [[PTR_i32]] @[[MINI_ACCUM]]
; CHECK: %[[ACCUM:.*]] = call i32 @llvm.smin.i32(i32 %[[CURRVAL]], i32 %[[SUBGROUP]])
; CHECK: store i32 %[[ACCUM]], [[PTR_i32]] @[[MINI_ACCUM]]
; CHECK: call void @__mux_work_group_barrier(i32 0, i32 2, i32 272)
; CHECK: %[[RESULT:.*]] = load i32, [[PTR_i32]] @[[MINI_ACCUM]]
; CHECK: ret i32 %[[RESULT]]

; CHECK: define spir_func i32 @_Z29work_group_scan_inclusive_maxj(i32 [[PARAM:%.*]])
declare spir_func i32 @_Z29work_group_scan_inclusive_maxj(i32 %x)
; CHECK-LABEL: entry:
; CHECK: call void @__mux_work_group_barrier(i32 0, i32 2, i32 272) [[SCHEDULE_ONCE]]
; CHECK: store i32 0, [[PTR_i32]] @_Z29work_group_scan_inclusive_maxj.accumulator
; CHECK: call void @__mux_work_group_barrier(i32 0, i32 2, i32 272) [[SCHEDULE_LINEAR:#[0-9]+]]
; CHECK: %[[CURRVAL:.+]] = load i32, [[PTR_i32]] @_Z29work_group_scan_inclusive_maxj.accumulator
; CHECK: %[[SCAN:.+]] = call i32 @_Z28sub_group_scan_inclusive_maxj(i32 %x)
; CHECK: %[[RESULT:.+]] = call i32 @llvm.umax.i32(i32 %[[CURRVAL]], i32 %[[SCAN]])
; CHECK: %[[SIZE:.+]] = call i32 @_Z18get_sub_group_sizev()
; CHECK: %[[LAST:.+]] = sub nuw i32 %[[SIZE]], 1
; CHECK: %[[TAIL:.+]] = call i32 @_Z19sub_group_broadcastjj(i32 %[[SCAN]], i32 %[[LAST]])
; CHECK: %[[ACCUM:.+]] = call i32 @llvm.umax.i32(i32 %[[CURRVAL]], i32 %[[TAIL]])
; CHECK: store i32 %[[ACCUM]], [[PTR_i32]] @_Z29work_group_scan_inclusive_maxj.accumulator
; CHECK: call void @__mux_work_group_barrier(i32 0, i32 2, i32 272)
; CHECK: ret i32 %[[RESULT]]


; CHECK: define spir_func float @_Z29work_group_scan_exclusive_addf(float [[PARAM:%.*]])
declare spir_func float @_Z29work_group_scan_exclusive_addf(float %x)
; CHECK-LABEL: entry:
; CHECK: call void @__mux_work_group_barrier(i32 0, i32 2, i32 272) [[SCHEDULE_ONCE]]
; CHECK: store float -0.000000e+00, [[PTR_float:(float addrspace\(3\)\*)|(ptr addrspace\(3\))]] @_Z29work_group_scan_exclusive_addf.accumulator
; CHECK: call void @__mux_work_group_barrier(i32 0, i32 2, i32 272) [[SCHEDULE_LINEAR]]
; CHECK: %[[CURRVAL:.+]] = load float, [[PTR_float]] @_Z29work_group_scan_exclusive_addf.accumulator
; CHECK: %[[SGSCAN:.+]] = call float @_Z28sub_group_scan_exclusive_addf(float %x)
; CHECK: %[[SGID:.+]] = call i32 @_Z22get_sub_group_local_idv()
; CHECK: %[[CMPID:.+]] = icmp eq i32 %[[SGID]], 0
; CHECK: %[[SELECT:.+]] = select i1 %[[CMPID]], float -0.000000e+00, float %[[SGSCAN]]
; CHECK: %[[WGSCAN:.+]] = fadd float %[[CURRVAL]], %[[SELECT]]
; CHECK: %[[SIZE:.+]] = call i32 @_Z18get_sub_group_sizev()
; CHECK: %[[LAST:.+]] = sub nuw i32 %[[SIZE]], 1
; CHECK: %[[SCAN_TAIL:.+]] = call float @_Z19sub_group_broadcastfj(float %[[SELECT]], i32 %[[LAST]])
; CHECK: %[[TAIL:.+]] = call float @_Z19sub_group_broadcastfj(float %x, i32 %[[LAST]])
; CHECK: %[[TOTAL:.+]] = fadd float %[[SCAN_TAIL]], %[[TAIL]]
; CHECK: %[[ACCUM:.+]] = fadd float %[[CURRVAL]], %[[TOTAL]]
; CHECK: store float %[[ACCUM]], [[PTR_float]] @_Z29work_group_scan_exclusive_addf.accumulator
; CHECK: call void @__mux_work_group_barrier(i32 0, i32 2, i32 272)
; CHECK: %[[LIDX:.+]] = call spir_func i64 @__mux_get_local_id(i32 0)
; CHECK: %[[CMPX:.+]] = icmp eq i64 %[[LIDX]], 0
; CHECK: %[[LIDY:.+]] = call spir_func i64 @__mux_get_local_id(i32 1)
; CHECK: %[[CMPY:.+]] = icmp eq i64 %[[LIDY]], 0
; CHECK: %[[CMPXY:.+]] = and i1 %[[CMPX]], %[[CMPY]]
; CHECK: %[[LIDZ:.+]] = call spir_func i64 @__mux_get_local_id(i32 2)
; CHECK: %[[CMPZ:.+]] = icmp eq i64 %[[LIDZ]], 0
; CHECK: %[[CMPXYZ:.+]] = and i1 %[[CMPXY]], %[[CMPZ]]
; CHECK: %[[RESULT:.+]] = select i1 %[[CMPXYZ]], float 0.000000e+00, float %[[WGSCAN]]
; CHECK: ret float %[[RESULT]]


; CHECK: define spir_func i32 @_Z20work_group_broadcastij(i32 [[PARAM:%.*]], i64 {{%.*}})
declare spir_func i32 @_Z20work_group_broadcastij(i32 %x, i64 %id)
; CHECK-LABEL: entry:
; CHECK: call i64 @_Z12get_local_idj(i32 0)

; CHECK-LABEL: broadcast:
; CHECK: store i32 [[PARAM]], ptr addrspace(3) @_Z20work_group_broadcastij.accumulator

; CHECK-LABEL: exit:
; CHECK: call void @__mux_work_group_barrier(i32 0, i32 2, i32 272)
; CHECK: [[RESULT:%.*]] = load i32, ptr addrspace(3) @_Z20work_group_broadcastij.accumulator
; CHECK: call void @__mux_work_group_barrier(i32 0, i32 2, i32 272)
; CHECK: ret i32 [[RESULT]]

; CHECK: define spir_func half @_Z29work_group_scan_exclusive_addDh(half [[PARAM:%.*]])
declare spir_func half @_Z29work_group_scan_exclusive_addDh(half %x)

; CHECK-DAG: attributes [[SCHEDULE_ONCE]] = { "mux-barrier-schedule"="once" }
; CHECK-DAG: attributes [[SCHEDULE_LINEAR]] = { "mux-barrier-schedule"="linear" }

!opencl.ocl.version = !{!0}
!0 = !{i32 3, i32 0}
