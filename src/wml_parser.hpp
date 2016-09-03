#pragma once

#include <libwml/wml.hpp>

#include <boost/optional.hpp>
#include <string>

namespace wml {
// bool strip_preprocessor(std::string& str);

boost::optional<wml::body> parse(const std::string & str);
boost::optional<wml::body> parse_document(const std::string & str, const std::string & filename);

// bool parse_attr(const std::string& str);

} // end namespace wml
