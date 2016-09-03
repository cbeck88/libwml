#pragma once

#include <boost/optional.hpp>
#include <boost/spirit/include/qi.hpp>
#include <string>
#include <vector>

// In Wesnoth, version numbers are often represented as e.g. "1.10", "1.11+dev", etc.
// version_number struct is able to represent such versions, and sort them, where,
// a + or - at the end affects the sorting properly.

struct version_number {
  std::vector<int> nums;
  char version_separator;
  std::string modifier;

  int version_separator_value() const {
    if (version_separator == '+') {
      return 1;
    } else if (version_separator == '-') {
      return -1;
    }
    return 0;
  }

  bool operator==(const version_number & other) const {
    return (*this <= other) && (other <= *this);
  }
  bool operator<(const version_number & other) const { return !(other <= *this); }
  bool operator<=(const version_number & other) const {
    for (size_t i = 0; i < nums.size(); ++i) {
      if (nums[i] > other.nums[i]) { return false; }
      if (nums[i] < other.nums[i]) { return true; }
    }
    if (other.nums.size() > nums.size()) { return true; }
    if (version_separator_value() < other.version_separator_value()) { return true; }
    if (version_separator_value() > other.version_separator_value()) { return false; }
    return (modifier <= other.modifier);
  }
  bool operator!=(const version_number & other) const { return !(*this == other); }
  bool operator>(const version_number & other) const { return other < *this; }
  bool operator>=(const version_number & other) const { return other <= *this; }
};

boost::optional<version_number>
parse_version(const std::string & str) {
  using str_ti = std::string::const_iterator;

  namespace qi = boost::spirit::qi;

  str_it iter = str.begin();
  str_it end = str.end();
  std::vector<int> temp;
  qi::parse(iter, end, qi::int_ % qi::char_('.'), temp);

  for (int i : temp) {
    if (i < 0) { return boost::none; }
  }
  while (temp.size() && (temp.back() == 0)) {
    temp.resize(temp.size() - 1);
  }
  if (!temp.size()) { return boost::none; }

  std::string string_part{iter, end};
  char separator = '\0';
  if (string_part.size() && !isalpha(string_part[0])) {
    separator = string_part[0];
    string_part = std::string{iter + 1, end};
  }

  return version_number{std::move(temp), separator, std::move(string_part)};
}
