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

// REQUIRES: images
// RUN: oclc -execute -enqueue image1CopyInc -arg img,repeat(256,randint(0,254)) -show dst,64 -global 64,1,1 -local 16,1,1 %s  > %t
// RUN: FileCheck < %t %s

__kernel void image1CopyInc(__read_only image1d_t img, __write_only image1d_t dst) {
	size_t gid0 = get_global_id(0);
	size_t width = (size_t)get_image_width(img);

	int x = (int) gid0 > width - 1  ? width - 1 : gid0;
	uint4 pxl = read_imageui(img, x);
	uint4 dstVal = (uint4)(pxl.x + 1, pxl.y + 1, pxl.z + 1, pxl.w + 1);
	write_imageui(dst, x, dstVal);
}

// CHECK: dst,201,64,182,242,5,104,65,6,133,88,70,144,36,139,134,219,128,107,190,64,62,82,233,43,63,51,183,247,197,21,118,66,199,149,243,112,83,136,66,12,129,178,24,232,8,39,251,159,77,93,123,76,73,236,161,193,183,185,18,125,23,194,109,153,23,70,160,57,58,111,252,54,218,73,214,94,239,220,126,6,93,188,133,108,8,232,29,250,176,125,188,151,135,117,212,222,126,248,227,36,112,88,31,140,194,9,26,150,178,202,217,224,155,244,253,222,165,155,233,120,21,231,211,38,206,171,36,172,213,245,161,91,83,17,72,66,23,19,26,34,161,80,243,249,52,186,173,145,184,54,191,191,229,30,167,7,73,157,94,72,182,222,32,227,152,203,145,253,246,130,203,184,103,30,213,87,208,61,239,117,218,192,95,165,151,216,2,48,76,87,186,254,230,196,33,72,33,227,206,103,135,76,235,208,133,154,213,122,84,152,211,120,68,206,33,94,5,32,42,111,177,22,252,158,95,61,41,10,26,175,243,201,61,93,93,239,255,52,141,178,177,22,92,74,25,241,121,133,39,99,26,43,149,121,166,139
