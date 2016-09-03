#pragma once

/***
 * In this header we define the grammar elements that are parsed from WML source files.
 *
 * This includes, macros definitions and invocations, strings composed of
 * string data and macro invocations, and wml tags composed of attribute pairs using
 * such strings, WML children, and macro invocations expanding to WML children.
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace wml {

// Preprocessor specific definitions
using pp_sym = std::string;
using pp_param_list = std::vector<pp_sym>;
using pp_macro_decl = std::pair<pp_sym, pp_param_list>;
using pp_macro_body = std::string;
using pp_macro_defn = std::pair<pp_macro_decl, pp_macro_body>;

} // end namespace wml
