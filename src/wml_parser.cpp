
#include <libwml/wml_parser.hpp>

#include "unit_test.hpp"

#include <iostream>
#include <sstream>

////
// Test Cases
////

namespace wml {

static int test_case_count = 0;

template <typename T, typename U>
bool
test_case(const char * str, T & gram, U & root_gram, bool expected = true, bool with_skip = true) {
  using str_it = std::string::const_iterator;

  std::stringstream foo;

  foo << "Test case: "
      << "'" << str << "' " << ++test_case_count << std::endl;

  static constexpr bool always_show = false;

  std::string storage = str;
  if (storage.at(storage.size() - 1) != '\n' && with_skip) {
    storage += "\n"; // terminate with endl
  }
  str_it iter = storage.begin();
  str_it end = storage.end();

  /*  foo << "-------------------------\n";
    foo << "Test case:\n";
    foo << str << std::endl;
    foo << "-------------------------\n";
  */
  bool r;
  if (with_skip) {
    r = phrase_parse(iter, end, gram, boost::spirit::qi::space_type(),
                     boost::spirit::qi::skip_flag::dont_postskip);
  } else {
    r = parse(iter, end, gram);
  }

  if (r && iter == end) {
    foo << "-------------------------\n";
    foo << "Parsing succeeded (unexpected!)\n";
    foo << "-------------------------\n";
    if (always_show || !expected) { std::cout << foo.str() << std::endl; }
    return expected;
  } else {
    if (always_show || expected) {
      foo << extract_error(root_gram, iter, end);
      std::cout << foo.str() << std::endl;
    }
    return !expected;
  }
}

} // end namespace wml

/*** Tests ***/

/*
bool strip_preprocessor(std::string& input) {
  std::stringstream ss;
  ss.str(input);

  std::string output;

  bool in_define = false;

  int line = 1;
  //std::string line_buffer;
  int define_line = 0;
  int brace_depth = 0;
  char c;

  while (ss) {
    ss.get(c);

    switch (c) {
    case '#': {
      std::string temp;
      getline(ss, temp);
      output += "\n"; // needed for trailing comments
      if (temp.size() >= 6) {
        if (temp.substr(0, 6) == "define") {
          //std::cerr << "DEBUG: got a line with a #define, number '" << line << "':\n'" << temp <<
"'\n";
          if (in_define) {
            std::cerr << "Found #define inside of #define\n";
            std::cerr << "Earlier define was at line " << define_line << "\n";
            std::cerr << "***\n" << output << "\n";
            return false;
          }
          in_define = true;
          define_line = line;
        } else if (temp.substr(0, 6) == "enddef") {
          if (!in_define) {
            std::cerr << "Found #enddef outside of #define\n";
            std::cerr << "***\n" << output << "\n";
            return false;
          }
          in_define = false;
        }
      }
      break;
    }
    case '{': {
      brace_depth++;
      break;
    }
    case '}': {
      if (brace_depth <= 0) {
        std::cerr << "Found unexpected '{'\n";
        std::cerr << "***\n" << output << "\n";
        return false;
      }
      brace_depth--;
      break;
    }
    default: {
      if (!in_define && brace_depth == 0) {
        output += c;
        //line_buffer += c;
      }

      break;
    }
    }
    if (c == '\n') {
      ++line;
      //line_buffer = "";
    }
  }

  // std::cout << "*** Finished stripping preprocessing: ***" << std::endl;
  // std::cout << input << std::endl;
  // std::cout << "----->" << std::endl;
  // std::cout << output << std::endl;
  // std::cout << "***\n" << std::endl;

  input = output;

  return true;
}
*/

