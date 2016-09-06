#pragma once

/***
 * A wml tag container which holds a sequence of tags of several possible types
 */

#include <libwml/child_containers.hpp>
#include <libwml/traits/tag.hpp>
#include <libwml/util/variant.hpp>
#include <libwml/wml.hpp>
#include <libwml/coerce_log.hpp>

namespace wml {

template <typename ... Ts>
struct heterogenous_helper;

template <typename T, typename... Ts>
struct heterogenous_helper<T, Ts...> {

  static bool allow_convert(const wml::body & b) {
    if (traits::tag<T>::name() == b.name) { return true; }
    else { return heterogenous_helper<Ts...>::allow_convert(b); }
  }

  template <typename return_t>
  static return_t convert_impl(const wml::body & b, coerce_log * log) {
    static_assert(traits::tag<T>::value, "heterogenous sequence must be used with types which are wml tags!");

    if (traits::tag<T>::name() == b.name) {
      T result;
      coerce_to_tag(result, b, log);
      return result;
    } else {
      return heterogenous_helper<Ts...>::template convert_impl<return_t>(b, log);
    }
  }
};

// Support for recursive_wrapper, we just have to strip it
template <typename T, typename... Ts>
struct heterogenous_helper<util::recursive_wrapper<T>, Ts...> : heterogenous_helper<T, Ts...> {};

// Empty: Means we ran out of types to try
template <>
struct heterogenous_helper<> {
  static bool allow_convert(const wml::body &) { return false; }
  template <typename return_t>
  static return_t convert_impl(const wml::body & b, coerce_log * log) {
    assert(false && "Bottomed out when inserting into heterogenous container!");
    // return *static_cast<return_t*>(nullptr);
  }
};

template <typename... Ts>
struct heterogenous_sequence {
  using helper_t = heterogenous_helper<Ts...>;

  using var_t = util::variant<Ts...>;

  std::vector<var_t> children;
};

namespace traits {

template <typename... Ts>
struct child_container<heterogenous_sequence<Ts...>> : std::true_type {
  using seq_t = heterogenous_sequence<Ts...>;
  using helper_t = typename seq_t::helper_t;
  using var_t = typename seq_t::var_t;

  static bool allows_tag(const wml::body & b) { return helper_t::allow_convert(b); }
  static void insert_tag(seq_t & s, const wml::body & b, coerce_log * log) {
    s.children.emplace_back(helper_t::template convert_impl<var_t>(b, log));
  }
};

} // end namespace traits
} // end namespace wml
