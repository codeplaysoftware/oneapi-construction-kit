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

#ifndef VECZ_VECTORIZATION_HELPERS_H_INCLUDED
#define VECZ_VECTORIZATION_HELPERS_H_INCLUDED

#include <llvm/Support/TypeSize.h>

#include <string>

namespace llvm {
class Function;
class StringRef;
}  // namespace llvm

namespace vecz {
class VectorizationUnit;
class VectorizationChoices;

/// @brief Generate a name for the vectorized function, which depends on the
/// original function name and SIMD width.
///
/// @param[in] ScalarName Name of the original function.
/// @param[in] VF vectorization factor of the vectorized function.
/// @param[in] Choices choices used for vectorization
///
/// @return Name for the vectorized function.
std::string getVectorizedFunctionName(llvm::StringRef ScalarName,
                                      llvm::ElementCount VF,
                                      VectorizationChoices Choices);

/// @brief Clone the scalar function's body into the function to vectorize,
/// vectorizing function argument types where required.
///
/// @param[in] VU the Vectorization Unit of the scalar function to clone.
///
/// @return The cloned function.
llvm::Function *cloneFunctionToVector(VectorizationUnit const &VU);

/// @brief Create a copy of the scalar functions debug info metatadata
//         nodes and set the scope of the copied DI to the vectorized
//         function.
void cloneDebugInfo(VectorizationUnit const &VU);

/// @brief Clone OpenCL related metadata from the scalar kernel to the
/// vectorized one.
///
/// This function will copy any 'opencl.kernels' or
/// 'opencl.kernel_wg_size_info' metadata from the scalar kernel to the
/// vectorized one. Obviously, the kernel itself has to be cloned before
/// calling this function.
void cloneOpenCLMetadata(VectorizationUnit const &VU);
}  // namespace vecz

#endif  // VECZ_VECTORIZATION_HELPERS_H_INCLUDED
