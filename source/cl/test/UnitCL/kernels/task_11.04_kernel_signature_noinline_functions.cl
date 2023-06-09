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

__attribute__((noinline)) size_t function_noinline_a(size_t a) { return a; }

kernel __attribute__((noinline)) void kernel_signature_noinline_functions(
    global int* in, global int* out) {
  size_t gid = function_noinline_a(get_global_id(0));
  out[gid] = in[gid];
}

__attribute__((noinline)) size_t function_noinline_ab(size_t b, int a) {
  return a;
}

__attribute__((noinline)) size_t function_noinline() {
  return get_global_id(0);
}

kernel __attribute__((noinline)) void
kernel_signature_noinline_functions_second(global int* in, global int* out) {
  size_t gid = function_noinline();
  gid = function_noinline_ab(gid, gid);
  out[gid] = in[gid];
}