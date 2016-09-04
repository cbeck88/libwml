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
#include <libwml/wml.hpp>
#include <libwml/wml_pp.hpp>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace wml {

struct stringizer {
  std::string operator()(const Key & k) const { return '"' + k + "\" "; }
  std::string operator()(const MacroInstance & i) const { return '{' + i.text + "} "; }
};

inline std::ostream &
operator<<(std::ostream & ss, const TextVariant & v) {
  ss << util::apply_visitor(stringizer{}, v);
  return ss;
}

inline std::ostream &
operator<<(std::ostream & ss, const Str & vec) {
  for (const auto & v : vec) {
    ss << v;
  }
  return ss;
}

///////////////////////////////////////////////////////////////////////////
//  Print out the wml tree
///////////////////////////////////////////////////////////////////////////

constexpr unsigned int tabsize = 4;
inline std::string
tab(unsigned int indent) {
  return std::string(indent, ' ');
}

struct body_printer {
  body_printer(std::ostream & s, unsigned int indent = 0)
    : ss(s)
    , indent(indent) {}

  inline void operator()(body const &) const;

  std::ostream & ss;
  unsigned int indent;
};

struct node_printer {
  node_printer(std::ostream & s, unsigned int indent = 0)
    : ss(s)
    , indent(indent) {}

  void operator()(body const & w) const { body_printer(ss, indent + tabsize)(w); }

  void operator()(Str const & text) const {
    ss << tab(indent + tabsize) << "text: \"" << text << '"' << std::endl;
  }

  void operator()(Pair const & p) const {
    ss << tab(indent + tabsize) << p.first << ": \"" << p.second << "\"" << std::endl;
  }

  void operator()(const MacroInstance & m) const {
    ss << tab(indent + tabsize) << "macro: \"" << m.text << '"' << std::endl;
  }

  std::ostream & ss;
  unsigned int indent;
};

inline void
body_printer::operator()(body const & w) const {
  ss << tab(indent) << "tag: \"" << w.name << "\""
     << "( " << w.children.size() << " children )" << std::endl;
  ss << tab(indent) << '{' << std::endl;

  for (node const & n : w.children) {
    util::apply_visitor(node_printer(ss, indent), n);
  }

  ss << tab(indent) << '}' << std::endl;
}

struct config_printer {
  config_printer(std::ostream & s, unsigned int indent = 0)
    : ss(s)
    , indent(indent) {}

  void operator()(config const & c) const {
    ss << tab(indent) << '{' << std::endl;

    for (node const & n : c) {
      util::apply_visitor(node_printer(ss, indent), n);
    }

    ss << tab(indent) << '}' << std::endl;
  }

  std::ostream & ss;
  unsigned int indent;
};

inline std::ostream &
operator<<(std::ostream & ss, const parse_error & e) {
  ss << "-------------------------\n";
  if (e.context.size()) {
    ss << "Parsing failed\n";
    ss << "stopped at: \": " << e.context << "...\"\n";
    ss << "-------------------------\n";
  }

  ss << "Error at position: " << e.position << "\n"
     << "Expected a node of type '" << e.expected_node << "'\n"
     << "--- Source Lines ---\n"
     << e.source << "\n";

  ss << "-------------------------\n";
  return ss;
}

} // end namespace wml
