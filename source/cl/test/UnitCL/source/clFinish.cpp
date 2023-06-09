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

#include <thread>

#include "Common.h"

using clFinishTest = ucl::CommandQueueTest;

TEST_F(clFinishTest, InvalidCommandQueue) {
  ASSERT_EQ_ERRCODE(CL_INVALID_COMMAND_QUEUE, clFinish(nullptr));
}

TEST_F(clFinishTest, Default) { ASSERT_SUCCESS(clFinish(command_queue)); }

// This test can essentially only fail under a thread-sanitizer build as it
// doesn't "do" anything, so it will never get the wrong result.  The original
// issue being tracked down was a rare crash though, not an incorrect result.
//
// It is aiming to cause enqueuing-work and flushes to be happening
// concurrently on a single cl_command_queue.
//
// See also `TEST_F(clFlushTest, ConcurrentFlushes).
TEST_F(clFinishTest, ConcurrentFinishes) {
  if (!getDeviceCompilerAvailable()) {
    GTEST_SKIP();
  }
  const char *src = "kernel void k() {}";
  const size_t range = 1;

  cl_int errcode = !CL_SUCCESS;
  cl_program program =
      clCreateProgramWithSource(context, 1, &src, nullptr, &errcode);
  EXPECT_TRUE(program);
  ASSERT_SUCCESS(errcode);

  ASSERT_SUCCESS(
      clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr));
  cl_kernel kernel = clCreateKernel(program, "k", &errcode);
  EXPECT_TRUE(kernel);
  ASSERT_SUCCESS(errcode);

  auto worker = [this, &kernel, &range]() {
    for (int i = 0; i < 32; i++) {
      ASSERT_SUCCESS(clEnqueueNDRangeKernel(command_queue, kernel, 1, nullptr,
                                            &range, nullptr, 0, nullptr,
                                            nullptr));

      ASSERT_SUCCESS(clFinish(command_queue));
    }
  };

  const size_t threads = 4;
  UCL::vector<std::thread> workers(threads);

  for (size_t i = 0; i < threads; i++) {
    workers[i] = std::thread(worker);
  }

  for (size_t i = 0; i < threads; i++) {
    workers[i].join();
  }

  clReleaseKernel(kernel);
  clReleaseProgram(program);
}
