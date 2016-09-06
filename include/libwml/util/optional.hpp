#pragma once

#include <boost/optional.hpp>

/***
 * Alias for optional type.
 * Currently we use boost::optional.
 */

namespace wml {
namespace util {

template <typename T>
using optional = boost::optional<T>;

} // end namespace util
} // end namespace wml
