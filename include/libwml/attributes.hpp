#pragma once

/***
 * Some implementations of "attribute" trait for primitive values.
 */

#include <ostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <libwml/util/spirit.hpp>
#include <libwml/traits/attribute.hpp>
#include <libwml/util/lexical_cast.hpp>
#include <libwml/util/optional.hpp>

namespace wml {
namespace traits {

template <typename T>
struct lexical_cast_conversion : std::true_type {
  static util::optional<std::string> coerce(T & target, const wml::Str & s) {
    if (auto result = util::lexical_cast<T>(s)) {
      target = *result;
      return {};
    } else {
      return std::string{"lexical cast failed"};
    }
  }
};

template <>
struct attribute<int> : lexical_cast_conversion<int> {
  static constexpr const char * debug_name() { return "integer"; }
};

template <>
struct attribute<unsigned int> : lexical_cast_conversion<unsigned int> {
  static constexpr const char * debug_name() { return "nonnegative integer"; }
};

template <>
struct attribute<float> : lexical_cast_conversion<float> {
  static constexpr const char * debug_name() { return "decimal number"; }
};

template <>
struct attribute<std::string> : lexical_cast_conversion<std::string> {
  static constexpr const char * debug_name() { return "string"; }
};

template <>
struct attribute<bool> : std::true_type {
  static constexpr const char * debug_name() { return "boolean"; }
  static util::optional<std::string> coerce(bool & target, const wml::Str & s) {
    std::string f { util::lexical_cast_default<std::string>(s) };

    if (f == "yes" || f == "on") { target = true; return {}; }
    if (f == "no" || f == "off") { target = false; return {}; }

    return "Legal values are: 'yes', 'no', 'on', 'off'. Found '" + f + "'.";
  }
};

namespace detail {

template <typename Iterator>
struct string_list_grammar : qi::grammar<Iterator, std::vector<std::string>(), qi::space_type> {
	qi::rule<Iterator, std::string(), qi::space_type> str;
	qi::rule<Iterator, std::vector<std::string>(), qi::space_type> main;

	string_list_grammar() : string_list_grammar::base_type(main) {
		str = *(qi::char_ - ',');
		main = str % ',';
	}
};

} // end namespace detail

template <>
struct attribute<std::vector<std::string>> : std::true_type {
  static constexpr const char * debug_name() { return "comma separated list"; }
  static util::optional<std::string> coerce(std::vector<std::string> & target, const wml::Str & s) {
    std::string buffer{util::lexical_cast_default<std::string>(s)};

    using str_it = std::string::const_iterator;
    detail::string_list_grammar<str_it> g;

    std::vector<std::string> result;

    str_it it{buffer.begin()}, end{buffer.end()};
    if(qi::phrase_parse(it, end, g, qi::space, result)) {
      target = std::move(result);
      return {};
    } else {
      return "Stopped parsing at '" + std::string{it, end} + "'";
    }
  }
};

template <>
struct attribute<std::pair<int, int>> : std::true_type {
  static constexpr const char * debug_name() { return "int pair"; }
  static util::optional<std::string> coerce(std::pair<int, int> & target, const wml::Str & s) {
    using temp_t = std::vector<std::string>;
    temp_t temp;
    if (auto maybe_err = attribute<temp_t>::coerce(temp, s)) {
      return maybe_err;
    }
    if (temp.size() != 2) { return "Expected pair, found " + std::to_string(temp.size()) + " elements"; }
    auto maybe_first = util::lexical_cast<int>(temp[0]);
    if (!maybe_first) { return "Expected integer, found '" + temp[0] + "' (first element)"; }
    auto maybe_second = util::lexical_cast<int>(temp[1]);
    if (!maybe_second) { return "Expected integer, found '" + temp[1] + "' (second element)"; }
    target.first = *maybe_first;
    target.second = *maybe_second;
    return {};
  }
};

// Generic optional attributes

template <typename T>
struct attribute<util::optional<T>> : std::true_type {
  // TODO: Does this need to return const char *?
  static std::string debug_name() { return "optional " + attribute<T>::debug_name(); }

  static util::optional<std::string> coerce(util::optional<T> & target, const wml::Str & s) {
    T temp;

    if (auto maybe_err = attribute<T>::coerce(temp, s)) {
      return maybe_err;
    }

    target = std::move(temp);
  }
};

} // end namespace traits
} // end namespace wml
