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

#include "uur/checks.h"
#include "uur/fixtures.h"

using urEnqueueKernelLaunchTest = uur::KernelTest;

UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urEnqueueKernelLaunchTest);

TEST_P(urEnqueueKernelLaunchTest, Success) {
  const size_t offsets[]{0, 0, 0};
  const size_t global_size[]{32};
  const size_t local_size[]{8};
  ur_event_handle_t event = nullptr;
  ASSERT_SUCCESS(urEnqueueKernelLaunch(queue, kernel, 1, offsets, global_size,
                                       local_size, 0, nullptr, &event));
  EXPECT_NE(event, nullptr);
  EXPECT_SUCCESS(urQueueFlush(queue));
  EXPECT_SUCCESS(urEventWait(1, &event));
  EXPECT_SUCCESS(urEventRelease(event));
}

TEST_P(urEnqueueKernelLaunchTest, InvalidNullHandleQueue) {
  const size_t offsets[]{0, 0, 0};
  const size_t global_size[]{32};
  const size_t local_size[]{8};
  ur_event_handle_t event = nullptr;
  ASSERT_EQ_RESULT(
      UR_RESULT_ERROR_INVALID_NULL_HANDLE,
      urEnqueueKernelLaunch(nullptr, kernel, 1, offsets, global_size,
                            local_size, 0, nullptr, &event));
}

TEST_P(urEnqueueKernelLaunchTest, InvalidNullHandleKernel) {
  const size_t offsets[]{0, 0, 0};
  const size_t global_size[]{32};
  const size_t local_size[]{8};
  ur_event_handle_t event = nullptr;
  ASSERT_EQ_RESULT(
      UR_RESULT_ERROR_INVALID_NULL_HANDLE,
      urEnqueueKernelLaunch(queue, nullptr, 1, offsets, global_size, local_size,
                            0, nullptr, &event));
}

TEST_P(urEnqueueKernelLaunchTest, InvalidNullPointerGlobalWorkOffset) {
  const size_t global_size[]{32};
  const size_t local_size[]{8};
  ur_event_handle_t event = nullptr;
  ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER,
                   urEnqueueKernelLaunch(queue, kernel, 1, nullptr, global_size,
                                         local_size, 0, nullptr, &event));
}

TEST_P(urEnqueueKernelLaunchTest, InvalidNullPointerGlobalWorkSize) {
  const size_t offsets[]{0, 0, 0};
  const size_t local_size[]{8};
  ur_event_handle_t event = nullptr;
  ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER,
                   urEnqueueKernelLaunch(queue, kernel, 1, offsets, nullptr,
                                         local_size, 0, nullptr, &event));
}

using urEnqueueMultiKernelLaunchTest = uur::MultiKernelTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urEnqueueMultiKernelLaunchTest);
TEST_P(urEnqueueMultiKernelLaunchTest, Success) {
  const size_t offsets[]{0, 0, 0};
  const size_t global_size[]{32};
  const size_t local_size[]{8};
  ur_event_handle_t events[2] = {nullptr, nullptr};
  ASSERT_SUCCESS(urEnqueueKernelLaunch(queue, kernel1, 1, offsets, global_size,
                                       local_size, 0, nullptr, &events[0]));
  EXPECT_NE(events[0], nullptr);
  ASSERT_SUCCESS(urEnqueueKernelLaunch(queue, kernel2, 1, offsets, global_size,
                                       local_size, 0, nullptr, &events[1]));
  EXPECT_NE(events[0], nullptr);
  EXPECT_SUCCESS(urQueueFlush(queue));
  EXPECT_SUCCESS(urEventWait(2, events));
  EXPECT_SUCCESS(urEventRelease(events[0]));
  EXPECT_SUCCESS(urEventRelease(events[1]));
}

using urEnqueueKernelLaunchMultiDeviceTest = uur::MultiDeviceMemBufferQueueTest;

