#pragma once

/***
 * Some basic typelist facilities
 */

namespace wml {
namespace mpl {

template <typename... Ts>
struct typelist {
  static constexpr unsigned size_t = sizeof...(Ts);
};

} // end namespace mpl
} // end namespace wml
