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

#include <array>

#include "uur/environment.h"
#include "uur/fixtures.h"

struct urKernelSetArgValueTest : uur::QueueTest {
  void SetUp() override {
    UUR_RETURN_ON_FATAL_FAILURE(uur::QueueTest::SetUp());
    // Generated from the following OpenCL C:
    //
    // __kernel void signed_ints(char c, short s, int i, long l) {}
    // __kernel void unsigned_ints(uchar c, ushort s, uint i, ulong l) {}
    // __kernel void floating_point(float f, double b) {}
    // __kernel void signed_char(char c, global char *p) {
    //  p[0] = c;
    // }
    // __kernel void vector_char4(char4 c) {}
    // __kernel void vector_char4_buffer(char4 c, global char4 *p) {
    //    p[0] = c;
    //  }
    const uint8_t source[] = {
        0x03, 0x02, 0x23, 0x07, 0x00, 0x00, 0x01, 0x00, 0x0e, 0x00, 0x06, 0x00,
        0x4d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x11, 0x00, 0x02, 0x00, 0x06, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00, 0x0b, 0x00, 0x00, 0x00,
        0x11, 0x00, 0x02, 0x00, 0x16, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00,
        0x27, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x4f, 0x70, 0x65, 0x6e, 0x43, 0x4c, 0x2e, 0x73, 0x74, 0x64, 0x00, 0x00,
        0x0e, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x0f, 0x00, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00,
        0x73, 0x69, 0x67, 0x6e, 0x65, 0x64, 0x5f, 0x69, 0x6e, 0x74, 0x73, 0x00,
        0x0f, 0x00, 0x07, 0x00, 0x06, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00,
        0x75, 0x6e, 0x73, 0x69, 0x67, 0x6e, 0x65, 0x64, 0x5f, 0x69, 0x6e, 0x74,
        0x73, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x07, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x3a, 0x00, 0x00, 0x00, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x69, 0x6e, 0x67,
        0x5f, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x00, 0x00, 0x0f, 0x00, 0x06, 0x00,
        0x06, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x73, 0x69, 0x67, 0x6e,
        0x65, 0x64, 0x5f, 0x63, 0x68, 0x61, 0x72, 0x00, 0x0f, 0x00, 0x07, 0x00,
        0x06, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x76, 0x65, 0x63, 0x74,
        0x6f, 0x72, 0x5f, 0x63, 0x68, 0x61, 0x72, 0x34, 0x00, 0x00, 0x00, 0x00,
        0x0f, 0x00, 0x08, 0x00, 0x06, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00,
        0x76, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x63, 0x68, 0x61, 0x72, 0x34,
        0x5f, 0x62, 0x75, 0x66, 0x66, 0x65, 0x72, 0x00, 0x03, 0x00, 0x03, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x70, 0x8e, 0x01, 0x00, 0x05, 0x00, 0x05, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x73, 0x69, 0x67, 0x6e, 0x65, 0x64, 0x5f, 0x69,
        0x6e, 0x74, 0x73, 0x00, 0x05, 0x00, 0x03, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x63, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x0a, 0x00, 0x00, 0x00,
        0x73, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x0b, 0x00, 0x00, 0x00,
        0x69, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x00, 0x00,
        0x6c, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x00, 0x0d, 0x00, 0x00, 0x00,
        0x65, 0x6e, 0x74, 0x72, 0x79, 0x00, 0x00, 0x00, 0x05, 0x00, 0x06, 0x00,
        0x0e, 0x00, 0x00, 0x00, 0x75, 0x6e, 0x73, 0x69, 0x67, 0x6e, 0x65, 0x64,
        0x5f, 0x69, 0x6e, 0x74, 0x73, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00,
        0x0f, 0x00, 0x00, 0x00, 0x63, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00,
        0x11, 0x00, 0x00, 0x00, 0x69, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00,
        0x12, 0x00, 0x00, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x00,
        0x13, 0x00, 0x00, 0x00, 0x65, 0x6e, 0x74, 0x72, 0x79, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x06, 0x00, 0x17, 0x00, 0x00, 0x00, 0x66, 0x6c, 0x6f, 0x61,
        0x74, 0x69, 0x6e, 0x67, 0x5f, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x00, 0x00,
        0x05, 0x00, 0x03, 0x00, 0x18, 0x00, 0x00, 0x00, 0x66, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x03, 0x00, 0x19, 0x00, 0x00, 0x00, 0x62, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x04, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x65, 0x6e, 0x74, 0x72,
        0x79, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00, 0x1d, 0x00, 0x00, 0x00,
        0x73, 0x69, 0x67, 0x6e, 0x65, 0x64, 0x5f, 0x63, 0x68, 0x61, 0x72, 0x00,
        0x05, 0x00, 0x03, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x63, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x03, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00, 0x65, 0x6e, 0x74, 0x72,
        0x79, 0x00, 0x00, 0x00, 0x05, 0x00, 0x06, 0x00, 0x23, 0x00, 0x00, 0x00,
        0x76, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x63, 0x68, 0x61, 0x72, 0x34,
        0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x24, 0x00, 0x00, 0x00,
        0x63, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x00, 0x25, 0x00, 0x00, 0x00,
        0x65, 0x6e, 0x74, 0x72, 0x79, 0x00, 0x00, 0x00, 0x05, 0x00, 0x07, 0x00,
        0x28, 0x00, 0x00, 0x00, 0x76, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x63,
        0x68, 0x61, 0x72, 0x34, 0x5f, 0x62, 0x75, 0x66, 0x66, 0x65, 0x72, 0x00,
        0x05, 0x00, 0x03, 0x00, 0x29, 0x00, 0x00, 0x00, 0x63, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x03, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x04, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x65, 0x6e, 0x74, 0x72,
        0x79, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x2d, 0x00, 0x00, 0x00,
        0x63, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x2e, 0x00, 0x00, 0x00,
        0x73, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x2f, 0x00, 0x00, 0x00,
        0x69, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x30, 0x00, 0x00, 0x00,
        0x6c, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x34, 0x00, 0x00, 0x00,
        0x63, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x35, 0x00, 0x00, 0x00,
        0x73, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x69, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x37, 0x00, 0x00, 0x00,
        0x6c, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x3b, 0x00, 0x00, 0x00,
        0x66, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x3c, 0x00, 0x00, 0x00,
        0x62, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x40, 0x00, 0x00, 0x00,
        0x63, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x41, 0x00, 0x00, 0x00,
        0x70, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x45, 0x00, 0x00, 0x00,
        0x63, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x49, 0x00, 0x00, 0x00,
        0x63, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x4a, 0x00, 0x00, 0x00,
        0x70, 0x00, 0x00, 0x00, 0x47, 0x00, 0x07, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x29, 0x00, 0x00, 0x00, 0x73, 0x69, 0x67, 0x6e, 0x65, 0x64, 0x5f, 0x69,
        0x6e, 0x74, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x47, 0x00, 0x04, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x47, 0x00, 0x08, 0x00, 0x0e, 0x00, 0x00, 0x00,
        0x29, 0x00, 0x00, 0x00, 0x75, 0x6e, 0x73, 0x69, 0x67, 0x6e, 0x65, 0x64,
        0x5f, 0x69, 0x6e, 0x74, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x47, 0x00, 0x04, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x08, 0x00,
        0x17, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x66, 0x6c, 0x6f, 0x61,
        0x74, 0x69, 0x6e, 0x67, 0x5f, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x07, 0x00, 0x1d, 0x00, 0x00, 0x00,
        0x29, 0x00, 0x00, 0x00, 0x73, 0x69, 0x67, 0x6e, 0x65, 0x64, 0x5f, 0x63,
        0x68, 0x61, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
        0x1e, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x47, 0x00, 0x04, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x47, 0x00, 0x08, 0x00, 0x23, 0x00, 0x00, 0x00,
        0x29, 0x00, 0x00, 0x00, 0x76, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x63,
        0x68, 0x61, 0x72, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x47, 0x00, 0x09, 0x00, 0x28, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00,
        0x76, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x63, 0x68, 0x61, 0x72, 0x34,
        0x5f, 0x62, 0x75, 0x66, 0x66, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x47, 0x00, 0x04, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x2d, 0x00, 0x00, 0x00,
        0x26, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
        0x2e, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x47, 0x00, 0x04, 0x00, 0x34, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x35, 0x00, 0x00, 0x00,
        0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
        0x40, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x47, 0x00, 0x04, 0x00, 0x41, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x4a, 0x00, 0x00, 0x00,
        0x26, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x15, 0x00, 0x04, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x15, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x04, 0x00,
        0x06, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x13, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x21, 0x00, 0x07, 0x00,
        0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x16, 0x00, 0x03, 0x00, 0x14, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
        0x16, 0x00, 0x03, 0x00, 0x15, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
        0x21, 0x00, 0x05, 0x00, 0x16, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x14, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00,
        0x1b, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x21, 0x00, 0x05, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x17, 0x00, 0x04, 0x00,
        0x21, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x21, 0x00, 0x04, 0x00, 0x22, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x21, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x26, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x21, 0x00, 0x05, 0x00,
        0x27, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00,
        0x26, 0x00, 0x00, 0x00, 0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x05, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
        0xf8, 0x00, 0x02, 0x00, 0x0d, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00,
        0x38, 0x00, 0x01, 0x00, 0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x0e, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x05, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00,
        0xf8, 0x00, 0x02, 0x00, 0x13, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00,
        0x38, 0x00, 0x01, 0x00, 0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x17, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x14, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x15, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00,
        0xf8, 0x00, 0x02, 0x00, 0x1a, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00,
        0x38, 0x00, 0x01, 0x00, 0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x1d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00,
        0xf8, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x05, 0x00,
        0x1f, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00,
        0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00,
        0x21, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x02, 0x00,
        0x25, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00,
        0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00,
        0x21, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00,
        0x26, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x02, 0x00,
        0x2b, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x05, 0x00, 0x2a, 0x00, 0x00, 0x00,
        0x29, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0xfd, 0x00, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00, 0x36, 0x00, 0x05, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x07, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x2d, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x2e, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x2f, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x30, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x02, 0x00, 0x31, 0x00, 0x00, 0x00,
        0x39, 0x00, 0x08, 0x00, 0x02, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x00, 0x00, 0x2e, 0x00, 0x00, 0x00,
        0x2f, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00,
        0x38, 0x00, 0x01, 0x00, 0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x33, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x05, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x00,
        0xf8, 0x00, 0x02, 0x00, 0x38, 0x00, 0x00, 0x00, 0x39, 0x00, 0x08, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x39, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00,
        0x34, 0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x37, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00,
        0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00, 0x3a, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00,
        0x14, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00,
        0x15, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x02, 0x00,
        0x3d, 0x00, 0x00, 0x00, 0x39, 0x00, 0x06, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x3e, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x00, 0x00,
        0x3c, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00,
        0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00,
        0x1b, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x02, 0x00,
        0x42, 0x00, 0x00, 0x00, 0x39, 0x00, 0x06, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x43, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
        0x41, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00,
        0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00,
        0x21, 0x00, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x02, 0x00,
        0x46, 0x00, 0x00, 0x00, 0x39, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x47, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00,
        0xfd, 0x00, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00, 0x36, 0x00, 0x05, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x27, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00, 0x21, 0x00, 0x00, 0x00,
        0x49, 0x00, 0x00, 0x00, 0x37, 0x00, 0x03, 0x00, 0x26, 0x00, 0x00, 0x00,
        0x4a, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x02, 0x00, 0x4b, 0x00, 0x00, 0x00,
        0x39, 0x00, 0x06, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4c, 0x00, 0x00, 0x00,
        0x28, 0x00, 0x00, 0x00, 0x49, 0x00, 0x00, 0x00, 0x4a, 0x00, 0x00, 0x00,
        0xfd, 0x00, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00};
    const size_t source_size = sizeof(source) / sizeof(source[0]);
    const char *kernel_names[] = {"signed_ints",    "unsigned_ints",
                                  "floating_point", "signed_char",
                                  "vector_char4",   "vector_char4_buffer"};

    ASSERT_SUCCESS(
        urProgramCreateWithIL(context, source, source_size, nullptr, &program));
    ASSERT_NE(nullptr, program);
    ASSERT_SUCCESS(urProgramBuild(context, program, nullptr));

    for (size_t i = 0; i < kernels.size(); i++) {
      ASSERT_SUCCESS(urKernelCreate(program, kernel_names[i], &kernels[i]));
      ASSERT_NE(nullptr, kernels[i]);
    }

    ASSERT_SUCCESS(urMemBufferCreate(context, UR_MEM_FLAG_READ_WRITE, size,
                                     nullptr, &buffer));
    ASSERT_NE(nullptr, buffer);
  }

