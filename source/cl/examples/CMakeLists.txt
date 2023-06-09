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

#[=======================================================================[.rst:
.. cmake:command:: add_ca_cl_example

  The ``add_ca_example()`` function creates an executable using
  :cmake:command:`add_ca_cl_executable`, adds a check target, and
  places the target in the install directory.

  Arguments:
    * ``ARGV0`` - Target name for example
    * ``ARGN`` - C/C++ source files

#]=======================================================================]
function(add_ca_cl_example)
  add_ca_cl_executable(${ARGV})
  target_include_directories(${ARGV0} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

  add_ca_cl_check(${ARGV0}
    COMMAND ${ARGV0} --platform ComputeAorta
    DEPENDS ${ARGV0})

  install(TARGETS ${ARGV0}
    RUNTIME DESTINATION bin COMPONENT OCLExamples)
endfunction()

add_ca_cl_example(clVectorAddition
    ${CMAKE_CURRENT_SOURCE_DIR}/VectorAddition/source/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common.h)

if(${OCL_EXTENSION_cl_khr_command_buffer})
  add_ca_cl_example(clCommandBufferKHR
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandBufferKHR/source/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common.h)
endif()

if(${OCL_EXTENSION_cl_khr_command_buffer_mutable_dispatch})
  add_ca_cl_example(clMutableDispatchKHR
    ${CMAKE_CURRENT_SOURCE_DIR}/MutableDispatchKHR/source/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common.h)
endif()

add_ca_cl_example(clSubGroups
  ${CMAKE_CURRENT_SOURCE_DIR}/SubGroups/source/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/common.h)
