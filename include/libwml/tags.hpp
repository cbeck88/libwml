#pragma once

/***
 * Some structures which represent certain kinds of generic tags
 */

#include <libwml/coerce_log.hpp>
#include <libwml/traits/attribute.hpp>
#include <libwml/traits/tag.hpp>
#include <libwml/wml.hpp>

#include <map>
#include <string>
#include <type_traits>
#include <utility>

namespace wml {

struct raw_config : body {};

namespace traits {

template <>
struct tag<raw_config> {
  static constexpr const char * name() { return "config"; }
  static void coerce(raw_config & r, const wml::config & cfg, coerce_log *) { r.children = cfg; }
};

template <typename A>
struct tag<std::map<std::string, A>, std::enable_if_t<traits::attribute<A>::value>> {
  static constexpr const char * name() { return "string-map"; }
  static void coerce(std::map<std::string, A> & m, const wml::config & cfg, coerce_log * log) {
    for (const wml::node & n : cfg) {
      if (const wml::Pair * p = util::get<wml::Pair>(&n)) {
        if (auto maybe_error = traits::attribute<A>::coerce(m[p->first], p->second)) {
          if (log) { log->report_attribute_fail<A>(p->first, p->second, *maybe_error); }
        }
      } else {
        if (log) { log->report_unused(n); }
      }
    }
  }
};

} // end namespace traits
} // end namespace wml
