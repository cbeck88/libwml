#pragma once

// We currently use strict_variant for our variant type.

#include <strict_variant/variant.hpp>

namespace util {

template <typename... types>
using variant = strict_variant::variant<types...>;

template <typename T>
using recursive_wrapper = strict_variant::recursive_wrapper<T>;

template <typename Vis, typename Var>
decltype(auto)
apply_visitor(Vis && vis, Var && var) {
  return strict_variant::apply_visitor(std::forward<Vis>(vis), std::forward<Var>(var));
}

template <typename T, typename... types>
T *
get(variant<types...> * v) {
  return strict_variant::get<T>(v);
}

template <typename T, typename... types>
const T *
get(const variant<types...> * v) {
  return strict_variant::get<T>(v);
}

// using get_or_default = strict_variant::get_or_default;

} // end namespace util
