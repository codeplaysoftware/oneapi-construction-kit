// Copyright (C) Codeplay Software Limited. All Rights Reserved.

#include "Common.h"

class clGetHostTimerTest : public ucl::DeviceTest {
 protected:
  virtual void SetUp() {
    UCL_RETURN_ON_FATAL_FAILURE(DeviceTest::SetUp());
    if (!UCL::isDeviceVersionAtLeast({3, 0})) {
      GTEST_SKIP();
    }
  }
};

TEST_F(clGetHostTimerTest, NotImplemented) {
  cl_ulong host_timer_resolution{};
  ASSERT_SUCCESS(clGetPlatformInfo(platform, CL_PLATFORM_HOST_TIMER_RESOLUTION,
                                   sizeof(host_timer_resolution),
                                   &host_timer_resolution, nullptr));
  if (0 != host_timer_resolution) {
    // Since we test against other implementations that may implement this
    // but we aren't actually testing the functionality, just skip.
    GTEST_SKIP();
  }
  cl_ulong host_timestamp{};
  ASSERT_EQ_ERRCODE(CL_INVALID_OPERATION,
                    clGetHostTimer(device, &host_timestamp));
}