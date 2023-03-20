// Copyright (C) Codeplay Software Limited. All Rights Reserved.

#include "mux/select.h"
#include "mux/utils/id.h"
#include "tracer/tracer.h"

mux_result_t muxGetQueue(mux_device_t device, mux_queue_type_e queue_type,
                         uint32_t queue_index, mux_queue_t *out_queue) {
  tracer::TraceGuard<tracer::Mux> guard(__func__);

  if (mux::objectIsInvalid(device)) {
    return mux_error_invalid_value;
  }

  switch (queue_type) {
    default:
      // bad queue type provided!
      return mux_error_invalid_value;
    case mux_queue_type_compute:
      break;
  }

  if (device->info->queue_types[queue_type] <= queue_index) {
    // our queue index was out of bounds for the queues we have!
    return mux_error_invalid_value;
  }

  if (nullptr == out_queue) {
    return mux_error_null_out_parameter;
  }

  mux_result_t error =
      muxSelectGetQueue(device, queue_type, queue_index, out_queue);

  // Note that all the muxCreate* functions do mux::setId on their object
  // here, but this is a muxGet* function and thus the object may not be
  // unique.  The user can assume that muxGetQueue is thread-safe, and thus we
  // can't do mux::setId on the object here because that is a data-race.

  return error;
}

mux_result_t muxDispatch(
    mux_queue_t queue, mux_command_buffer_t command_buffer, mux_fence_t fence,
    mux_semaphore_t *wait_semaphores, uint32_t wait_semaphores_length,
    mux_semaphore_t *signal_semaphores, uint32_t signal_semaphores_length,
    void (*user_function)(mux_command_buffer_t command_buffer,
                          mux_result_t error, void *const user_data),
    void *user_data) {
  tracer::TraceGuard<tracer::Mux> guard(__func__);

  if (mux::objectIsInvalid(queue)) {
    return mux_error_invalid_value;
  }

  if (mux::objectIsInvalid(command_buffer)) {
    return mux_error_invalid_value;
  }

  // The fence parameter is optional.
  if (fence && mux::objectIsInvalid(fence)) {
    return mux_error_invalid_value;
  }

  if ((nullptr == wait_semaphores) ^ (0 == wait_semaphores_length)) {
    return mux_error_invalid_value;
  }

  for (uint32_t index = 0; index < wait_semaphores_length; index++) {
    if (mux::objectIsInvalid(wait_semaphores[index])) {
      return mux_error_invalid_value;
    }
  }

  if ((nullptr == signal_semaphores) ^ (0 == signal_semaphores_length)) {
    return mux_error_invalid_value;
  }

  for (uint32_t index = 0; index < signal_semaphores_length; index++) {
    if (mux::objectIsInvalid(signal_semaphores[index])) {
      return mux_error_invalid_value;
    }
  }

  if ((nullptr == user_function) && (nullptr != user_data)) {
    return mux_error_invalid_value;
  }

  return muxSelectDispatch(queue, command_buffer, fence, wait_semaphores,
                           wait_semaphores_length, signal_semaphores,
                           signal_semaphores_length, user_function, user_data);
}

mux_result_t muxTryWait(mux_queue_t queue, uint64_t timeout,
                        mux_fence_t fence) {
  tracer::TraceGuard<tracer::Mux> guard(__func__);

  if (mux::objectIsInvalid(queue)) {
    return mux_error_invalid_value;
  }

  if (mux::objectIsInvalid(fence)) {
    return mux_error_invalid_value;
  }

  return muxSelectTryWait(queue, timeout, fence);
}

mux_result_t muxWaitAll(mux_queue_t queue) {
  tracer::TraceGuard<tracer::Mux> guard(__func__);

  if (mux::objectIsInvalid(queue)) {
    return mux_error_invalid_value;
  }

  return muxSelectWaitAll(queue);
}