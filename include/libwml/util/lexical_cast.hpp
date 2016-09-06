#pragma once

#include <libwml/util/optional.hpp>
#include <sstream>
#include <utility>

/***
 * A few simple lexical_cast implementations, similar in spirit to boost lexical cast.
 *
 * Our versions do not throw exceptions, instead they return optional<T>. T is required
 * to be default constructible. We also provide lexical_cast_default, which returns T.
 */

namespace wml {
namespace util {

template <typename T, typename U>
util::optional<T> lexical_cast(const U & u) {
  T t;
  std::stringstream ss;
  if (ss << u && ss >> t) {
    return t;
  } else {
    return {};
  }
}

template <typename T>
util::optional<T> lexical_cast(const std::string & str) {
  T t;
  std::stringstream ss{str};
  if (ss >> t) {
    return t;
  } else {
    std::string err = "bad lexical cast : '";
    err += str;
    err += "'";
    return err.c_str();
  }
}

template <typename T, typename U>
T lexical_cast_default(U && u, T def = T()) {
  T result;
  std::stringstream ss;
  if (ss << std::forward<U>(u) && ss >> result) {
    return result;
  } else {
    return def;
  }
}

} // end namespace util
} // end namespace wml
