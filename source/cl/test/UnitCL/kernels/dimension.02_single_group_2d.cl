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

__kernel void single_group_2d(__global int *A, uint groupX, uint groupY) {
  size_t id = get_global_id(0) + get_global_id(1) * get_global_size(0);
  A[id] = 0;

  barrier(CLK_GLOBAL_MEM_FENCE);

  if ((get_group_id(0) == groupX) && (get_group_id(1) == groupY)) {
    A[id] = 1;
  }
}
