//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>
#include <utility>

/***
 * Some traits that seem like they could (should?) be standard but aren't.
 */

namespace strict_variant {
namespace mpl {

// Modified version of std::common_type

// Support: Need a std::decay which allows l-value references to pass through
template <class T>
using mini_decay_t = typename std::conditional<std::is_lvalue_reference<T>::value, T,
                                               typename std::decay<T>::type>::type;

// Our version of common_type uses mini_decay instead of decay
template <typename T, typename... Ts>
struct common_return_type;

template <typename T>
struct common_return_type<T> {
  using type = mini_decay_t<T>;
};

template <typename T1, typename T2>
struct common_return_type<T1, T2> {
  using type = mini_decay_t<decltype(true ? std::declval<T1>() : std::declval<T2>())>;
};

template <typename T1, typename T2, typename T3, typename... Ts>
struct common_return_type<T1, T2, T3, Ts...> {
  using type =
    typename common_return_type<typename common_return_type<T1, T2>::type, T3, Ts...>::type;
};

template <typename T, typename... Ts>
using common_return_type_t = typename common_return_type<T, Ts...>::type;

// is_nothrow_swappable

namespace adl_swap_ns {

using std::swap;

template <typename T>
struct is_nothrow_swappable {
  static constexpr bool value =
    noexcept(swap(*static_cast<T *>(nullptr), *static_cast<T *>(nullptr)));
};

} // end namespace adl_swap_ns

using adl_swap_ns::is_nothrow_swappable;

} // end namespace mpl
} // end namespace strict_variant
