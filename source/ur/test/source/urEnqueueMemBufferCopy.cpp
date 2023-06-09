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

#include "uur/fixtures.h"

struct urEnqueueMemBufferCopyTest : uur::QueueTest {
  void SetUp() override {
    UUR_RETURN_ON_FATAL_FAILURE(QueueTest::SetUp());
    ASSERT_SUCCESS(urMemBufferCreate(context, UR_MEM_FLAG_WRITE_ONLY, size,
                                     nullptr, &src_buffer));
    ASSERT_SUCCESS(urMemBufferCreate(context, UR_MEM_FLAG_READ_ONLY, size,
                                     nullptr, &dst_buffer));
    input.assign(count, 42);
    ASSERT_SUCCESS(urEnqueueMemBufferWrite(queue, src_buffer, true, 0, size,
                                           input.data(), 0, nullptr, nullptr));
  }

  void TearDown() override {
    if (src_buffer) {
      EXPECT_SUCCESS(urMemRelease(src_buffer));
    }
    if (src_buffer) {
      EXPECT_SUCCESS(urMemRelease(dst_buffer));
    }
    QueueTest::TearDown();
  }

  const size_t count = 1024;
  const size_t size = sizeof(uint32_t) * count;
  ur_mem_handle_t src_buffer = nullptr;
  ur_mem_handle_t dst_buffer = nullptr;
  std::vector<uint32_t> input;
};
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urEnqueueMemBufferCopyTest);

TEST_P(urEnqueueMemBufferCopyTest, Success) {
  ASSERT_SUCCESS(urEnqueueMemBufferCopy(queue, src_buffer, dst_buffer, 0, 0,
                                        size, 0, nullptr, nullptr));
  std::vector<uint32_t> output(count, 1);
  ASSERT_SUCCESS(urEnqueueMemBufferRead(queue, dst_buffer, true, 0, size,
                                        output.data(), 0, nullptr, nullptr));
  ASSERT_EQ(input, output);
}

TEST_P(urEnqueueMemBufferCopyTest, InvalidNullHandleQueue) {
  ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE,
                   urEnqueueMemBufferCopy(nullptr, src_buffer, dst_buffer, 0, 0,
                                          size, 0, nullptr, nullptr));
}

TEST_P(urEnqueueMemBufferCopyTest, InvalidNullHandleBufferSrc) {
  ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE,
                   urEnqueueMemBufferCopy(queue, nullptr, dst_buffer, 0, 0,
                                          size, 0, nullptr, nullptr));
}

TEST_P(urEnqueueMemBufferCopyTest, InvalidNullHandleBufferDst) {
  ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE,
                   urEnqueueMemBufferCopy(queue, src_buffer, nullptr, 0, 0,
                                          size, 0, nullptr, nullptr));
}

using urEnqueueMemBufferCopyMultiDeviceTest =
    uur::MultiDeviceMemBufferQueueTest;

TEST_F(urEnqueueMemBufferCopyMultiDeviceTest, CopyReadDifferentQueues) {
  // First queue does a fill.
  const uint32_t input = 42;
  ASSERT_SUCCESS(urEnqueueMemBufferFill(
      queues[0], buffer, &input, sizeof(input), 0, size, 0, nullptr, nullptr));

  // Then a copy.
  ur_mem_handle_t dst_buffer = nullptr;
  ASSERT_SUCCESS(urMemBufferCreate(context, UR_MEM_FLAG_READ_ONLY, size,
                                   nullptr, &dst_buffer));
  EXPECT_SUCCESS(urEnqueueMemBufferCopy(queues[0], buffer, dst_buffer, 0, 0,
                                        size, 0, nullptr, nullptr));

  // Wait for the queue to finish executing.
  EXPECT_SUCCESS(urEnqueueEventsWait(queues[0], 0, nullptr, nullptr));

  // Then the remaining queues do blocking reads from the buffer. Since the
  // queues target different devices this checks that any devices memory has
  // been synchronized.
  for (unsigned i = 1; i < queues.size(); ++i) {
    const auto queue = queues[i];
    std::vector<uint32_t> output(count, 0);
    EXPECT_SUCCESS(urEnqueueMemBufferRead(queue, dst_buffer, true, 0, size,
                                          output.data(), 0, nullptr, nullptr));
    for (unsigned j = 0; j < count; ++j) {
      EXPECT_EQ(input, output[j])
          << "Result on queue " << i << " did not match at index " << j << "!";
    }
  }

  EXPECT_SUCCESS(urMemRelease(dst_buffer));
}
