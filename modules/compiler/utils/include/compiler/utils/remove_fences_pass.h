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
/// Remove fences pass

#ifndef COMPILER_UTILS_REMOVE_FENCES_PASS_H_INCLUDED
#define COMPILER_UTILS_REMOVE_FENCES_PASS_H_INCLUDED

#include <llvm/IR/PassManager.h>

namespace compiler {
namespace utils {

/// @brief A pass that removes all memory fence instructions.
///
/// ENORMOUS WARNING:
/// Removing memory fences can result in invalid code or incorrect behaviour in
/// general. This pass is a workaround for backends that do not yet support
/// memory fences.
class RemoveFencesPass final : public llvm::PassInfoMixin<RemoveFencesPass> {
 public:
  llvm::PreservedAnalyses run(llvm::Function &,
                              llvm::FunctionAnalysisManager &);
};

}  // namespace utils
}  // namespace compiler

#endif  // COMPILER_UTILS_REMOVE_FENCES_PASS_H_INCLUDED
