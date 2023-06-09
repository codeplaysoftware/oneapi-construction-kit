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

using urEnqueueMemBufferMapTest = uur::MemBufferQueueTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urEnqueueMemBufferMapTest);

TEST_P(urEnqueueMemBufferMapTest, SuccessRead) {
  const std::vector<uint32_t> input(count, 42);
  ASSERT_SUCCESS(urEnqueueMemBufferWrite(queue, buffer, true, 0, size,
                                         input.data(), 0, nullptr, nullptr));

  uint32_t *map = nullptr;
  ASSERT_SUCCESS(urEnqueueMemBufferMap(queue, buffer, true, UR_MAP_FLAG_READ, 0,
                                       size, 0, nullptr, nullptr,
                                       (void **)&map));
  for (unsigned i = 0; i < count; ++i) {
    ASSERT_EQ(map[i], 42) << "Result mismatch at index: " << i;
  }
}

TEST_P(urEnqueueMemBufferMapTest, SuccessWrite) {
  const std::vector<uint32_t> input(count, 0);
  ASSERT_SUCCESS(urEnqueueMemBufferWrite(queue, buffer, true, 0, size,
                                         input.data(), 0, nullptr, nullptr));

  uint32_t *map = nullptr;
  ASSERT_SUCCESS(urEnqueueMemBufferMap(queue, buffer, true, UR_MAP_FLAG_WRITE,
                                       0, size, 0, nullptr, nullptr,
                                       (void **)&map));
  for (unsigned i = 0; i < count; ++i) {
    map[i] = 42;
  }
  ASSERT_SUCCESS(urEnqueueMemUnmap(queue, buffer, map, 0, nullptr, nullptr));
  std::vector<uint32_t> output(count, 1);
  ASSERT_SUCCESS(urEnqueueMemBufferRead(queue, buffer, true, 0, size,
                                        output.data(), 0, nullptr, nullptr));
  for (unsigned i = 0; i < count; ++i) {
    ASSERT_EQ(output[i], 42) << "Result mismatch at index: " << i;
  }
}

TEST_P(urEnqueueMemBufferMapTest, SuccessOffset) {
  const std::vector<uint32_t> input(count, 0);
  ASSERT_SUCCESS(urEnqueueMemBufferWrite(queue, buffer, true, 0, size,
                                         input.data(), 0, nullptr, nullptr));

  uint32_t *map = nullptr;
  const size_t offset_size = size / 2;
  ASSERT_SUCCESS(urEnqueueMemBufferMap(queue, buffer, true, UR_MAP_FLAG_WRITE,
                                       offset_size, size - offset_size, 0,
                                       nullptr, nullptr, (void **)&map));

  const size_t offset_count = count / 2;
  for (unsigned i = 0; i < offset_count; ++i) {
    map[i] = 42;
  }

  ASSERT_SUCCESS(urEnqueueMemUnmap(queue, buffer, map, 0, nullptr, nullptr));
  std::vector<uint32_t> output(count, 1);
  ASSERT_SUCCESS(urEnqueueMemBufferRead(queue, buffer, true, 0, size,
                                        output.data(), 0, nullptr, nullptr));

  for (unsigned i = 0; i < offset_count; ++i) {
    ASSERT_EQ(output[i], 0) << "Result mismatch at index: " << i;
  }
  for (unsigned i = offset_count; i < count; ++i) {
    ASSERT_EQ(output[i], 42) << "Result mismatch at index: " << i;
  }
}

TEST_P(urEnqueueMemBufferMapTest, SuccessPartialMap) {
  const std::vector<uint32_t> input(count, 0);
  ASSERT_SUCCESS(urEnqueueMemBufferWrite(queue, buffer, true, 0, size,
                                         input.data(), 0, nullptr, nullptr));
  uint32_t *map = nullptr;
  const size_t map_size = size / 2;
  ASSERT_SUCCESS(urEnqueueMemBufferMap(queue, buffer, true, UR_MAP_FLAG_WRITE,
                                       0, map_size, 0, nullptr, nullptr,
                                       (void **)&map));

  const size_t offset_count = count / 2;
  for (unsigned i = 0; i < offset_count; ++i) {
    map[i] = 42;
  }

  ASSERT_SUCCESS(urEnqueueMemUnmap(queue, buffer, map, 0, nullptr, nullptr));
  std::vector<uint32_t> output(count, 1);
  ASSERT_SUCCESS(urEnqueueMemBufferRead(queue, buffer, true, 0, size,
                                        output.data(), 0, nullptr, nullptr));

  for (unsigned i = 0; i < offset_count; ++i) {
    ASSERT_EQ(output[i], 42) << "Result mismatch at index: " << i;
  }
  for (unsigned i = offset_count; i < count; ++i) {
    ASSERT_EQ(output[i], 0) << "Result mismatch at index: " << i;
  }
}