namespace wml {

#define TEST_QUOTED_VALUE(STR)                                                                     \
  do {                                                                                             \
    wml::test_case(STR, gram.quoted_value, gram, true, false);                                     \
    wml::test_case("foo = " STR, pair_gram, gram);                                                 \
  } while (0)

UNIT_TEST(wml_grammar) {
  using str_it = std::string::const_iterator;
  using my_grammar = wml::wml_grammar<str_it>;

  pp_database defines;
  my_grammar gram(defines); // Our grammar
  wml::body ast;            // Our tree

  auto pair_gram = gram.pair;

  wml::test_case("a=b", pair_gram, gram);
  wml::test_case("a23=b43", pair_gram, gram);
  wml::test_case("a=", pair_gram, gram);
  wml::test_case("a-asdf=23432", pair_gram, gram, false);
  wml::test_case("a_asdf=23432", pair_gram, gram);
  wml::test_case("a=\"\nfoooooooo\"", pair_gram, gram);
  wml::test_case("a=<<asdf>>\n", pair_gram, gram, true, false);
  wml::test_case("a=asd,fgh\n", pair_gram, gram, true, false);
  wml::test_case("a=asd,fgh,{BAR}\n", pair_gram, gram, true, false);
  wml::test_case("{BAR}\n", gram.value, gram, true, false);
  wml::test_case(",asd,fgh", gram.no_quotes_no_endl_string, gram, true, false);
  wml::test_case(",asd,fgh\n", gram.no_quotes_no_endl_string, gram, false, false);
  wml::test_case(",asd,fgh\n", gram.no_quotes_no_endl_string >> gram.ws_endl, gram, true, false);
  wml::test_case(",asd,fgh\n", gram.no_quotes_no_endl_string >> *gram.ws_weak >> gram.ws_endl, gram,
                 true, false);
  wml::test_case("{BAR},asd,fgh", gram.quoted_value, gram, false);
  wml::test_case("{BAR},asd,fgh", gram.unquoted_value, gram, true, false);
  wml::test_case("{BAR},asd,fgh", gram.unquoted_value >> *gram.ws_weak, gram, true, false);
  wml::test_case("\n", gram.ws_endl, gram, true, false);
  wml::test_case("{BAR},asd,fgh\n", gram.unquoted_value >> *gram.ws_weak >> gram.ws_endl, gram,
                 true, false);
  wml::test_case("{BAR},asd,fgh\n", gram.unquoted_value >> *gram.ws_weak
                                      >> ((&boost::spirit::qi::lit('#')) | gram.ws_endl),
                 gram, true, false);
  wml::test_case("{BAR},asd,fgh\n", gram.value, gram, true, false);
  wml::test_case("a={BAR},asd,fgh\n", pair_gram, gram, true, false);
  wml::test_case("user_team_name=_\"Enemies\"", pair_gram, gram);
  wml::test_case("#foo bar baz\n", gram.nodelist, gram, true, false);
  wml::test_case("# foo bar baz\n", gram.nodelist, gram, true, false);
  wml::test_case("    # foo bar baz\n", gram.nodelist, gram, true, false);
  wml::test_case(" \n\n  #foo bar baz\n", gram.nodelist, gram, true, false);
  wml::test_case("foo=_\"bar\" # blah blah blah\n",
                 pair_gram >> boost::spirit::qi::lit('#') >> gram.pp_comment, gram, true, false);
  wml::test_case("foo=_\"bar\" # blah blah blah\n", gram.nodelist, gram, true, false);
  wml::test_case("foo=bar # blah blah blah\n", gram.nodelist, gram, true, false);

  TEST_QUOTED_VALUE("_ \" bar \"");
  TEST_QUOTED_VALUE("_ \" bar  \n asdf \"");
  TEST_QUOTED_VALUE("_ \" bar \n\n jkl;\"");
  TEST_QUOTED_VALUE("\" bar \" + \"baz\"");
  TEST_QUOTED_VALUE("\" bar \" + _\"baz\"");
  TEST_QUOTED_VALUE("\" bar \n\" + \"baz\"");
  TEST_QUOTED_VALUE("\" bar \n\n\" + \"baz\"");
  TEST_QUOTED_VALUE("\" bar \" +\n\n \"baz\"");
  TEST_QUOTED_VALUE("\" bar \" +\n\n _ \"baz\"");
  TEST_QUOTED_VALUE("\" bar \"\n\n + \"baz\"");

  const char * test_str_1 =
    "difficulty_descriptions={MENU_IMG_TXT2 "
    "\"units/human-loyalists/horseman/horseman.png~RC(magenta>red)\" _\"Horseman\" "
    "_\"(Beginner)\"} +\n     \";\" + {MENU_IMG_TXT2 "
    "\"units/human-loyalists/grand-knight/grand-knight.png~RC(magenta>red)\" _\"Knight\" "
    "_\"(Challenging)\"}";
  wml::test_case(test_str_1, pair_gram, gram);
  wml::test_case(test_str_1, gram.node, gram);

  const char * test_str_2 =
    "    description= _ \"An evil mage is threatening the small village of Maghre and its "
    "inhabitants. The village’s mage sends to his warrior brother for help, but not all goes as "
    "planned. Can you help?\n\n\" + _\"(Novice level, 4 scenarios.)\"";
  wml::test_case(test_str_2, pair_gram, gram);
  wml::test_case("[foo][/foo]\n", gram, gram);
  wml::test_case("[foo][bar][/bar][/foo][baz][/baz]\n", gram, gram, false);
  wml::test_case(
    "\
[foo]\n\
  a=b\n\
  [bar]\n\
    c=d\n\
  [/bar]\n\
[/foo]\n\
[baz]\n\
[/baz]",
    gram, gram, false);

  wml::test_case(
    "[foo]\n\
a = bde4_@342\n\
[bar]\n\
[foo]\n\
[sd]\n\
a= b\n\
[/sd]\n\
[/foo]\n\
[/bar]\n\
[/foo]\n\
",
    gram, gram);

  wml::test_case("[foo]\na=\n[/foo]\n", gram, gram);

  auto node_gram = gram.pair;

  wml::test_case("a=\n", node_gram, gram);

  wml::test_case("[foo]a=b\n[/foo]\n", gram, gram);

  wml::test_case("[foo]x,y=1,2\n[/foo]\n", gram, gram);

  auto check = wml::parse("[foo]x,y=1,2\n[/foo]\n");

  if (!check || !(check->name == "foo") || !(check->children.size() == 2)) {
    std::cerr << "*** Bad parse of multi-assignment in wml node!\n";
  }
}

} // end namespace wml
