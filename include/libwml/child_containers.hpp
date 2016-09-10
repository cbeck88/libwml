#pragma once

/***
 * Some implementations of child container templates.
 *
 * - child_vector
 *   stores a sequence of children of a particular type
 * - all_children_map
 *   catch all which stores any child tags in a map, using their name as key
 *   this is really useful only as a fallback
 *
 * Heterogenous sequence is in its own file
 */

#include <cassert>
#include <map>
#include <type_traits>
#include <utility>
#include <vector>

#include <libwml/coerce_log.hpp>
#include <libwml/traits/child_container.hpp>
#include <libwml/traits/tag.hpp>
#include <libwml/util/variant.hpp>

namespace wml {
namespace traits {

template <typename T>
struct child_container<std::vector<T>> : std::true_type {
  static bool allows_tag(const wml::body & b) { return b.name == tag<T>::name(); }

  static void insert_tag(std::vector<T> & v, const wml::body & b, coerce_log * log) {
    v.resize(v.size() + 1);
    traits::tag<T>::coerce(v.back(), b.children, log);
  }
};

// Sometimes we need to use recursive wrapper if there are cycles in the AST structure
template <typename T>
struct child_container<std::vector<util::recursive_wrapper<T>>> : std::true_type {
  static bool allows_tag(const wml::body & b) { return b.name == tag<T>::name(); }

  static void insert_tag(std::vector<util::recursive_wrapper<T>> & v, const wml::body & b,
                         coerce_log * log) {
    v.resize(v.size() + 1);
    traits::tag<T>::coerce(v.back().get(), b.children, log);
  }
};

} // end namespace traits

struct all_children_map {
  std::map<std::string, std::vector<wml::body>> children;
};

namespace traits {

template <>
struct child_container<all_children_map> : std::true_type {
  static bool allows_tag(const wml::body &) { return true; }

  static void insert_tag(all_children_map & m, const wml::body & b, coerce_log *) {
    m.children[b.name].emplace_back(b);
  }
};

} // end namespace traits
} // end namespace wml
