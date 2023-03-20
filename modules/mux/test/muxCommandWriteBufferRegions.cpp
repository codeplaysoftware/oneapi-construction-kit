// Copyright (C) Codeplay Software Limited. All Rights Reserved.

#include <mux/utils/helpers.h>

#include "common.h"

enum { MEMORY_SIZE = 128 };

struct muxCommandWriteBufferRegionsTest : DeviceTest {
  mux_memory_t memory = nullptr;
  mux_buffer_t buffer = nullptr;
  mux_command_buffer_t command_buffer = nullptr;

  void SetUp() override {
    RETURN_ON_FATAL_FAILURE(DeviceTest::SetUp());
    ASSERT_SUCCESS(muxCreateBuffer(device, MEMORY_SIZE, allocator, &buffer));

    const mux_allocation_type_e allocation_type =
        (mux_allocation_capabilities_alloc_device &
         device->info->allocation_capabilities)
            ? mux_allocation_type_alloc_device
            : mux_allocation_type_alloc_host;
    uint32_t heap = mux::findFirstSupportedHeap(
        buffer->memory_requirements.supported_heaps);

    ASSERT_SUCCESS(muxAllocateMemory(device, MEMORY_SIZE, heap,
                                     mux_memory_property_host_visible,
                                     allocation_type, 0, allocator, &memory));

    ASSERT_SUCCESS(muxBindBufferMemory(device, memory, buffer, 0));

    ASSERT_SUCCESS(
        muxCreateCommandBuffer(device, callback, allocator, &command_buffer));
  }

  void TearDown() override {
    if (command_buffer) {
      muxDestroyCommandBuffer(device, command_buffer, allocator);
    }
    if (buffer) {
      muxDestroyBuffer(device, buffer, allocator);
    }
    if (memory) {
      muxFreeMemory(device, memory, allocator);
    }
    DeviceTest::TearDown();
  }
};

INSTANTIATE_DEVICE_TEST_SUITE_P(muxCommandWriteBufferRegionsTest);

