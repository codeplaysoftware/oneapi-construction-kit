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

add_library(clik_runtime_async SHARED
  clik_async.cpp
  clik_command_queue.cpp
)

target_link_libraries(clik_runtime_async hal_common pthread)

target_include_directories(clik_runtime_async PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_definitions(clik_runtime_async PUBLIC -DCLIK_DLL)
target_compile_definitions(clik_runtime_async PRIVATE -DCLIK_HAL_NAME="${CLIK_HAL_NAME}")

install(TARGETS clik_runtime_async
        LIBRARY DESTINATION lib
)

install(FILES ${PROJECT_SOURCE_DIR}/include/clik_async_api.h DESTINATION include)
