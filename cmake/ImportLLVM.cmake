# Copyright (C) Codeplay Software Limited. All Rights Reserved.

#[=======================================================================[.rst:
Module imports LLVM into the ComputeAorta build by pulling in LLVM CMake
modules from the ``install`` directory of a build.

User option :cmake:variable:`CA_LLVM_INSTALL_DIR` is required to be set
to the filepath of the LLVM install before including this module. The option
:cmake:variable:`CA_RUNTIME_COMPILER_ENABLED` is also required to be ``TRUE``
to use this module, otherwise a LLVM-less offline build is desired.

.. code:: cmake

  set(CA_LLVM_INSTALL_DIR "<path/to/install/directory>")
  set(CA_RUNTIME_COMPILER_ENABLED TRUE)
  include(ImportLLVM)

Using this location we append the directories containing LLVM CMake modules
to :cmake-variable:`CMAKE_MODULE_PATH`, and import the following LLVM modules:

* :cmake:module:`LLVMConfig` for LLVM.
* :cmake:module:`ClangTargets` for Clang.

Once the modules are included we verify the LLVM version is supported and set
some additional compile definitions.
#]=======================================================================]

if(CA_RUNTIME_COMPILER_ENABLED AND NOT CA_LLVM_INSTALL_DIR)
  message(FATAL_ERROR
    "CA_LLVM_INSTALL_DIR must be given when CA_RUNTIME_COMPILER_ENABLED is set")
endif()

# Add our cmake modules directory to the cmake include path including
# LLVM/Clang.
string(REPLACE "\\" "/" CA_LLVM_INSTALL_DIR "${CA_LLVM_INSTALL_DIR}")
if(NOT EXISTS "${CA_LLVM_INSTALL_DIR}/lib/cmake/llvm/LLVMConfig.cmake")
  message(FATAL_ERROR
    "'${CA_LLVM_INSTALL_DIR}/lib/cmake/llvm/LLVMConfig.cmake' does not exist"
    " (search path set with CA_LLVM_INSTALL_DIR)")
endif()
if(NOT EXISTS "${CA_LLVM_INSTALL_DIR}/lib/cmake/clang/ClangTargets.cmake")
  message(FATAL_ERROR
    "'${CA_LLVM_INSTALL_DIR}/lib/cmake/clang/ClangTargets.cmake' does not exist"
    " (search path set with CA_LLVM_INSTALL_DIR)")
endif()
list(APPEND CMAKE_MODULE_PATH
  ${CA_LLVM_INSTALL_DIR}/lib/cmake/llvm
  ${CA_LLVM_INSTALL_DIR}/lib/cmake/clang)

# Include LLVM.
include(LLVMConfig)
# Include Clang.
include(ClangTargets)
# Detect which CRT LLVM was built with.
include(DetectLLVMMSVCCRT)

#[=======================================================================[.rst:
.. cmake:variable:: CA_LLVM_VERSIONS

  Defines a list of supported LLVM versions, used to verify that
  :cmake:variable:`LLVM_PACKAGE_VERSION` from the imported LLVM install is
  a supported version.
#]=======================================================================]
set(CA_LLVM_MINIMUM_VERSION 14.0.0)
set(CA_LLVM_MAXIMUM_VERSION 15.0.4)
string(REPLACE ";" "', '" CA_LLVM_VERSIONS_PRETTY "${CA_LLVM_VERSIONS}")
if("${LLVM_PACKAGE_VERSION}" VERSION_LESS "${CA_LLVM_MINIMUM_VERSION}")
  message(FATAL_ERROR
    "LLVM version '${LLVM_PACKAGE_VERSION}' is not supported. ComputeAorta "
    "supports released versions between: "
    "'${CA_LLVM_MINIMUM_VERSION}' and '${CA_LLVM_MAXIMUM_VERSION}'.")
else()
  if("${LLVM_PACKAGE_VERSION}" VERSION_GREATER "${CA_LLVM_MAXIMUM_VERSION}")
    message(AUTHOR_WARNING
      "Build with LLVM version '${LLVM_PACKAGE_VERSION}' which is not "
      "supported. ComputeAorta supports released versions between: "
      "'${CA_LLVM_MINIMUM_VERSION}' and '${CA_LLVM_MAXIMUM_VERSION}'.")
  endif()
endif()
message(STATUS "ComputeAorta using LLVM ${LLVM_PACKAGE_VERSION}")

# Stay consistent with LLVM's HandleLLVMOptions.cmake,
# by making type 'off_t' size 64 bit so we can access > 2GB.
# LLVM 8.0 changed how these options are set for MinGW.
if(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CA_BUILD_32_BITS OR
    (MINGW AND LLVM_VERSION_MAJOR GREATER 7))
  add_definitions(-D_LARGEFILE_SOURCE)
  add_definitions(-D_FILE_OFFSET_BITS=64)
endif()