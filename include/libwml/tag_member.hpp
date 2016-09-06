#pragma once

/***
 * Mechanism that is used to visit the members of a tag in sequence.
 */

#include <type_traits>
#include <utility>

namespace wml {

template <typename Key, typename Tag, typename Value, Value Tag::* member_ptr, bool has_explicit_default>
struct tag_member_base {
  template <typename V, typename T>
  static void visit(V && visitor, T && tag) {
    std::forward<V>(visitor)(Key::name(), std::forward<T>(tag).*member_ptr);
  }

  template <typename V, typename T>
  static void visit_with_extra_info(V && visitor, T && tag) {
    std::forward<V>(visitor)(Key::name(), std::forward<T>(tag).*member_ptr, has_explicit_default);
  }
};


} // end namespace wml
