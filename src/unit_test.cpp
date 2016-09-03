#include "unit_test.hpp"

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using uint = unsigned int;

#define LOG_INFO(X)                                                                                \
  do {                                                                                             \
    std::cerr << "TEST: " << X << std::endl;                                                       \
  } while (0)

// LOG_CHANNEL(test);

namespace test {

namespace {

typedef std::map<std::string, UnitTest> TestMap;
TestMap &
get_test_map() {
  static TestMap map;
  return map;
}

static bool test_failed;

} // end anonymous namespace

int
register_test(const std::string & name, UnitTest test) {
  get_test_map()[name] = test;
  return 0;
}

void
log_and_report_failure(const char * fname, int line, std::string msg) {
  std::cerr << '[' << fname << ':' << line << "] " << msg << std::endl;
  report_failure();
}

void
report_failure() {
  test_failed = true;
}

bool
run_tests() {
  // uint32_t start_time = SDL_GetTicks();

  std::vector<std::string> all_tests;
  for (const auto & v : get_test_map()) {
    all_tests.push_back(v.first);
  }

  uint npass = 0, nfail = 0;
  for (const auto & name : all_tests) {
    test_failed = false;
    get_test_map()[name]();
    if (!test_failed) {
      ++npass;
      LOG_INFO("TEST " << name << " PASSED.");
    } else {
      ++nfail;
      LOG_INFO("TEST " << name << " FAILED!");
    }
  }

  if (nfail) {
    LOG_INFO(" " << npass << " TESTS PASSED, " << nfail << " TESTS FAILED");
    return false;
  } else {
    // LOG_INFO("ALL " << npass << " TESTS PASSED IN " << (SDL_GetTicks() - start_time) << "ms");
    return true;
  }
}

} // end namespace test
