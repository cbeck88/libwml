#pragma once

/***
 * A wml tag container which holds a sequence of tags of several possible types,
 * possibly also with alias names, which are used instead of what the tag trait
 * says.
 */

#include <libwml/mpl.hpp>
#include <libwml/child_containers.hpp>
#include <libwml/traits/tag.hpp>
#include <libwml/util/variant.hpp>
#include <libwml/wml.hpp>
#include <libwml/coerce_log.hpp>

namespace wml {

template <const char * const * strings, typename TL>
struct heterogenous_helper;

template <typename T, typename... Ts>
struct heterogenous_helper<const char * const * strings, mpl::typelist<T, Ts...>> {

  static bool allow_convert(const wml::body & b) {
    if (*strings == b.name) { return true; }
    else { return heterogenous_helper<strings+1, Ts...>::allow_convert(b); }
  }

  template <typename return_t>
  static return_t convert_impl(const wml::body & b, coerce_log * log) {

    if (*strings == b.name) {
      using U = util::unwrap_type_t<T>;
      static_assert(traits::tag<U>::value, "heterogenous sequence must be used with types which are wml tags!");
      U result;
      traits::tag<U>::coerce(result, b, log);
      return result;
    } else {
      return heterogenous_helper<strings+1, Ts...>::template convert_impl<return_t>(b, log);
    }
  }
};

// Empty: Means we ran out of types to try
template <>
struct heterogenous_helper<const char * const *, mpl::typelist<>> {
  static bool allow_convert(const wml::body &) { return false; }
  template <typename return_t>
  static return_t convert_impl(const wml::body & b, coerce_log * log) {
    assert(false && "Bottomed out when inserting into heterogenous container!");
    // return *static_cast<return_t*>(nullptr);
  }
};

template <const char * const * strings, typename... Ts>
struct heterogenous_sequence {
  using helper_t = heterogenous_helper<strings, mpl::typelist<Ts...>>;

  using var_t = util::variant<Ts...>;

  std::vector<var_t> children;

  static const char * name(const var_t & v) {
    return strings[v.which()];
  }
};

namespace traits {

template <const char * const * strings, typename... Ts>
struct child_container<heterogenous_sequence<strings, Ts...>> : std::true_type {
  using seq_t = heterogenous_sequence<strings, Ts...>;
  using helper_t = typename seq_t::helper_t;
  using var_t = typename seq_t::var_t;

  static bool allows_tag(const wml::body & b) { return helper_t::allow_convert(b); }
  static void insert_tag(seq_t & s, const wml::body & b, coerce_log * log) {
    s.children.emplace_back(helper_t::template convert_impl<var_t>(b, log));
  }
};

} // end namespace traits

} // end namespace wml
