 // Copyright (C) Codeplay Software Limited. All Rights Reserved.

#ifndef {{cookiecutter.target_name_capitals}}_RISCV_CLMUL_H_INCLUDED
#define {{cookiecutter.target_name_capitals}}_RISCV_CLMUL_H_INCLUDED
 
 #include <extension/extension.h>
 
 namespace extension {
 
 class riscv_clmul final : public extension {
  public:
   riscv_clmul();
 };
 
 }  // namespace extension
 #endif