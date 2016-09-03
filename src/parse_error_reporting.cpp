#include "parse_error_reporting.hpp"

#include <iostream>
#include <string>

namespace wml {

void
report_errors(const std::string & context, const std::string & errors) {
  std::cout << "-------------------------\n";
  std::cout << "Parsing failed\n";
  std::cout << "stopped at: \": " << context << "...\"\n";
  std::cout << "-------------------------\n";
  std::cout << "Error log:\n";
  std::cout << errors << std::endl;
  std::cout << "-------------------------" << std::endl;
}

} // end namespace wml
