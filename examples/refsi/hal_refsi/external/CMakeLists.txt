# Copyright (C) Codeplay Software Limited
#
# Licensed under the Apache License, Version 2.0 (the "License") with LLVM
# Exceptions; you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://github.com/codeplaysoftware/oneapi-construction-kit/blob/main/LICENSE.txt
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

if (NOT TARGET hal_common)
  set(HAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/hal")
  if (NOT EXISTS ${HAL_DIR}/include/hal.h)
    message(FATAL_ERROR "Could not find HAL API headers. Is there a 'hal' checkout at ${HAL_DIR} ?")
  endif()
  add_subdirectory(hal)
endif()

if (NOT TARGET refsidrv)
  add_subdirectory(refsidrv)
endif()
