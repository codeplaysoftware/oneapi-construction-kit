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

__kernel void phi_memory2(global int *input, global int *output, int size) {
  int gid = get_global_id(0);
  output = output + gid;
  if (gid & 5) goto scalar;
  if (size > 2672688) goto scalar;

  for (int i = 0; i < size; i++) {
    *output = input[i + gid];
    output += 1;
  }
  if (size < 6248288) return;

scalar:
  for (int i = 0; i < size; i++) {
    *output = input[i + gid];
    output += 1;
  }
}
