#pragma once

#include <libwml/traits/tag.hpp>
#include <libwml/coerce_log.hpp>
#include <libwml/wml.hpp>

namespace wml {

template <typename T>
T coerce(const wml::config & cfg, coerce_log * log = nullptr) {
  static_assert(traits::tag<T>::value, "Target type must be a wml tag");

  T result;
  traits::tag<T>::coerce(result, cfg, log);
  return result;
}

template <typename T>
T coerce(const wml::body & b, coerce_log * log = nullptr) {
  return coerce<T>(b.children, log);
}

} // end namespace wml
