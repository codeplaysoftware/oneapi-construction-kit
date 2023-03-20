// Copyright (C) Codeplay Software Limited. All Rights Reserved.

#ifndef __ABACUS_INTERNAL_RSQRT_INITIAL_GUESS_H__
#define __ABACUS_INTERNAL_RSQRT_INITIAL_GUESS_H__

#include <abacus/abacus_cast.h>
#include <abacus/abacus_config.h>
#include <abacus/abacus_detail_cast.h>
#include <abacus/abacus_type_traits.h>

namespace abacus {
namespace internal {
template <typename T>
inline typename TypeTraits<T>::UnsignedType rsqrt_initial_guess_helper();

#ifdef __CA_BUILTINS_HALF_SUPPORT
template <>
inline abacus_ushort rsqrt_initial_guess_helper<abacus_half>() {
  // Worked out using https://cs.uwaterloo.ca/~m32rober/rsqrt.pdf
  // Page 42, equation 4.10
  // See http://h14s.p5r.org/2012/09/0x5f3759df.html?mwh=1 for a more readable
  // discussion
  return 0x59ba;
}
#endif  //__CA_BUILTINS_HALF_SUPPORT

template <>
inline abacus_uint rsqrt_initial_guess_helper<abacus_float>() {
  return 0x5f3759dfu;
}

#ifdef __CA_BUILTINS_DOUBLE_SUPPORT
template <>
inline abacus_ulong rsqrt_initial_guess_helper<abacus_double>() {
  return 0x5fe6eb50c7b537a9u;
}
#endif  // __CA_BUILTINS_DOUBLE_SUPPORT

template <typename T>
inline T rsqrt_initial_guess(T x) {
  typedef typename TypeTraits<T>::UnsignedType UnsignedType;
  typedef typename TypeTraits<T>::ElementType ElementType;

  const UnsignedType splat = rsqrt_initial_guess_helper<ElementType>();

  return abacus::detail::cast::as<T>(
      UnsignedType((splat - (abacus::detail::cast::as<UnsignedType>(x) >> 1))));
}
}  // namespace internal
}  // namespace abacus

#endif  //__ABACUS_INTERNAL_RSQRT_INITIAL_GUESS_H__