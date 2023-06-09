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
#include "compiler/context.h"
#include "compiler/info.h"
#include "compiler/library.h"

extern "C" {
const char *caCompilerLLVMVersion() { return compiler::llvmVersion(); }

// MSVC will refuse to compile a function with C linkage that returns a C++
// type. Therefore, we use an out parameter instead.
void caCompilers(cargo::array_view<const compiler::Info *> *out_compilers) {
  assert(out_compilers && "out_compilers must not be nullptr");
  *out_compilers = compiler::compilers();
}

const compiler::Info *caGetCompilerForDevice(mux_device_info_t device_info) {
  return compiler::getCompilerForDevice(device_info);
}

// MSVC will refuse to compile a function with C linkage that returns a C++
// type. Therefore, we release and return the raw pointer instead. The loader
// library will then re-wrap it in a unique_ptr.
compiler::Context *caCompilerCreateContext() {
  return compiler::createContext().release();
}
}