TEST_P(urEnqueueMemBufferMapTest, SuccessMultiMaps) {
  const std::vector<uint32_t> input(count, 0);
  ASSERT_SUCCESS(urEnqueueMemBufferWrite(queue, buffer, true, 0, size,
                                         input.data(), 0, nullptr, nullptr));

  // Create two maps with non-overlapping ranges and write separate values into
  // each of them to check we can maintain multiple maps on the same buffer.
  uint32_t *map_a = nullptr;
  uint32_t *map_b = nullptr;
  const auto map_size = size / 2;
  const auto map_offset = size / 2;
  const auto map_count = count / 2;

  ASSERT_SUCCESS(urEnqueueMemBufferMap(queue, buffer, true, UR_MAP_FLAG_WRITE,
                                       0, map_size, 0, nullptr, nullptr,
                                       (void **)&map_a));
  ASSERT_SUCCESS(urEnqueueMemBufferMap(queue, buffer, true, UR_MAP_FLAG_WRITE,
                                       map_offset, map_size, 0, nullptr,
                                       nullptr, (void **)&map_b));
  for (unsigned i = 0; i < map_count; ++i) {
    map_a[i] = 42;
  }
  for (unsigned i = map_count; i < count; ++i) {
    map_a[i] = 24;
  }
  ASSERT_SUCCESS(urEnqueueMemUnmap(queue, buffer, map_a, 0, nullptr, nullptr));
  ASSERT_SUCCESS(urEnqueueMemUnmap(queue, buffer, map_b, 0, nullptr, nullptr));
  std::vector<uint32_t> output(count, 1);
  ASSERT_SUCCESS(urEnqueueMemBufferRead(queue, buffer, true, 0, size,
                                        output.data(), 0, nullptr, nullptr));
  for (unsigned i = 0; i < map_count; ++i) {
    ASSERT_EQ(output[i], 42) << "Result mismatch at index: " << i;
  }
  for (unsigned i = map_count; i < count; ++i) {
    ASSERT_EQ(output[i], 24) << "Result mismatch at index: " << i;
  }
}

TEST_P(urEnqueueMemBufferMapTest, InvalidNullHandleQueue) {
  uint32_t *map = nullptr;
  ASSERT_EQ_RESULT(
      UR_RESULT_ERROR_INVALID_NULL_HANDLE,
      urEnqueueMemBufferMap(nullptr, buffer, true,
                            UR_MAP_FLAG_READ | UR_MAP_FLAG_WRITE, 0, size, 0,
                            nullptr, nullptr, (void **)&map));
}

TEST_P(urEnqueueMemBufferMapTest, InvalidNullHandleBuffer) {
  uint32_t *map = nullptr;
  ASSERT_EQ_RESULT(
      UR_RESULT_ERROR_INVALID_NULL_HANDLE,
      urEnqueueMemBufferMap(queue, nullptr, true,
                            UR_MAP_FLAG_READ | UR_MAP_FLAG_WRITE, 0, size, 0,
                            nullptr, nullptr, (void **)&map));
}

TEST_P(urEnqueueMemBufferMapTest, InvalidEnumerationMapFlags) {
  uint32_t *map = nullptr;
  ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_ENUMERATION,
                   urEnqueueMemBufferMap(queue, buffer, true, 0x4, 0, size, 0,
                                         nullptr, nullptr, (void **)&map));
}

TEST_P(urEnqueueMemBufferMapTest, InvalidNullPointerRetMap) {
  ASSERT_EQ_RESULT(
      UR_RESULT_ERROR_INVALID_NULL_POINTER,
      urEnqueueMemBufferMap(queue, buffer, true,
                            UR_MAP_FLAG_READ | UR_MAP_FLAG_WRITE, 0, size, 0,
                            nullptr, nullptr, nullptr));
}

using urEnqueueMemBufferMapMultiDeviceTest = uur::MultiDeviceMemBufferQueueTest;

TEST_F(urEnqueueMemBufferMapMultiDeviceTest, WriteMapDifferentQueues) {
  // First queue does a blocking write of 42 into the buffer.
  std::vector<uint32_t> input(count, 42);
  ASSERT_SUCCESS(urEnqueueMemBufferWrite(queues[0], buffer, true, 0, size,
                                         input.data(), 0, nullptr, nullptr));

  // Then the remaining queues map the buffer into suome host memory. Since the
  // queues target different devices this checks that any devices memory has
  // been synchronized.
  for (unsigned i = 1; i < queues.size(); ++i) {
    const auto queue = queues[i];
    uint32_t *map = nullptr;
    ASSERT_SUCCESS(urEnqueueMemBufferMap(queue, buffer, true, UR_MAP_FLAG_READ,
                                         0, size, 0, nullptr, nullptr,
                                         (void **)&map));
    for (unsigned j = 0; j < count; ++j) {
      ASSERT_EQ(input[j], map[j])
          << "Result on queue " << i << " at index " << j << " did not match!";
    }
    ASSERT_SUCCESS(urEnqueueMemUnmap(queue, buffer, map, 0, nullptr, nullptr));
  }
}