TEST_P(muxCommandWriteBufferRegionsTest, Default) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {
      {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1}, {1, 1}};

  ASSERT_SUCCESS(muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                              &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, MultipleRegions) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info[4] = {
      {{1, 1, 1}, {0, 0, 0}, {0, 0, 0}, {MEMORY_SIZE, 1}, {MEMORY_SIZE, 1}},
      {{1, 1, 1}, {1, 0, 0}, {2, 0, 0}, {MEMORY_SIZE, 1}, {MEMORY_SIZE, 1}},
      {{1, 1, 1}, {2, 0, 0}, {4, 0, 0}, {MEMORY_SIZE, 1}, {MEMORY_SIZE, 1}},
      {{1, 1, 1}, {3, 0, 0}, {8, 0, 0}, {MEMORY_SIZE, 1}, {MEMORY_SIZE, 1}},
  };

  ASSERT_SUCCESS(muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                              info, 4, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, InvalidHostPointer) {
  mux_buffer_region_info_t info = {
      {1, 1, 1}, {0, 0, 0}, {0, 0, 0}, {MEMORY_SIZE, 1}, {MEMORY_SIZE, 1}};

  ASSERT_ERROR_EQ(mux_error_invalid_value,
                  muxCommandWriteBufferRegions(command_buffer, buffer, nullptr,
                                               &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, ZeroSizeRegion) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {
      {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0}, {0, 0}};

  ASSERT_ERROR_EQ(mux_error_invalid_value,
                  muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                               &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, ValidSizeRegionX) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {{MEMORY_SIZE, 1, 1},
                                   {0, 0, 0},
                                   {0, 0, 0},
                                   {MEMORY_SIZE, 1},
                                   {MEMORY_SIZE, 1}};

  ASSERT_SUCCESS(muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                              &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, ValidSizeRegionY) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {{1, MEMORY_SIZE, 1},
                                   {0, 0, 0},
                                   {0, 0, 0},
                                   {1, MEMORY_SIZE},
                                   {1, MEMORY_SIZE}};

  ASSERT_SUCCESS(muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                              &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, ValidSizeRegionZ) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {
      {1, 1, MEMORY_SIZE}, {0, 0, 0}, {0, 0, 0}, {1, 1}, {1, 1}};

  ASSERT_SUCCESS(muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                              &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, InvalidSizeRegionX) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {{MEMORY_SIZE + 1, 1, 1},
                                   {0, 0, 0},
                                   {0, 0, 0},
                                   {MEMORY_SIZE, 1},
                                   {MEMORY_SIZE, 1}};

  ASSERT_ERROR_EQ(mux_error_invalid_value,
                  muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                               &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, InvalidSizeRegionY) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {{1, MEMORY_SIZE + 1, 1},
                                   {0, 0, 0},
                                   {0, 0, 0},
                                   {MEMORY_SIZE, 1},
                                   {MEMORY_SIZE, 1}};

  ASSERT_ERROR_EQ(mux_error_invalid_value,
                  muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                               &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, InvalidSizeRegionZ) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {{1, 1, MEMORY_SIZE + 1},
                                   {0, 0, 0},
                                   {0, 0, 0},
                                   {MEMORY_SIZE, 1},
                                   {MEMORY_SIZE, 1}};

  ASSERT_ERROR_EQ(mux_error_invalid_value,
                  muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                               &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, InvalidSizeSrcOriginX) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {{1, 1, 1},
                                   {MEMORY_SIZE + 1, 0, 0},
                                   {0, 0, 0},
                                   {MEMORY_SIZE, 1},
                                   {MEMORY_SIZE, 1}};

  ASSERT_ERROR_EQ(mux_error_invalid_value,
                  muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                               &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, InvalidSizeSrcOriginY) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {{1, 1, 1},
                                   {0, MEMORY_SIZE + 1, 0},
                                   {0, 0, 0},
                                   {MEMORY_SIZE, 1},
                                   {MEMORY_SIZE, 1}};

  ASSERT_ERROR_EQ(mux_error_invalid_value,
                  muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                               &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, InvalidSizeSrcOriginZ) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {{1, 1, 1},
                                   {0, 0, MEMORY_SIZE + 1},
                                   {0, 0, 0},
                                   {MEMORY_SIZE, 1},
                                   {MEMORY_SIZE, 1}};

  ASSERT_ERROR_EQ(mux_error_invalid_value,
                  muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                               &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, OverlappingSrcRegions) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info[2] = {{{2, 1, 1},
                                       {0, 0, 0},
                                       {0, 0, 0},
                                       {MEMORY_SIZE / 4, MEMORY_SIZE},
                                       {MEMORY_SIZE / 4, MEMORY_SIZE}},
                                      {{2, 1, 1},
                                       {1, 0, 0},
                                       {4, 0, 0},
                                       {MEMORY_SIZE / 4, MEMORY_SIZE},
                                       {MEMORY_SIZE / 4, MEMORY_SIZE}}};

  ASSERT_SUCCESS(muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                              info, 2, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, OverlappingDstRegionsX) {
  char data[MEMORY_SIZE] = {0};

  {
    mux_buffer_region_info_t info[2] = {
        {{4, 1, 1}, {0, 0, 0}, {2, 0, 0}, {MEMORY_SIZE, 1}, {MEMORY_SIZE, 1}},
        {{4, 1, 1}, {0, 0, 0}, {0, 0, 0}, {MEMORY_SIZE, 1}, {MEMORY_SIZE, 1}}};

    ASSERT_ERROR_EQ(mux_error_invalid_value,
                    muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                                 info, 2, 0, nullptr, nullptr));
  }

  {
    mux_buffer_region_info_t info[2] = {
        {{4, 1, 1}, {0, 0, 0}, {0, 0, 0}, {MEMORY_SIZE, 1}, {MEMORY_SIZE, 1}},
        {{4, 1, 1}, {0, 0, 0}, {2, 0, 0}, {MEMORY_SIZE, 1}, {MEMORY_SIZE, 1}}};

    ASSERT_ERROR_EQ(mux_error_invalid_value,
                    muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                                 info, 2, 0, nullptr, nullptr));
  }
}

