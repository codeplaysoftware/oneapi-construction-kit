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

/// @file
///
/// @brief Write kernel metadata into a binary.

#ifndef COMPILER_UTILS_METADATA_HOOKS_H_INCLUDED
#define COMPILER_UTILS_METADATA_HOOKS_H_INCLUDED

#include <llvm/IR/Module.h>
#include <metadata/metadata.h>

#include <string>

namespace compiler {
namespace utils {

constexpr const char MD_NOTES_SECTION[] = "notes";
constexpr const char MD_BLOCK_NAME[] = "KernelMetadata";

/// @brief Get hooks for writing metadata into an elf binary with the binary
/// metadata API.
///
/// @return md_hooks
md_hooks getElfMetadataWriteHooks();

}  // namespace utils
}  // namespace compiler

#endif  // COMPILER_UTILS_METADATA_HOOKS_H_INCLUDED