  void TearDown() override {
    if (buffer) {
      EXPECT_SUCCESS(urMemRelease(buffer));
    }

    for (auto &kernel : kernels) {
      if (kernel) {
        EXPECT_SUCCESS(urKernelRelease(kernel));
      }
    }

    if (program) {
      EXPECT_SUCCESS(urProgramRelease(program));
    }
    uur::ContextTest::TearDown();
  }

  ur_program_handle_t program = nullptr;
  std::array<ur_kernel_handle_t, 6> kernels;
  ur_mem_handle_t buffer;

  const size_t count = 4;
  const size_t size = count * sizeof(char);
};
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urKernelSetArgValueTest);

TEST_P(urKernelSetArgValueTest, SuccessSingleArg) {
  {
    char c = 1;
    short s = 2;
    int i = 3;
    long l = 4;
    ASSERT_SUCCESS(urKernelSetArgValue(kernels[0], 0, sizeof(c), &c));
    ASSERT_SUCCESS(urKernelSetArgValue(kernels[0], 1, sizeof(s), &s));
    ASSERT_SUCCESS(urKernelSetArgValue(kernels[0], 2, sizeof(i), &i));
    ASSERT_SUCCESS(urKernelSetArgValue(kernels[0], 3, sizeof(l), &l));
  }

  {
    unsigned char c = 1;
    unsigned short s = 2;
    unsigned int i = 3;
    unsigned long l = 4;
    ASSERT_SUCCESS(urKernelSetArgValue(kernels[1], 0, sizeof(c), &c));
    ASSERT_SUCCESS(urKernelSetArgValue(kernels[1], 1, sizeof(s), &s));
    ASSERT_SUCCESS(urKernelSetArgValue(kernels[1], 2, sizeof(i), &i));
    ASSERT_SUCCESS(urKernelSetArgValue(kernels[1], 3, sizeof(l), &l));
  }

  {
    float f = 0.0;
    double b = 1.0;
    ASSERT_SUCCESS(urKernelSetArgValue(kernels[2], 0, sizeof(f), &f));
    ASSERT_SUCCESS(urKernelSetArgValue(kernels[2], 1, sizeof(b), &b));
  }

  {
    char c = 42;
    ASSERT_SUCCESS(urKernelSetArgValue(kernels[3], 0, sizeof(c), &c));
    EXPECT_SUCCESS(urKernelSetArgMemObj(kernels[3], 1, buffer));
  }

  {
    char c4[4] = {0, 1, 2, 3};
    ASSERT_SUCCESS(urKernelSetArgValue(kernels[4], 0, sizeof(c4), &c4));
  }
}

