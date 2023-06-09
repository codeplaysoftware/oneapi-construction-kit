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

#include "cl_codeplay_wfv.h"

TEST_F(cl_codeplay_wfv_Test, KernelStatusNone) {
  BuildKernel("__kernel void foo() {}", "foo", "-cl-wfv=never");
  cl_kernel_wfv_status_codeplay status;
  ASSERT_SUCCESS(clGetKernelWFVInfoCODEPLAY(kernel, device, 1, nullptr, nullptr,
                                            CL_KERNEL_WFV_STATUS_CODEPLAY,
                                            sizeof(status), &status, nullptr));
  ASSERT_EQ(CL_WFV_NONE_CODEPLAY, status);
}

// Disabling as some targets vectorize and encode in the binary the
// vectorization information - which breaks this path - see CA-4025
TEST_F(cl_codeplay_wfv_Test, DISABLED_KernelStatusSuccess) {
  BuildKernel("__kernel void foo() {}", "foo", "-cl-wfv=always");
  cl_kernel_wfv_status_codeplay status;
  ASSERT_SUCCESS(clGetKernelWFVInfoCODEPLAY(kernel, device, 1, nullptr, nullptr,
                                            CL_KERNEL_WFV_STATUS_CODEPLAY,
                                            sizeof(status), &status, nullptr));
  ASSERT_EQ(CL_WFV_SUCCESS_CODEPLAY, status);
}
