#pragma once

#include <type_traits>

/***
 * The "child_container" trait is used to identify which types represent containers of WML tags.
 *
 * Specializations of the trait should derive from std::true_type, and should
 * provide class member functions

   `static bool allows_tag(const wml::body &)`;
   `static void insert_tag(T & target, const wml::body &, coerce_log *);`

 * `allows_tag` should check the name member of the body, and report if this tag can
 * be held by this container.
 * `insert_tag` should insert the wml body into the container, and coerce it as necessary,
 * It should write errors to the coerce log.
 *
 * Neither of these functions should fail.
 */

namespace wml {
namespace traits {

template <typename T>
struct child_container : std::false_type {};

} // end namespace traits
} // end namespace wml
