// Copyright (C) Codeplay Software Limited. All Rights Reserved.
//
// DEFINITIONS: "-DREAD_LOCAL_SIZE=4";"-DGLOBAL_ID=0"

__kernel void barrier_in_loop_2(__global int* output) {
  int global_id = get_global_id(0);
  int local_id = get_local_id(0);
  __local int cache[READ_LOCAL_SIZE];

  int my_value = 0;

  for (unsigned int i = 0; i < get_local_size(0); i += get_local_size(0)) {
    // The access to cache at this stage is required to reveal the bug
    // however, the if condition is intentionally not met.
    if (i > get_local_size(0) / 2) {
      my_value = cache[local_id];
    }

    for (unsigned int j = 0; j < get_local_size(0); j++) {
      cache[local_id] = local_id;

      // This is the barrier that can cause issues in certain cases.
      barrier(CLK_LOCAL_MEM_FENCE);

      // Unrolled loop so we don't hide the bug
      my_value += cache[0];
      my_value += cache[1];
      my_value += cache[2];
      my_value += cache[3];
    }

    // The access to cache at this stage is required to reveal the bug
    // however, the if condition is intentionally not met.
    if (get_local_id(0) > get_local_size(0)) {
      my_value = cache[get_local_id(0)];
    }
  }

  if (get_global_id(0) == GLOBAL_ID) {
    output[0] = my_value;
  }
}