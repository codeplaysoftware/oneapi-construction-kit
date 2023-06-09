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

struct __attribute__((packed)) PerItemKernelInfo {
  uint4 global_id;
  uint4 group_id;
};

void kernel work_item(global struct PerItemKernelInfo* info) {
  size_t xId = get_global_id(0);
  size_t yId = get_global_id(1);
  size_t zId = get_global_id(2);
  size_t id = xId + (get_global_size(0) * yId) +
              (get_global_size(0) * get_global_size(1) * zId);
  info[id].global_id =
      (uint4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);
  info[id].group_id =
      (uint4)(get_group_id(0), get_group_id(1), get_group_id(2), 0);
}
