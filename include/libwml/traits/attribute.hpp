#pragma once

#include <string>
#include <type_traits>

#include <libwml/wml.hpp>
#include <libwml/util/optional.hpp>

/***
 * The "attribute" trait is used to identify which types represent WML attributes.
 *
 * Specializations of the trait should derive from std::true_type, and should
 * provide a class member function

   `static util::optional<std::string> coerce(T & target, const wml::Str &);`
   `static constexpr const char * debug_name();`

 * `coerce` should coerces the value represented by a `wml::Str` into the target.
 * `debug_name` should report a name for the type appropriate for error messages.
 *
 * It should not fail, instead it should leave the value unchanged, and
 * return a diagnostic message.
 */

namespace wml {
namespace traits {

template <typename T>
struct attribute : std::false_type {};

} // end namespace traits
} // end namespace wml
