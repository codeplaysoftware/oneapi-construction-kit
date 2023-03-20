// Copyright (C) Codeplay Software Limited. All Rights Reserved.

// REQUIRES: half
#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#ifndef TYPE
#define TYPE half
#endif

__kernel void half_fmod(__global TYPE* a, __global TYPE* b,
                        __global TYPE* out) {
  size_t tid = get_global_id(0);
  out[tid] = fmod(a[tid], b[tid]);
}