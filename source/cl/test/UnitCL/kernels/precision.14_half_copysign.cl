// Copyright (C) Codeplay Software Limited. All Rights Reserved.

// REQUIRES: half
#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#ifndef TYPE
#define TYPE half
#endif

__kernel void half_copysign(__global TYPE* in1, __global TYPE* in2,
                            __global TYPE* out) {
  size_t tid = get_global_id(0);
  out[tid] = copysign(in1[tid], in2[tid]);
}