/// Copyright (C) Codeplay Software Limited
///
/// Licensed under the Apache License, Version 2.0 (the "License") with LLVM
/// Exceptions; you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     https://github.com/codeplaysoftware/oneapi-construction-kit/blob/main/LICENSE.txt
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
/// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
/// License for the specific language governing permissions and limitations
/// under the License.
///
/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
{% if cookiecutter.copyright_name != "" -%}
/// Additional changes Copyright (C) {{cookiecutter.copyright_name}}. All Rights
/// Reserved.
{% endif -%}

#include <mux/mux.h>
#include <{{cookiecutter.target_name}}/device_info.h>
#include <{{cookiecutter.target_name}}/device_info_get.h>
#include <{{cookiecutter.target_name}}/hal.h>

#include <array>
#include <cassert>

namespace {{cookiecutter.target_name}} {

bool enumerate_device_infos() {
  // if we have a valid device_info we have already enumerated and can return
  if (device_infos[0].is_valid()) {
    return true;
  }
  // load the hal library
  hal::hal_t *hal = {{cookiecutter.target_name}}::hal_get();
  if (!hal) {
    return false;
  }
  const auto &hal_info = hal->get_info();
  // check we have something to enumerate
  if (hal_info.num_devices == 0) {
    return false;
  }
  // enumerate all reported devices
  uint32_t j = 0;
  for (uint32_t i = 0; i < hal_info.num_devices; ++i) {
    assert(i < device_infos.size());
    const auto *hal_dev_info = hal->device_get_info(i);

    // update this device_info entry and continue
    auto &dev_info = device_infos[j++];
    dev_info.update_from_hal_info(hal_dev_info);
    dev_info.hal_device_index = i;
    // device info should be valid at this point
    assert(dev_info.is_valid());
  }

  // success if we have at least one device
  return j > 0;
}

std::array<{{cookiecutter.target_name}}::device_info_s, {{cookiecutter.target_name}}::max_device_infos> device_infos;

cargo::array_view<{{cookiecutter.target_name}}::device_info_s> GetDeviceInfosArray() {
  // ensure our device infos have been enumerated
  if (!{{cookiecutter.target_name}}::enumerate_device_infos()) {
    return {};
  }
  return {{cookiecutter.target_name}}::device_infos;
}

}  // namespace {{cookiecutter.target_name}}
