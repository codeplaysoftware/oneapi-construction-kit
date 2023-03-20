// Copyright (C) Codeplay Software Limited. All Rights Reserved.

#include "mux/select.h"
#include "mux/utils/id.h"
#include "tracer/tracer.h"

mux_result_t muxCreateImage(mux_device_t device, mux_image_type_e type,
                            mux_image_format_e format, uint32_t width,
                            uint32_t height, uint32_t depth,
                            uint32_t array_layers, uint64_t row_size,
                            uint64_t slice_size,
                            mux_allocator_info_t allocator_info,
                            mux_image_t *out_image) {
  tracer::TraceGuard<tracer::Mux> guard(__func__);

  if (mux::objectIsInvalid(device)) {
    return mux_error_invalid_value;
  }

  {
    auto checkMinMax = [](uint32_t value, uint32_t max) -> bool {
      return value == 0 || value > max;
    };

    switch (type) {
      case mux_image_type_1d:
        if (checkMinMax(width, device->info->max_image_dimension_1d) ||
            1 != height || 1 != depth) {
          return mux_error_invalid_value;
        }
        break;
      case mux_image_type_2d:
        if (checkMinMax(width, device->info->max_image_dimension_2d) ||
            checkMinMax(height, device->info->max_image_dimension_2d) ||
            1 != depth) {
          return mux_error_invalid_value;
        }
        break;
      case mux_image_type_3d:
        if (checkMinMax(width, device->info->max_image_dimension_3d) ||
            checkMinMax(height, device->info->max_image_dimension_3d) ||
            checkMinMax(depth, device->info->max_image_dimension_3d)) {
          return mux_error_invalid_value;
        }
        break;
      default:
        return mux_error_invalid_value;
    }
  }

  if (array_layers && device->info->max_image_array_layers < array_layers) {
    return mux_error_invalid_value;
  }

  if (mux::allocatorInfoIsInvalid(allocator_info)) {
    return mux_error_null_allocator_callback;
  }

  if (nullptr == out_image) {
    return mux_error_null_out_parameter;
  }

  mux_result_t error = muxSelectCreateImage(
      device, type, format, width, height, depth, array_layers, row_size,
      slice_size, allocator_info, out_image);

  if (mux_success == error) {
    mux::setId<mux_object_id_image>(device->info->id, *out_image);
  }

  return error;
}

void muxDestroyImage(mux_device_t device, mux_image_t image,
                     mux_allocator_info_t allocator_info) {
  tracer::TraceGuard<tracer::Mux> guard(__func__);

  if (mux::objectIsInvalid(device)) {
    return;
  }

  if (mux::objectIsInvalid(image)) {
    return;
  }

  if (mux::allocatorInfoIsInvalid(allocator_info)) {
    return;
  }

  muxSelectDestroyImage(device, image, allocator_info);
}

mux_result_t muxBindImageMemory(mux_device_t device, mux_memory_t memory,
                                mux_image_t image, uint64_t offset) {
  tracer::TraceGuard<tracer::Mux> guard(__func__);

  if (mux::objectIsInvalid(device)) {
    return mux_error_invalid_value;
  }

  if (mux::objectIsInvalid(memory)) {
    return mux_error_invalid_value;
  }

  if (mux::objectIsInvalid(image)) {
    return mux_error_invalid_value;
  }

  if (memory->size < offset) {
    return mux_error_invalid_value;
  }

  if (memory->size < (image->memory_requirements.size + offset)) {
    return mux_error_invalid_value;
  }

  return muxSelectBindImageMemory(device, memory, image, offset);
}

mux_result_t muxGetSupportedImageFormats(mux_device_t device,
                                         mux_image_type_e image_type,
                                         mux_allocation_type_e allocation_type,
                                         uint32_t count,
                                         mux_image_format_e *out_formats,
                                         uint32_t *out_count) {
  tracer::TraceGuard<tracer::Mux> guard(__func__);

  if (mux::objectIsInvalid(device)) {
    return mux_error_invalid_value;
  }

  switch (image_type) {
    case mux_image_type_1d:
    case mux_image_type_2d:
    case mux_image_type_3d:
      break;
    default:
      return mux_error_invalid_value;
  }

  switch (allocation_type) {
    case mux_allocation_type_alloc_host:
    case mux_allocation_type_alloc_device:
      break;
    default:
      return mux_error_invalid_value;
  }

  if (0 == count && nullptr != out_formats) {
    return mux_error_invalid_value;
  }

  return muxSelectGetSupportedImageFormats(device, image_type, allocation_type,
                                           count, out_formats, out_count);
}