TEST_F(urEnqueueKernelLaunchMultiDeviceTest, KernelReadDifferentQueues) {
  // Enqueue a kernel that writes the value 42 into each element in the buffer.
  // Generated from the following OpenCL C:
  // kernel void foo(global uint *in) { in[get_global_id(0)] = 42; }
  const uint8_t source[]{
      0x03, 0x02, 0x23, 0x07, 0x00, 0x00, 0x01, 0x00, 0x0e, 0x00, 0x06, 0x00,
      0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00,
      0x04, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00, 0x05, 0x00, 0x00, 0x00,
      0x11, 0x00, 0x02, 0x00, 0x06, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00,
      0x0b, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x4f, 0x70, 0x65, 0x6e, 0x43, 0x4c, 0x2e, 0x73, 0x74, 0x64, 0x00, 0x00,
      0x0e, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
      0x0f, 0x00, 0x05, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00,
      0x66, 0x6f, 0x6f, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x00, 0x09, 0x00,
      0x11, 0x00, 0x00, 0x00, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x5f, 0x61,
      0x72, 0x67, 0x5f, 0x74, 0x79, 0x70, 0x65, 0x2e, 0x66, 0x6f, 0x6f, 0x2e,
      0x75, 0x69, 0x6e, 0x74, 0x2a, 0x2c, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00,
      0x03, 0x00, 0x00, 0x00, 0xa0, 0x86, 0x01, 0x00, 0x05, 0x00, 0x0b, 0x00,
      0x05, 0x00, 0x00, 0x00, 0x5f, 0x5f, 0x73, 0x70, 0x69, 0x72, 0x76, 0x5f,
      0x42, 0x75, 0x69, 0x6c, 0x74, 0x49, 0x6e, 0x47, 0x6c, 0x6f, 0x62, 0x61,
      0x6c, 0x49, 0x6e, 0x76, 0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x49,
      0x64, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x0b, 0x00, 0x00, 0x00,
      0x69, 0x6e, 0x00, 0x00, 0x05, 0x00, 0x04, 0x00, 0x0c, 0x00, 0x00, 0x00,
      0x65, 0x6e, 0x74, 0x72, 0x79, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x00,
      0x0e, 0x00, 0x00, 0x00, 0x63, 0x61, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00,
      0x05, 0x00, 0x05, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x61, 0x72, 0x72, 0x61,
      0x79, 0x69, 0x64, 0x78, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
      0x05, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00,
      0x47, 0x00, 0x03, 0x00, 0x05, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00,
      0x47, 0x00, 0x04, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00,
      0x05, 0x00, 0x00, 0x00, 0x47, 0x00, 0x0d, 0x00, 0x05, 0x00, 0x00, 0x00,
      0x29, 0x00, 0x00, 0x00, 0x5f, 0x5f, 0x73, 0x70, 0x69, 0x72, 0x76, 0x5f,
      0x42, 0x75, 0x69, 0x6c, 0x74, 0x49, 0x6e, 0x47, 0x6c, 0x6f, 0x62, 0x61,
      0x6c, 0x49, 0x6e, 0x76, 0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x49,
      0x64, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x15, 0x00, 0x04, 0x00,
      0x02, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x15, 0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0x00,
      0x10, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x17, 0x00, 0x04, 0x00,
      0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
      0x20, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x03, 0x00, 0x00, 0x00, 0x13, 0x00, 0x02, 0x00, 0x06, 0x00, 0x00, 0x00,
      0x20, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
      0x07, 0x00, 0x00, 0x00, 0x21, 0x00, 0x04, 0x00, 0x09, 0x00, 0x00, 0x00,
      0x06, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00,
      0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x36, 0x00, 0x05, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00,
      0x08, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x02, 0x00,
      0x0c, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x04, 0x00, 0x03, 0x00, 0x00, 0x00,
      0x0d, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x51, 0x00, 0x05, 0x00,
      0x02, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x05, 0x00, 0x08, 0x00, 0x00, 0x00,
      0x0f, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00,
      0x3e, 0x00, 0x05, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
      0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00,
      0x38, 0x00, 0x01, 0x00};
  ur_program_handle_t program = nullptr;
  EXPECT_SUCCESS(urProgramCreateWithIL(context, source, sizeof(source), nullptr,
                                       &program));
  EXPECT_SUCCESS(urProgramBuild(context, program, nullptr));

  ur_kernel_handle_t kernel = nullptr;
  EXPECT_SUCCESS(urKernelCreate(program, "foo", &kernel));

  EXPECT_SUCCESS(urKernelSetArgMemObj(kernel, 0, buffer));

  const size_t offsets[]{0, 0, 0};
  const size_t global_size[]{count};
  const size_t local_size[]{8};
  ur_event_handle_t event = nullptr;
  EXPECT_SUCCESS(urEnqueueKernelLaunch(queues[0], kernel, 1, offsets,
                                       global_size, local_size, 0, nullptr,
                                       &event));

  // queues target different devices this checks that any devices memory has
  // been synchronized.
  for (unsigned i = 1; i < queues.size(); ++i) {
    const auto queue = queues[i];
    std::vector<uint32_t> output(count, 0);
    EXPECT_SUCCESS(urEnqueueMemBufferRead(queue, buffer, true, 0, size,
                                          output.data(), 0, nullptr, nullptr));
    for (unsigned j = 0; j < count; ++j) {
      EXPECT_EQ(42, output[j])
          << "Result on queue " << i << " at index " << j << " did not match!";
    }
  }

  EXPECT_SUCCESS(urKernelRelease(kernel));
  EXPECT_SUCCESS(urProgramRelease(program));
}
