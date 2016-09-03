#pragma once

#include <cstring>
#include <functional>
#include <sstream>
#include <string>

/***
 * Stripped-down unit test module.
 *
 * To make unit tests, define this header, and use the UNIT_TEST macro, followed by test name, and a
 * function body.
 * Use the various CHECK macros to check test conditions. For instance,

UNIT_TEST(example) {
        CHECK( (1 + 1) == 2, "Why god, why?");
}

 * will display the message "Why god, why?" if the condition fails.
 *
 * Note that the check macros do not throw exceptions. Instead, they call "test::report_failure()"
 * and then return.
 * This is so that we can run them in emscripten without enabling C++ exceptions.
 * We might revisit this in the future, but for now don't use check macros inside of functions which
 * are called by unit tests, because they won't do what you expect.
 */

namespace test {

void log_and_report_failure(const char * fname, int line, std::string);

void report_failure();

typedef std::function<void()> UnitTest;

int register_test(const std::string & name, UnitTest test);

bool run_tests();

} // end namespace test

#define CHECK(cond, msg)                                                                           \
  do {                                                                                             \
    if (!(cond)) {                                                                                 \
      std::ostringstream log_stream_;                                                              \
      log_stream_ << "Assertion " #cond " failed: " << msg;                                        \
      log_and_report_failure(__SHORT_FORM_OF_FILE__, __LINE__, log_stream_.str());                 \
      return;                                                                                      \
    }                                                                                              \
  } while (0)

#define CHECK_TRUE(a) CHECK(a, #a " == true FAILED.")
#define CHECK_FALSE(a) CHECK(!a, #a " == false FAILED.")

#define CHECK_CMP(a, b, cmp) CHECK((a)cmp(b), #a ": " << (a) << "; " #b ": " << (b))

#define CHECK_EQ(a, b) CHECK_CMP(a, b, ==)
#define CHECK_NE(a, b) CHECK_CMP(a, b, !=)
#define CHECK_LE(a, b) CHECK_CMP(a, b, <=)
#define CHECK_GE(a, b) CHECK_CMP(a, b, >=)
#define CHECK_LT(a, b) CHECK_CMP(a, b, <)
#define CHECK_GT(a, b) CHECK_CMP(a, b, >)

#define CHECK_EXPECTED(a) CHECK(a, a.error())

#define UNIT_TEST(name)                                                                            \
  void TEST_##name();                                                                              \
  static int TEST_VAR_##name = test::register_test(#name, TEST_##name);                            \
  void TEST_##name()
