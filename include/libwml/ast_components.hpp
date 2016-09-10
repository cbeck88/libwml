#pragma once

#include <libwml/wml.hpp>
#include <libwml/coercer.hpp>
#include <libwml/coerce_log.hpp>
#include <libwml/traits/child_container.hpp>
#include <libwml/traits/tag.hpp>
#include <libwml/util/optional.hpp>
#include <libwml/util/variant.hpp>
#include <cassert>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace wml {

// C++ types referred to by XML file

using int_val = int;
using bool_val = bool;
using string_val = std::string;
using string_list_val = std::vector<std::string>;
using string_to_int_map = std::map<std::string, int_val>;

template <typename T>
using child_vector = std::vector<T>;

using string_opt = util::optional<std::string>;
using int_opt = util::optional<int>;
using bool_opt = util::optional<bool>;

// tag trait

template <typename T>
struct tag_base : std::true_type {
  static void coerce(T & t, const wml::config & cfg, coerce_log * log) {
    coercer c{cfg, log};
    t.visit_extended(c);
    c.report_unused(); 
  }
};

// heterogenous sequence
//
// Defines a tag container of the form std::vector<util::variant<...>>
// This definition is made using CRTP, the variant type and aliases for the
// tags should be defined by the template parameter to heterogenous_sequence_base.

template <typename T>
struct heterogenous_sequence_base {
  using var_t = typename T::var_t;
  std::vector<var_t> contents;

  static int find_tag(const std::string & s) {
    int result = 0;
    while (result < T::num_types && T::names()[result] != s) { ++result; }
    return result;
  }

  static bool has_tag(const std::string & s) {
    return find_tag(s) < T::num_types;
  }

  static const char * name(const var_t & v) {
    return T::names()[v.which()];
  }

  // Visitor for var_t which applies the coerce function of current tag type.
  struct coerce_visitor {
    explicit coerce_visitor(const wml::config & cfg, coerce_log * log)
      : cfg_(cfg)
      , log_(log)
    {}

    const wml::config & cfg_;
    coerce_log * log_;

    template <typename U>
    void operator()(U & u) const {
      traits::tag<U>::coerce_to_tag(u, cfg_, log_);
    }
  };

  // Metaprogramming which changes the type of a variant to match a given index
  // and coerces it to match some wml::config
  template <typename IS>
  struct helper;

  template <int... Is>
  struct helper<std::integer_sequence<int, Is...>> {
    using emplacer_fcn = void(*)(var_t &);

    template <int idx>
    static void emplace_impl(var_t & v) {
      v.emplace<idx>({});
    }

    static var_t as_variant(int idx, const wml::config & cfg, coerce_log * log) {
      var_t result;

      constexpr std::array<emplacer_fcn, sizeof...(Is)> fcns = {{
        &emplace_impl<Is>...
      }};

      fcns[idx](result);
      coerce_visitor vis{cfg, log};
      util::apply_visitor(vis, result);

      return result;
    }
  };

  static var_t as_variant(const wml::body & b, coerce_log * log) {
    int idx = find_tag(b.name);
    assert(idx < T::num_types && "Improper coercion of a wml tag!");

    return helper<std::make_integer_sequence<int, T::num_types>>::as_variant(idx, b.children, log);
  }

  void emplace_back(const wml::body & b, coerce_log * log = nullptr) {
    contents->emplace_back(as_variant(b, log));
  }
};

// heterogenous sequence trait

template <typename T>
struct hs_trait_base : std::true_type {
  using seq_t = T;
  using var_t = typename seq_t::var_t;

  static bool allows_tag(const wml::body & b) { return seq_t::has_tag(b.name); }
  static void insert_tag(seq_t & s, const wml::body & b, coerce_log * log) {
    s.emplace_back(b, log);
  }
};

} // end namespace wml
