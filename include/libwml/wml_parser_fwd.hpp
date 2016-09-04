#pragma once

#include <libwml/wml.hpp>
#include <libwml/util/variant.hpp>

#include <string>
#include <utility>

namespace wml {

// Simple, one-off "expected" type for parse function results.
struct parse_result {
  util::variant<wml::parse_error, wml::body> payload_;

  parse_result() = delete;
  parse_result(const parse_result &) = default;
  parse_result(parse_result &&) = default;

  parse_result & operator = (const parse_result &) = default;
  parse_result & operator = (parse_result &&) = default;

  // ctors

  parse_result(wml::body && b) : payload_(std::move(b)) {}
  parse_result(const wml::body & b) : payload_(b) {}

  parse_result(wml::parse_error && e) : payload_(std::move(e)) {}
  parse_result(const wml::parse_error & e) : payload_(e) {}

  // accessors

  explicit operator bool() const { return payload_.which(); }

  wml::body & operator * () & { return *util::get<wml::body>(&payload_); }
  const wml::body & operator * () const & { return *util::get<wml::body>(&payload_); }
  wml::body && operator * () && { return std::move(*util::get<wml::body>(&payload_)); }

  wml::body * operator -> () & { return util::get<wml::body>(&payload_); }
  const wml::body * operator -> () const & { return util::get<wml::body>(&payload_); }

  wml::parse_error & err() & { return *util::get<wml::parse_error>(&payload_); }
  const wml::parse_error & err() const & { return *util::get<wml::parse_error>(&payload_); }
  wml::parse_error && err() && { return std::move(*util::get<wml::parse_error>(&payload_)); }

};

// bool strip_preprocessor(std::string& str);

parse_result parse(const std::string & str);
parse_result parse_document(const std::string & str, const std::string & filename);

// bool parse_attr(const std::string& str);

} // end namespace wml