TEST_P(urKernelSetArgValueTest, StoreArgValueInBufferTest) {
  char c = 42;
  ASSERT_SUCCESS(urKernelSetArgValue(kernels[3], 0, sizeof(c), &c));

  EXPECT_SUCCESS(urKernelSetArgMemObj(kernels[3], 1, buffer));

  const size_t offsets[]{0, 0, 0};
  const size_t global_size[]{count};
  const size_t local_size[]{4};
  ur_event_handle_t event = nullptr;
  EXPECT_SUCCESS(urEnqueueKernelLaunch(queue, kernels[3], 1, offsets,
                                       global_size, local_size, 0, nullptr,
                                       &event));

  char p = 0;
  EXPECT_SUCCESS(urEnqueueMemBufferRead(queue, buffer, true, 0, sizeof(p), &p,
                                        0, nullptr, nullptr));

  EXPECT_EQ(c, p);
}

TEST_P(urKernelSetArgValueTest, StoreVectorArgValueInBufferTest) {
  char c4[4] = {1, 2, 3, 4};
  ASSERT_SUCCESS(urKernelSetArgValue(kernels[5], 0, sizeof(c4), &c4));

  EXPECT_SUCCESS(urKernelSetArgMemObj(kernels[5], 1, buffer));

  const size_t offsets[]{0, 0, 0};
  const size_t global_size[]{count};
  const size_t local_size[]{4};
  ur_event_handle_t event = nullptr;
  EXPECT_SUCCESS(urEnqueueKernelLaunch(queue, kernels[5], 1, offsets,
                                       global_size, local_size, 0, nullptr,
                                       &event));

  char p4[4] = {0, 0, 0, 0};
  EXPECT_SUCCESS(urEnqueueMemBufferRead(queue, buffer, true, 0, 4, &p4, 0,
                                        nullptr, nullptr));
  for (int i = 0; i < 4; i++) {
    EXPECT_EQ(c4[i], p4[i]);
  }
}

TEST_P(urKernelSetArgValueTest, InvalidNullHandleKernel) {
  ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE,
                   urKernelSetArgValue(nullptr, 0, 0, nullptr));
}

TEST_P(urKernelSetArgValueTest, InvalidNullPointer) {
  ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER,
                   urKernelSetArgValue(kernels[0], 0, 0, nullptr));
}

TEST_P(urKernelSetArgValueTest, InvalidKernelArgumentIndex) {
  int i = 0;

  ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX,
                   urKernelSetArgValue(kernels[0], UINT32_MAX, sizeof(i), &i));
}

TEST_P(urKernelSetArgValueTest, InvalidKernelArgumentSize) {
  float f = 0;
  ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE,
                   urKernelSetArgValue(kernels[0], 0, sizeof(f), &f));
}
