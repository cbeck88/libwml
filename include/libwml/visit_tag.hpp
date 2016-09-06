#pragma once

/***
 * Mechanism that is used to visit the members of a tag in sequence.
 *
 * `visit_tag` applies a visitor to each key-value pair of a tag
 * `visit_tag_extra` supplies extra info: a boolean indiciating if it has an explicit default value
 */

#include <libwml/traits/tag.hpp>
#include <libwml/mpl.hpp>
#include <libwml/tag_member.hpp>
#include <type_traits>
#include <utility>

namespace wml {

template <typename TL>
struct visit_helper;

template <typename... Ts>
struct visit_helper<mpl::typelist<Ts...>> {
  template <typename T, typename V>
  static void apply_visitor(T && t, V && v) {
    int dummy [] = { (Ts::visit(std::forward<V>(v), std::forward<T>(t)), 0)... , 0 };
    static_cast<void>(dummy);
  }

  template <typename T, typename V>
  static void apply_visitor_extra(T && t, V && v) {
    int dummy [] = { (Ts::visit_with_extra_info(std::forward<V>(v), std::forward<T>(t)) , 0)... , 0 };
    static_cast<void>(dummy);
  }
};

template <typename T, typename V>
void visit_tag(T && t, V && v) {
  using tag_t = std::decay_t<T>;
  static_assert(traits::tag<tag_t>::value, "Attempt to visit something which is not a tag");
  using members_list = typename traits::tag<tag_t>::members;
  visit_helper<members_list>::apply_visitor(std::forward<T>(t), std::forward<V>(v));
}

template <typename T, typename V>
void visit_tag_extra(T && t, V && v) {
  using tag_t = std::decay_t<T>;
  static_assert(traits::tag<tag_t>::value, "Attempt to visit something which is not a tag");
  using members_list = typename traits::tag<tag_t>::members;
  visit_helper<members_list>::apply_visitor_extra(std::forward<T>(t), std::forward<V>(v));
}

} // end namespace wml
