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
// Note: This means we don't support macros as part of the attribute name...
// otherwise this would have to be std::pair<Str, Str>;
// Fortunately no one seems to be crazy enough to do that :)
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

// A "config" is just a sequence of nodes, same as interior of "body".
using config = std::vector<node>;

// Struct which represents a parse error
struct parse_error {
  std::string position;
  std::string expected_node;
  std::string source;
  std::string context;
};

} // end namespace wml
