#pragma once

/***
 * A mechanism for collecting errors while coercing parsed WML into an AST
 * structure.
 */

#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include <libwml/traits/attribute.hpp>
#include <libwml/traits/tag.hpp>
#include <libwml/wml.hpp>
#include <libwml/util/lexical_cast.hpp>

namespace wml {

struct coerce_incident {
  std::string where;
  std::string what;
  std::string source;

  void write(std::ostream & s) const {
    s << "At: " << this->where << "\n";
    s << "Error: " << this->what << "\n";
    if (this->source.size()) {
      s << "Source: " << this->source << "\n";
    }
    s << std::endl;
  }
};

struct coerce_log {
  std::vector<coerce_incident> incidents_;
  std::vector<std::string> context_;

  void push_context(std::string s) { context_.emplace_back(std::move(s)); }
  void pop_context() { if (context_.size()) { context_.resize(context_.size() - 1); } }

  std::string format_context() const {
    std::string result;
    for (const auto & s : context_) {
      result += "[" + s + "]";
    }
    return result;
  }

  template <typename T>
  void report_attribute_fail(std::string key, const wml::Str & source, std::string diagnostic) {
    coerce_incident i;
    i.where = this->format_context() + " Key: " + std::move(key); // Add line info from wml::Str?
    i.what = "Expected: " + wml::traits::attribute<T>::debug_name() + ".\n      " + std::move(diagnostic);
    i.source = util::lexical_cast_default<std::string>(source);

    incidents_.emplace_back(std::move(i));
  }

  template <typename T>
  void report_child_missing(std::string key, std::string diagnostic) {
    coerce_incident i;
    i.where = this->format_context();
    i.what = "Expected child of type: " + wml::traits::tag<T>::name() + ", with name '" + key + "'.\n      " + std::move(diagnostic);
    i.source = "";

    incidents_.emplace_back(std::move(i));
  }

  void report_unused(const wml::node & n) {
    if (const wml::Pair * p = util::get<wml::Pair>(&n)) {
      this->report_unused_attribute(*p);
    } else if (const wml::body * b = util::get<wml::body>(&n)) {
      this->report_unused_child(*b);
    }
    // TODO: Unused macro instances at config level?
  }

  void report_unused_attribute(const wml::Pair & p) {
    coerce_incident i;
    i.where = this->format_context() + "." + p.first;
    i.what = "Unused attribute. Value: " + util::lexical_cast_default<std::string>(p.second);
    i.source = "";
    incidents_.emplace_back(std::move(i));
  }

  void report_unused_child(const wml::body & b) {
    coerce_incident i;
    i.where = this->format_context();
    i.what = "Unused child tag. [" + b.name + "]";
    i.source = "";

    incidents_.emplace_back(std::move(i));
  }

  void write(std::ostream & s) const {
    for (const auto & i : incidents_) {
      i.write(s);
    }
  }
};

} // end namespace wml
