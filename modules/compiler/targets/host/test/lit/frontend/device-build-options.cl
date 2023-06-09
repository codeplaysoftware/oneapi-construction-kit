// Copyright (C) Codeplay Software Limited
//
// Licensed under the Apache License, Version 2.0 (the "License") with LLVM
// Exceptions; you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://github.com/codeplaysoftware/oneapi-construction-kit/blob/main/LICENSE.txt
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Run the following pipeline to check that host options are preserved:
// * host-late-passes adds host.build_options metadata
// * transfer-kernel-metadata sets up kernel entry points
// * host-kernel-passes runs the final pipeline

// This metadata is only set when CA_ENABLE_DEBUG_SUPPORT is enabled
// REQUIRES: debug

// RUN: muxc --device "%default_device" -cl-options "--dummy-host-flag --dummy-host-option test_value" \
// RUN:   --passes host-late-passes,transfer-kernel-metadata,host-kernel-passes %s \
// RUN:   | FileCheck %s --check-prefixes=CHECK,CHECK-FLAGVAL

// RUN: muxc --device "%default_device" -cl-options "--dummy-host-option2" \
// RUN:   --passes host-late-passes,transfer-kernel-metadata,host-kernel-passes %s \
// RUN:   | FileCheck %s --check-prefixes=CHECK,CHECK-NOSPACE

// RUN: muxc --device "%default_device" -cl-options "--dummy-host-flag --dummy-host-flag2" \
// RUN:   --passes host-late-passes,transfer-kernel-metadata,host-kernel-passes %s \
// RUN:   | FileCheck %s --check-prefixes=CHECK,CHECK-BOTHFLAGS

__kernel void add(__global int* input1,
                  __global int* input2,
                  __global int* output) {
  int gid = get_local_id(0);
  output[gid] = input1[gid] + input2[gid];
}

// CHECK: !host.build_options = !{[[MD_STRING:![0-9]+]]}

// CHECK-FLAGVAL: [[MD_STRING]] = !{!"--dummy-host-flag,;--dummy-host-option,test_value"}
// CHECK-NOSPACE: [[MD_STRING]] = !{!"--dummy-host-option,2"}
// CHECK-BOTHFLAGS: [[MD_STRING]] = !{!"--dummy-host-flag,;--dummy-host-flag2,"}
