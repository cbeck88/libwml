#pragma once

#include <type_traits>

/***
 * The "tag" trait is used to identify which structs represents WML tags
 *
 * Specializations of the trait should derive from std::true_type, and should
 * provide a `static constexpr const char * name` class member which indicates
 * the tag name, as it appears in wml.
 */

namespace wml {
namespace traits {

template <typename T, typename ENABLE = void>
struct tag : std::false_type {};

} // end namespace traits
} // end namespace wml
