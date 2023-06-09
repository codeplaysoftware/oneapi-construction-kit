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

#ifndef REFSI_G1_TARGET_H_INCLUDED
#define REFSI_G1_TARGET_H_INCLUDED

#include <hal_riscv.h>
#include <llvm/Target/TargetMachine.h>
#include <multi_llvm/multi_llvm.h>
#include <refsi_g1_wi/module.h>
#include <riscv/target.h>

namespace refsi_g1_wi {

/// @brief Compiler target class.
class RefSiG1Target final : public riscv::RiscvTarget {
 public:
  RefSiG1Target(const compiler::Info *compiler_info,
                const riscv::hal_device_info_riscv_t *hal_device_info,
                compiler::Context *context, compiler::NotifyCallbackFn callback)
      : riscv::RiscvTarget(compiler_info, hal_device_info, context, callback) {}

  /// @see BaseTarget::createModule
  std::unique_ptr<compiler::Module> createModule(uint32_t &num_errors,
                                                 std::string &log) override {
    return std::make_unique<RefSiG1Module>(
        *this, static_cast<compiler::BaseContext &>(context), num_errors, log);
  }
};
}  // namespace refsi_g1_wi

#endif  // REFSI_G1_TARGET_H_INCLUDED
