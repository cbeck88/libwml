#pragma once

/***
 * In this header we define the grammar elements that are parsed from WML source files.
 *
 * This includes, macros definitions and invocations, strings composed of
 * string data and macro invocations, and wml tags composed of attribute pairs using
 * such strings, WML children, and macro invocations expanding to WML children.
 */

#pragma once

#include <libwml/util/variant.hpp>
#include <libwml/wml_pp.hpp>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace wml {

using Key = std::string;

struct MacroInstance {
  Key text;
};

using TextVariant = util::variant<Key, MacroInstance>;

// A wml::Str is text interspersed with macro instances
using Str = std::vector<TextVariant>;
using Pair = std::pair<Key, Str>;

///////////////////////////////////////////////////////////////////////////
//  Our WML tree representation
///////////////////////////////////////////////////////////////////////////
struct body;

using node = util::variant<util::recursive_wrapper<body>, Pair, MacroInstance>;

struct body {
  Key name;                   // tag name
  std::vector<node> children; // children
};

typedef std::vector<node> config;

} // end namespace wml
