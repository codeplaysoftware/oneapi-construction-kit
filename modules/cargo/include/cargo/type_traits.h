// Copyright (C) Codeplay Software Limited
//
// Licensed under the Apache License, Version 2.0 (the "License") with LLVM
// Exceptions; you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://github.com/codeplaysoftware/oneapi-construction-kit/blob/main/LICENSE.txt
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

/// @file
///
/// @brief Type traits used in cargo.

#ifndef CARGO_TRAITS_H_INCLUDED
#define CARGO_TRAITS_H_INCLUDED

#include <cargo/platform_defines.h>

#include <iterator>
#include <type_traits>

namespace cargo {
namespace detail {
/// @brief Helper struct to turn void_t into void.
///
/// Work around for a C++14 standard defect to enable `void_t`.
///
/// @see http://open-std.org/JTC1/SC22/WG21/docs/cwg_defects.html#1558
///
/// @tparam Ts Template parameter pack.
template <class... Ts>
struct make_void {
  using type = void;
};
}  // namespace detail

/// @addtogroup cargo
/// @{

/// @brief Maps a sequence of any types to void.
///
/// Used to detect ill-formed types in SFINAE context.
///
/// @tparam Ts Template parameter pack.
template <class... Ts>
using void_t = typename detail::make_void<Ts...>::type;

/// @brief Alias for `std::enable_if` to reduce noise.
///
/// @tparam C Boolean condition.
/// @tparam T Specified type.
template <bool C, class T = void>
using enable_if_t = typename std::enable_if<C, T>::type;

/// @brief Alias for `std::aligned_storage` to reduce noise.
///
/// @tparam S Size for aligned storage.
/// @tparam A Alignment for aligned storage.
template <size_t S, size_t A>
using aligned_storage_t = typename std::aligned_storage<S, A>::type;

#ifdef CARGO_GCC49

/// @brief Alias for `std::has_trivial_copy_constructor` using correct name.
///
/// @note GCC versions below 5 do not support
/// `std::is_trivially_copy_constructible`.
///
/// @tparam T Sepcific type.
template <class T>
using is_trivially_copy_constructible = std::has_trivial_copy_constructor<T>;

/// @brief Alias for `std::has_trivial_copy_assign` using correct name.
///
/// @note GCC versions below 5 do not support
/// `std::is_trivially_copy_assignable`.
///
/// @tparam T Sepcific type.
template <class T>
using is_trivially_copy_assignable = std::has_trivial_copy_assign<T>;

/// @brief Implementation of missing `std::is_trivially_copyable`.
///
/// @note GCC versions below 5 do not support `std::is_trivially_copyable`.
///
/// @tparam T Specified type.
template <class T>
struct is_trivially_copyable {
  static const bool value = is_trivially_copy_constructible<T>::value &&
                            is_trivially_copy_assignable<T>::value &&
                            std::is_trivially_destructible<T>::value;
};

#else

/// @brief Alias for `std::has_trivial_copy_constructor` using correct name.
///
/// @note GCC versions below 5 do not support
/// `std::is_trivially_copy_constructible`.
///
/// @tparam T Sepcific type.
template <class T>
using is_trivially_copy_constructible = std::is_trivially_copy_constructible<T>;

/// @brief Alias for `std::has_trivial_copy_assign` using correct name.
///
/// @note GCC versions below 5 do not support
/// `std::is_trivially_copy_assignable`.
///
/// @tparam T Sepcific type.
template <class T>
using is_trivially_copy_assignable = std::is_trivially_copy_assignable<T>;

/// @brief Alias for `std::is_trivially_copyable`.
///
/// @note GCC versions below 5 do not support `std::is_trivially_copyable`.
///
/// @tparam T Specified type.
template <class T>
using is_trivially_copyable = std::is_trivially_copyable<T>;

#endif

/// @brief Alias for `std::remove_reference` to reduce noise.
///
/// @tparam Ts Type to remove reference from.
template <class... Ts>
using remove_reference_t = typename std::remove_reference<Ts...>::type;

/// @brief Alias for `std::remove_pointer` to reduce noise.
///
/// @tparam T Type to remove pointer from.
template <class T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

/// @brief Alias for `std::remove_const` to reduce noise.
///
/// @tparam Ts Type to remove const from.
template <class... Ts>
using remove_const_t = typename std::remove_const<Ts...>::type;

/// @brief Alias for `std::decay` to reduce noise.
///
/// @tparam Ts Type to decay.
template <class... Ts>
using decay_t = typename std::decay<Ts...>::type;

/// @brief Alias for `std::conditional` to reduce noise.
///
/// @tparam B Boolean condition.
/// @tparam T Result if the condition is true.
/// @tparam F Result if the condition is false.
template <bool B, class T, class F>
using conditional_t = typename std::conditional<B, T, F>::type;

/// @}

namespace detail {
/// @brief Failure case, member type `iterator_category` not found.
template <class, class = void>
struct has_iterator_category : public std::false_type {};

/// @brief Success case, member type `iterator_category` found.
///
/// @tparam IteratorTraits Type of iterator traits.
template <class IteratorTraits>
struct has_iterator_category<IteratorTraits,
                             void_t<typename IteratorTraits::iterator_category>>
    : std::true_type {};

/// @brief Success case, determine if iterator category matches.
///
/// @tparam Tag Iterator type tag.
/// @tparam Iterator Type of the iterator.
template <class Tag, class Iterator,
          bool = has_iterator_category<std::iterator_traits<Iterator>>::value>
struct has_iterator_category_convertible_to
    : public std::is_convertible<
          typename std::iterator_traits<Iterator>::iterator_category, Tag> {};

/// @brief Failure case, iterator category does not match.
///
/// @tparam Tag Iterator type tag.
/// @tparam Iterator Type of the iterator.
template <class Tag, class Iterator>
struct has_iterator_category_convertible_to<Tag, Iterator, false>
    : public std::false_type {};
}  // namespace detail

/// @addtogroup cargo
/// @{

/// @brief Determine if the iterator is a random access iterator.
///
/// @tparam Iterator Type of the iterator.
template <class Iterator>
struct is_random_access_iterator
    : public detail::has_iterator_category_convertible_to<
          std::random_access_iterator_tag, Iterator> {};

/// @brief Determine if the iterator is an input iterator.
///
/// @tparam Iterator Type of the iterator.
template <class Iterator>
struct is_input_iterator : public detail::has_iterator_category_convertible_to<
                               std::input_iterator_tag, Iterator> {};

/// @brief Calculates the conjunction of a pack of type traits
///
/// @tparam Traits The traits to calculate the conjunction of.
template <class... Traits>
struct conjunction : std::true_type {};
template <class B>
struct conjunction<B> : B {};
template <class B, class... Bs>
struct conjunction<B, Bs...>
    : std::conditional<bool(B::value), conjunction<Bs...>, B>::type {};

/// @}

namespace detail {
/// @brief Helper for `cargo::is_detected`, this is the false case.
///
/// @tparam Op Template or alias template of the operation to be detected.
/// @tparam Args Type parameter pack describing arguments to the expression.
template <class, template <class...> class Op, class... Args>
struct detector : std::false_type {};

/// @brief Helper for `cargo::is_detected`, this is a true case.
///
/// @tparam Op Template or alias template of the operation to be detected.
/// @tparam Args Type parameter pack describing arguments to the expression.
template <template <class...> class Op, class... Args>
struct detector<void_t<Op<Args...>>, Op, Args...> : std::true_type {};
}  // namespace detail

/// @addtogroup cargo
/// @{

/// @brief Detect if an operation results in a valid expression.
///
/// Example usage:
///
/// ```cpp
/// template <class T>
/// struct my_wrapper {
///   // Alias template describing the operation to be detected.
///   template <class U>
///   using data_member_fn = decltype(std::declval<U>().data());
///
///   // Enable this constuctor if U.data() is detected, disabled otherwise.
///   template <class U,
///             enable_if_t<is_detected<data_member_fn, U>::value> * = nullptr>
///   my_wrapper(const U &value) : data(value.data()) {}
///
///   T *data;
/// };
///
/// std::vector<int> values;
/// my_wrapper<int> wrapper(values);
/// ```
///
/// @tparam Op Template or alias template of the operation to be detected.
/// @tparam Args Type parameter pack describing arguments to the expression.
template <template <class...> class Op, class... Args>
using is_detected = typename detail::detector<void, Op, Args...>;

/// @}

namespace detail {
/// @brief Failure case, member type `value_type` not found.
template <class, class = void>
struct has_value_type : public std::false_type {};

/// @brief Success case, member type `value_type` found.
///
/// @tparam T Type to detect `value_type` member type on.
template <class T>
struct has_value_type<T, void_t<typename T::value_type>> : std::true_type {};

/// @brief Failure case, member type `iterator` not found.
template <class, class = void>
struct has_iterator : public std::false_type {};

/// @brief Success case, member type `iterator` found.
///
/// @tparam T Type to detect `iterator` member type on.
template <class T>
struct has_iterator<T, void_t<typename T::iterator>> : std::true_type {};
}  // namespace detail

/// @addtogroup cargo
/// @{

/// @brief Success case, determine if `value_type` matches.
///
/// @tparam T Type to check `value_type` is convertible to.
/// @tparam U Type to check if `value_type` member type can is convertible.
template <class T, class U, bool = detail::has_value_type<U>::value,
          bool = detail::has_iterator<U>::value>
struct has_value_type_convertible_to
    : public std::is_convertible<typename U::value_type, T> {};

/// @brief Success case, determine if iterator's `value_type` matches, when
/// there's no value_type but there is an iterator. This is here to handle
/// llvm::StringRef.
///
/// @tparam T Type to check `value_type` is convertible to.
/// @tparam U Type to check if `value_type` member type can is convertible.
template <class T, class U>
struct has_value_type_convertible_to<T, U, false, true>
    : public std::is_convertible<
          typename std::iterator_traits<typename U::iterator>::value_type, T> {
};

/// @brief Failure case, `value_type` does not match.
///
/// @tparam T Type to check `value_type` is convertible to.
/// @tparam U Type to check if `value_type` member type can is convertible.
template <class T, class U>
struct has_value_type_convertible_to<T, U, false, false>
    : public std::false_type {};

/// @}

namespace detail {
/// @brief Check if `U` has a member function `data`.
///
/// @tparam U Type to detect `data` member function on.
template <class U>
using data_member_fn = decltype(std::declval<U>().data());

/// @brief Check if `U` has a member function `size`.
///
/// @tparam U Type to detect `size` member function on.
template <class U>
using size_member_fn = decltype(std::declval<U>().size());

/// @brief Check if `U` has a subscript operator.
///
/// @tparam U Type to detect subscript operator on.
template <class U>
using subscript_member_fn = decltype(std::declval<U>()[std::declval<size_t>()]);

/// @brief Check if `U` has a member function `alloc`.
///
/// @tparam U Type to detect `alloc` member function on.
template <class U>
using alloc_member_fn =
    decltype(std::declval<U>().alloc(std::declval<size_t>()));

/// @brief Check if `U` has a member function `capacity`.
///
/// @tparam U Type to detect `capacity` member function on.
template <class U>
using capacity_member_fn = decltype(std::declval<U>().capacity());

/// @brief Check if `U` can be constructed from two iterators.
///
/// @tparam U Type to detect iterator constructor on.
/// @tparam iterator_type Type of the iterator used for constructing U.
template <class U, typename iterator_type>
using iterator_constructor =
    decltype(U(std::declval<iterator_type>(), std::declval<iterator_type>()));

/// @brief Check if `U` can be constructed from a pointer and a length.
///
/// `std::string` is an example of a class that can be constructed like this.
///
/// @tparam U Type to detect the constructor on.
/// @tparam pointer_type Type of a pointer to the underlying data.
/// @tparam size_type Type for length (e.g., `size_t`).
template <class U, typename pointer_type, typename size_type = std::size_t>
using ptr_len_constructor =
    decltype(U(std::declval<pointer_type>(), std::declval<size_type>()));
}  // namespace detail

}  // namespace cargo

#endif  // CARGO_TRAITS_H_INCLUDED