TEST_P(muxCommandWriteBufferRegionsTest, OverlappingDstRegionsY) {
  char data[MEMORY_SIZE] = {0};

  {
    mux_buffer_region_info_t info[2] = {
        {{1, 2, 1}, {0, 0, 0}, {0, 1, 0}, {1, MEMORY_SIZE}, {1, MEMORY_SIZE}},
        {{1, 2, 1}, {0, 0, 0}, {0, 0, 0}, {1, MEMORY_SIZE}, {1, MEMORY_SIZE}}};

    ASSERT_ERROR_EQ(mux_error_invalid_value,
                    muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                                 info, 2, 0, nullptr, nullptr));
  }

  {
    mux_buffer_region_info_t info[2] = {
        {{1, 2, 1}, {0, 0, 0}, {0, 0, 0}, {1, MEMORY_SIZE}, {1, MEMORY_SIZE}},
        {{1, 2, 1}, {0, 0, 0}, {0, 1, 0}, {1, MEMORY_SIZE}, {1, MEMORY_SIZE}}};

    ASSERT_ERROR_EQ(mux_error_invalid_value,
                    muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                                 info, 2, 0, nullptr, nullptr));
  }
}

TEST_P(muxCommandWriteBufferRegionsTest, OverlappingDstRegionsZ) {
  char data[MEMORY_SIZE] = {0};

  {
    mux_buffer_region_info_t info[2] = {
        {{1, 1, 6}, {0, 0, 0}, {0, 0, 3}, {1, 1}, {1, 1}},
        {{1, 1, 6}, {0, 0, 0}, {0, 0, 0}, {1, 1}, {1, 1}}};

    ASSERT_ERROR_EQ(mux_error_invalid_value,
                    muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                                 info, 2, 0, nullptr, nullptr));
  }

  {
    mux_buffer_region_info_t info[2] = {
        {{1, 1, 6}, {0, 0, 0}, {0, 0, 0}, {1, 1}, {1, 1}},
        {{1, 1, 6}, {0, 0, 0}, {0, 0, 3}, {1, 1}, {1, 1}}};

    ASSERT_ERROR_EQ(mux_error_invalid_value,
                    muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                                 info, 2, 0, nullptr, nullptr));
  }
}

TEST_P(muxCommandWriteBufferRegionsTest, InvalidSrcOriginX) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {{1, 1, 1},
                                   {MEMORY_SIZE, 0, 0},
                                   {0, 0, 0},
                                   {MEMORY_SIZE, 1},
                                   {MEMORY_SIZE, 1}};

  ASSERT_ERROR_EQ(mux_error_invalid_value,
                  muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                               &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, InvalidSrcOriginY) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {{1, 1, 1},
                                   {0, MEMORY_SIZE, 0},
                                   {0, 0, 0},
                                   {MEMORY_SIZE, 1},
                                   {MEMORY_SIZE, 1}};

  ASSERT_ERROR_EQ(mux_error_invalid_value,
                  muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                               &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, InvalidSrcOriginZ) {
  char data[MEMORY_SIZE] = {0};

  mux_buffer_region_info_t info = {{1, 1, 1},
                                   {0, 0, MEMORY_SIZE},
                                   {0, 0, 0},
                                   {MEMORY_SIZE, 1},
                                   {MEMORY_SIZE, 1}};

  ASSERT_ERROR_EQ(mux_error_invalid_value,
                  muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                               &info, 1, 0, nullptr, nullptr));
}

TEST_P(muxCommandWriteBufferRegionsTest, Sync) {
  char data[MEMORY_SIZE] = {0};
  mux_buffer_region_info_t info = {
      {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1}, {1, 1}};

  mux_sync_point_t wait = nullptr;
  ASSERT_SUCCESS(muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                              &info, 1, 0, nullptr, &wait));
  ASSERT_NE(wait, nullptr);

  ASSERT_SUCCESS(muxCommandWriteBufferRegions(command_buffer, buffer, data,
                                              &info, 1, 1, &wait, nullptr));
}