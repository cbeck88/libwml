///
// A WML parser using boost spirit2.
// Based heavily on http://www.boost.org/doc/libs/1_47_0/libs/spirit/example/qi/mini_xml2.cpp
///

#define BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_UNICODE

// #include <libwml/log.hpp>
#include "parse_error_reporting.hpp"
#include "unit_test.hpp"

#include <libwml/wml.hpp>
#include <libwml/wml_stream_ops.hpp>

#include <boost/config/warning_disable.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// AST adaptors for MAIN wml grammar object

// We need to tell fusion about our wml struct
// to make it a first-class fusion citizen
BOOST_FUSION_ADAPT_STRUCT(wml::body, (wml::Key, name)(std::vector<wml::node>, children))
BOOST_FUSION_ADAPT_STRUCT(wml::MacroInstance, (wml::Key, text))

// Helper function to adapt multi-assignment lines in wml syntax

typedef std::vector<wml::Key> StrList;
typedef std::pair<StrList, StrList> StrListPair;

inline std::vector<wml::node>
unfold_pairs_one(const StrListPair & pl) {
  std::vector<wml::node> result;
  unsigned n = std::min(pl.first.size(), pl.second.size());
  for (unsigned i = 0; i < n; ++i) {
    result.push_back(std::make_pair(pl.first[i], wml::Str{pl.second[i]}));
  }
  return result;
}

namespace wml {
namespace fusion = boost::fusion;
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;

///////////////////////////////////////////////////////////////////////////
//  Our WML grammar definition
///////////////////////////////////////////////////////////////////////////

static std::ostream * errbuf = 0;

BOOST_PHOENIX_ADAPT_FUNCTION(std::vector<wml::node>, unfold_pairs_, unfold_pairs_one, 1);

template <typename iterator>
void
report_error(const std::string & position, const iterator & it, const iterator & end,
             const boost::spirit::info & expected_node) {
  std::cerr << "Encountered an error:\n";
  (errbuf ? *errbuf : std::cerr) << "Error at position " << position << "! Expecting "
                                 << expected_node << " here: \"" << std::string(it, end) << "\""
                                 << std::endl;
}

BOOST_PHOENIX_ADAPT_FUNCTION(void, report_error_, report_error, 4);

#define RULE_DEBUGGING(VAR)                                                                        \
  do {                                                                                             \
    VAR.name(BOOST_PP_STRINGIZE(VAR));                                                             \
    on_error<fail>(VAR,                                                                            \
                   report_error_(wml_cursor_(ref(preprocessor_data)), qi::_3, qi::_2, qi::_4));    \
  } while (0)

struct pp_cursor {
  std::string file;
  int line;

  pp_cursor(const std::string & f)
    : file(f)
    , line(1) {}
};

inline std::ostream &
operator<<(std::ostream & ss, const pp_cursor & c) {
  ss << "[" << c.file << "," << c.line << "]" << std::endl;
  return ss;
}

struct pp_macro {
  pp_sym name;
  pp_param_list arguments;
  pp_macro_body lines;
  pp_cursor cursor;

  pp_macro(const pp_sym & n, const pp_param_list & a, const pp_macro_body & b, const pp_cursor & c)
    : name(n)
    , arguments(a)
    , lines(b)
    , cursor(c) {}
};

struct pp_database {
  std::unordered_map<pp_sym, std::shared_ptr<pp_macro>> pp_macros;
  std::stack<pp_cursor> current_file;
  std::string current_directory;

  pp_database(const std::string & file = "root", const std::string & dir = "~")
    : pp_macros()
    , current_file({pp_cursor{file}})
    , current_directory(dir) {}
};

inline void
wml_new_line(pp_database & d) {
  ++d.current_file.top().line;
}

inline void
wml_add_define(const pp_macro_defn & m, pp_database & d, bool verbose = false) {
  const auto & name = m.first.first;
  const auto & args = m.first.second;
  const auto & body = m.second;

  d.pp_macros.emplace(name, std::make_shared<pp_macro>(name, args, body, d.current_file.top()));
  // TODO: issue warning if insertion fails

  if (!verbose) { return; }

  std::stringstream ss_args;
  ss_args << "Macro Parameters: (" << args.size() << ") ";
  for (const auto & a : args) {
    ss_args << ", \"" << a << "\"";
  }

  std::stringstream ss_body;
  /*  for (const auto & b : body) {
      ss_body << b << std::endl;
    }*/
  ss_body << body << "\n";
  // LOG_WARN_ON(LOG::preprocessor, "Adding #define " << name << " at line # " <<
  // d.current_file.top().line << "\n" << ss_args.str() << "\n" << ss_body.str());
}

void
wml_remove_define(const pp_sym & s, pp_database & d, bool verbose = false) {
  d.pp_macros.erase(s);
  // TODO: issue warning if deletion fails

  if (!verbose) { return; }
  // LOG_WARN_ON(LOG::preprocessor, "Removing #define " << s << " at line # " <<
  // d.current_file.top().line);
}

std::string
wml_current_cursor(pp_database & d) {
  std::stringstream ss;
  ss << d.current_file.top();
  return ss.str();
}

BOOST_PHOENIX_ADAPT_FUNCTION(void, wml_new_line_, wml_new_line, 1);
BOOST_PHOENIX_ADAPT_FUNCTION(void, wml_add_define_, wml_add_define, 2);
BOOST_PHOENIX_ADAPT_FUNCTION(void, wml_remove_define_, wml_remove_define, 2);
BOOST_PHOENIX_ADAPT_FUNCTION(std::string, wml_cursor_, wml_current_cursor, 1);

template <typename Iterator>
struct wml_grammar : qi::grammar<Iterator, body(), qi::locals<Key>> {
  qi::rule<Iterator, wml::body(), qi::locals<Key>> wml;
  qi::rule<Iterator, wml::node()> node;
  qi::rule<Iterator, Key()> start_tag;
  qi::rule<Iterator, void(Key)> end_tag;
  qi::rule<Iterator, Pair()> pair;
  qi::rule<Iterator, Key()> key;
  qi::rule<Iterator, Str()> value;
  qi::rule<Iterator, Str()> quoted_value;
  qi::rule<Iterator, Str()> unquoted_value;
  qi::rule<Iterator, StrList()> keylist;
  qi::rule<Iterator, StrList()> valuelist;
  qi::rule<Iterator, StrListPair()> pairlist;
  qi::rule<Iterator, std::vector<wml::node>()> pairs;
  qi::rule<Iterator, std::vector<wml::node>()> nodelist;
  qi::rule<Iterator, std::vector<wml::node>()> config;
  qi::rule<Iterator, Key()> double_quoted_string;
  qi::rule<Iterator, Key()> angle_quoted_string;
  qi::rule<Iterator, Key()> no_quotes_no_endl_string;
  qi::rule<Iterator, Key()> no_comma_no_quotes_no_endl_string;

  qi::rule<Iterator> pp_ignore_line;

  qi::rule<Iterator, pp_sym()> pp_symbol;
  qi::rule<Iterator, pp_macro_decl()> pp_define_declaration;
  qi::rule<Iterator, pp_macro_body()> pp_define_body;
  qi::rule<Iterator, pp_macro_defn()> pp_define;
  qi::rule<Iterator> pp_enddef;
  qi::rule<Iterator, pp_sym()> pp_undef;

  qi::rule<Iterator, Key()> pp_macro_instance_str;
  qi::rule<Iterator, wml::MacroInstance()> pp_macro_instance;

  qi::rule<Iterator> pp_if;
  qi::rule<Iterator> pp_else;
  qi::rule<Iterator> pp_endif;
  qi::rule<Iterator> pp_comment;
  qi::rule<Iterator> pp_error;
  qi::rule<Iterator> pp_warning;
  qi::rule<Iterator> pp_directive;

  qi::rule<Iterator, qi::unused_type()> ws_weak, ws_endl, ws_all, ws_skip_to_eol, ws_consume_to_eol;

  wml_grammar(pp_database & preprocessor_data)
    : wml_grammar::base_type(wml, "wml") {
    using qi::lit;
    using qi::lexeme;
    using qi::on_error;
    using qi::fail;
    using boost::spirit::unicode::char_;
    using boost::spirit::unicode::string;
    using namespace qi::labels;

    using phoenix::construct;
    using phoenix::ref;
    using phoenix::val;

    ws_weak = char_(" \t\r");
    ws_endl = char_("\n")[wml_new_line_(ref(preprocessor_data))];
    ws_all = *(ws_weak | ws_endl);
    ws_skip_to_eol = *ws_weak >> -ws_endl;
    ws_consume_to_eol = *ws_weak >> ((&lit('#')) | ws_endl);

    pair = ws_all >> key >> *ws_weak >> lit('=') >> value >> ws_skip_to_eol;
    key = char_("a-zA-Z_") >> *char_("a-zA-Z_0-9");
    quoted_value = (*ws_weak >> -lit('_') >> *ws_weak
                    >> (pp_macro_instance | angle_quoted_string | double_quoted_string))
                   % (ws_all >> -lit('+') >> ws_all);
    unquoted_value = *ws_weak >> *(pp_macro_instance | no_quotes_no_endl_string);
    value = ((quoted_value >> ws_consume_to_eol) | (unquoted_value >> ws_consume_to_eol));

    keylist = (*ws_weak >> key) % (*ws_weak >> lit(","));
    valuelist = (*ws_weak >> no_comma_no_quotes_no_endl_string) % (*ws_weak >> lit(","));
    pairlist =
      (ws_all >> keylist >> *ws_weak) >> lit('=')
      >> (valuelist >> ws_skip_to_eol); // *ws_weak >> ws_endl (?) this prevents some things
    pairs = pairlist[_val = unfold_pairs_(_1)];

    angle_quoted_string = (lit("<<") >> *(char_ - ">>")) >> lit(">>");
    double_quoted_string = (lit('"') >> *(char_ - '"')) >> lit('"');
    no_quotes_no_endl_string = +(char_ - char_("\n\"{#") - "<<");
    no_comma_no_quotes_no_endl_string = +(char_ - char_(",\n\"{#") - "<<");

    pp_symbol = +(char_ - char_("{} \n\t\r"));
    pp_ignore_line = *(char_ - char_("\n")) >> ws_endl;

    pp_enddef = lit("#enddef") >> pp_ignore_line;
    pp_define_declaration = +ws_weak > pp_symbol >> *(+ws_weak >> pp_symbol) >> pp_ignore_line;
    pp_define_body = *((!pp_enddef >> lit('#') >> pp_comment) | (!lit('#') >> char_)) > pp_enddef;
    pp_define = lit("define") > pp_define_declaration > pp_define_body;
    pp_undef = lit("undef") >> +ws_weak > pp_symbol >> pp_ignore_line;
    pp_directive = (pp_define[wml_add_define_(_1, ref(preprocessor_data))])
                   | (pp_undef[wml_remove_define_(_1, ref(preprocessor_data))]) | pp_error
                   | pp_warning;
    pp_comment = pp_ignore_line;
    pp_error = lit("error") >> pp_ignore_line;
    pp_warning = lit("warning") >> pp_ignore_line;
    pp_else = lit("else") >> pp_ignore_line;
    pp_endif = lit("endif") >> pp_ignore_line;
    pp_if =
      (lit("ifver") | lit("ifnver") | lit("ifhave") | lit("ifnhave") | lit("ifdef") | lit("ifndef"))
      >> +ws_weak > pp_symbol >> pp_ignore_line
      >> *(!(lit('#') >> pp_else) >> !(lit('#') >> pp_endif) >> pp_ignore_line); // <-- hack

    pp_macro_instance_str =
      lit('{') >> *(+(char_ - char_("{}")) | pp_macro_instance_str) >> lit('}');
    pp_macro_instance %= pp_macro_instance_str;

    node = (wml | pair);
    nodelist %= ws_all >> ((lit('#') > (pp_directive | pp_if | pp_comment) >> -nodelist)
                           | (pp_macro_instance >> ws_skip_to_eol >> -nodelist)
                           | (node >> -nodelist) | (pairs >> -nodelist));
    config %= -nodelist;

    start_tag %= ws_all >> (lit('[') >> !lit('/') >> -lit('+') >> +(char_ - ']')) >> lit(']');

    end_tag = ws_all >> lit("[/") >> string(_r1) >> lit(']');

    // removed some > -> >> here (above and below)

    wml %= ws_all >> start_tag[_a = _1] >> ws_all >> config > end_tag(_a) >> ws_skip_to_eol;

    RULE_DEBUGGING(wml);
    RULE_DEBUGGING(node);
    RULE_DEBUGGING(nodelist);
    RULE_DEBUGGING(config);
    RULE_DEBUGGING(start_tag);
    RULE_DEBUGGING(end_tag);
    RULE_DEBUGGING(key);
    RULE_DEBUGGING(keylist);
    RULE_DEBUGGING(value);
    RULE_DEBUGGING(valuelist);
    RULE_DEBUGGING(pair);
    RULE_DEBUGGING(pairlist);
    RULE_DEBUGGING(pairs);
    RULE_DEBUGGING(quoted_value);
    RULE_DEBUGGING(unquoted_value);
    RULE_DEBUGGING(angle_quoted_string);
    RULE_DEBUGGING(double_quoted_string);
    RULE_DEBUGGING(no_quotes_no_endl_string);
    RULE_DEBUGGING(no_comma_no_quotes_no_endl_string);

    RULE_DEBUGGING(ws_weak);
    RULE_DEBUGGING(ws_endl);
    RULE_DEBUGGING(ws_all);
    RULE_DEBUGGING(ws_skip_to_eol);
    RULE_DEBUGGING(ws_consume_to_eol);

    RULE_DEBUGGING(pp_symbol);
    RULE_DEBUGGING(pp_ignore_line);
    RULE_DEBUGGING(pp_directive);
    RULE_DEBUGGING(pp_define);
    RULE_DEBUGGING(pp_define_declaration);
    RULE_DEBUGGING(pp_define_body);
    RULE_DEBUGGING(pp_enddef);
    RULE_DEBUGGING(pp_undef);
    RULE_DEBUGGING(pp_comment);
    RULE_DEBUGGING(pp_error);
    RULE_DEBUGGING(pp_warning);
    RULE_DEBUGGING(pp_if);
    RULE_DEBUGGING(pp_else);
    RULE_DEBUGGING(pp_endif);
    RULE_DEBUGGING(pp_macro_instance_str);
    RULE_DEBUGGING(pp_macro_instance);
  }
};

#undef RULE_DEBUGGING

} // end namespace wml

////
// Test Cases
////

namespace wml {

static int test_case_count = 0;

template <typename T>
bool
test_case(const char * str, T & gram, bool expected = true, bool with_skip = true) {
  using str_it = std::string::const_iterator;

  std::ostream * old_err_buf = wml::errbuf;
  std::stringstream foo;
  wml::errbuf = &foo;

  foo << "Test case: "
      << "'" << str << "' " << ++test_case_count << std::endl;

  static bool always_show = false;

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
    wml::errbuf = old_err_buf;
    return expected;
  } else {
    str_it some = iter + 80;
    std::string context(iter, (some > end) ? end : some);
    if (always_show || expected) { report_errors(context, foo.str()); }
    wml::errbuf = old_err_buf;
    return !expected;
  }
}

} // end namespace wml

///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////

namespace wml {

boost::optional<wml::body>
parse(const std::string & sto) {
  std::string storage = sto;
  if (storage.at(storage.size() - 1) != '\n') {
    storage += "\n"; // terminate with endl
  }

  using str_it = std::string::const_iterator;
  using my_grammar = wml_grammar<str_it>;
  pp_database defines;
  my_grammar gram(defines); // Our grammar
  wml::body ast;            // Our tree

  std::stringstream errors;
  wml::errbuf = &errors;

  str_it iter = storage.begin();
  str_it end = storage.end();
  bool r = parse(iter, end, gram, ast);

  wml::errbuf = NULL;

  if (r && iter == end) {
    /*std::cout << "-------------------------\n";
    std::cout << "Parsing succeeded\n";
    std::cout << "-------------------------\n";
    body_printer printer(std::cout);
    printer(ast);*/
    return ast;
  } else {
    str_it some = iter + 80;
    std::string context(iter, (some > end) ? end : some);
    report_errors(context, errors.str());
    return boost::none;
  }
}

boost::optional<wml::body>
parse_document(const std::string & sto, const std::string & filename) {
  std::string storage = sto;
  if (storage.at(storage.size() - 1) != '\n') {
    storage += "\n"; // terminate with endl
  }

  using str_it = std::string::const_iterator;
  using my_grammar = wml_grammar<str_it>;

  pp_database defines(filename);
  my_grammar gram(defines); // Our grammar

  wml::config result; // our result (vector of nodes)

  std::stringstream errors;
  wml::errbuf = &errors;

  str_it iter = storage.begin();
  str_it end = storage.end();

  if (!parse(iter, end, gram.config, result)) {
    str_it some = iter + 80;
    std::string context(iter, (some > end) ? end : some);
    report_errors(context, errors.str());
    wml::errbuf = nullptr;
    return boost::none;
  }
  errors.clear();

  wml::errbuf = nullptr;

  wml::body document;
  document.name = "root";
  document.children = std::move(result);

  return document;
}

inline bool
parse_attr(const std::string & sto) {
  std::string storage = sto;
  if (sto.at(sto.size() - 1) != '\n') {
    storage += "\n"; // terminate with endl
  }

  using str_it = std::string::const_iterator;
  using my_grammar = wml_grammar<str_it>;

  pp_database defines;
  my_grammar grammar(defines);
  auto gram = grammar.pair;
  wml::Pair ast; // Our tree

  str_it iter = storage.begin();
  str_it end = storage.end();
  bool r = phrase_parse(iter, end, gram, boost::spirit::qi::space, ast);

  if (r && iter == end) {
    std::cout << "-------------------------\n";
    std::cout << "Parsing succeeded\n";
    std::cout << "-------------------------\n";
    std::cout << "first: '" << ast.first << "'\n";
    std::cout << "second: '" << ast.second << "'\n";
    return true;
  } else {
    str_it some = iter + 80;
    std::string context(iter, (some > end) ? end : some);
    std::cout << "-------------------------\n";
    std::cout << "Parsing failed\n";
    std::cout << "stopped at: \": " << context << "...\"\n";
    std::cout << "-------------------------\n";
    return false;
  }
}

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

#define TEST_QUOTED_VALUE(STR)                                                                     \
  do {                                                                                             \
    wml::test_case(STR, gram.quoted_value, true, false);                                           \
    wml::test_case("foo = " STR, pair_gram);                                                       \
  } while (0)

UNIT_TEST(wml_grammar) {
  using str_it = std::string::const_iterator;
  using my_grammar = wml::wml_grammar<str_it>;

  pp_database defines;
  my_grammar gram(defines); // Our grammar
  wml::body ast;            // Our tree

  auto pair_gram = gram.pair;

  wml::test_case("a=b", pair_gram);
  wml::test_case("a23=b43", pair_gram);
  wml::test_case("a=", pair_gram);
  wml::test_case("a-asdf=23432", pair_gram, false);
  wml::test_case("a_asdf=23432", pair_gram);
  wml::test_case("a=\"\nfoooooooo\"", pair_gram);
  wml::test_case("a=<<asdf>>\n", pair_gram, true, false);
  wml::test_case("a=asd,fgh\n", pair_gram, true, false);
  wml::test_case("a=asd,fgh,{BAR}\n", pair_gram, true, false);
  wml::test_case("{BAR}\n", gram.value, true, false);
  wml::test_case(",asd,fgh", gram.no_quotes_no_endl_string, true, false);
  wml::test_case(",asd,fgh\n", gram.no_quotes_no_endl_string, false, false);
  wml::test_case(",asd,fgh\n", gram.no_quotes_no_endl_string >> gram.ws_endl, true, false);
  wml::test_case(",asd,fgh\n", gram.no_quotes_no_endl_string >> *gram.ws_weak >> gram.ws_endl, true,
                 false);
  wml::test_case("{BAR},asd,fgh", gram.quoted_value, false);
  wml::test_case("{BAR},asd,fgh", gram.unquoted_value, true, false);
  wml::test_case("{BAR},asd,fgh", gram.unquoted_value >> *gram.ws_weak, true, false);
  wml::test_case("\n", gram.ws_endl, true, false);
  wml::test_case("{BAR},asd,fgh\n", gram.unquoted_value >> *gram.ws_weak >> gram.ws_endl, true,
                 false);
  wml::test_case("{BAR},asd,fgh\n", gram.unquoted_value >> *gram.ws_weak
                                      >> ((&boost::spirit::qi::lit('#')) | gram.ws_endl),
                 true, false);
  wml::test_case("{BAR},asd,fgh\n", gram.value, true, false);
  wml::test_case("a={BAR},asd,fgh\n", pair_gram, true, false);
  wml::test_case("user_team_name=_\"Enemies\"", pair_gram);
  wml::test_case("#foo bar baz\n", gram.nodelist, true, false);
  wml::test_case("# foo bar baz\n", gram.nodelist, true, false);
  wml::test_case("    # foo bar baz\n", gram.nodelist, true, false);
  wml::test_case(" \n\n  #foo bar baz\n", gram.nodelist, true, false);
  wml::test_case("foo=_\"bar\" # blah blah blah\n",
                 pair_gram >> boost::spirit::qi::lit('#') >> gram.pp_comment, true, false);
  wml::test_case("foo=_\"bar\" # blah blah blah\n", gram.nodelist, true, false);
  wml::test_case("foo=bar # blah blah blah\n", gram.nodelist, true, false);

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
  wml::test_case(test_str_1, pair_gram);
  wml::test_case(test_str_1, gram.node);

  const char * test_str_2 =
    "    description= _ \"An evil mage is threatening the small village of Maghre and its "
    "inhabitants. The village’s mage sends to his warrior brother for help, but not all goes as "
    "planned. Can you help?\n\n\" + _\"(Novice level, 4 scenarios.)\"";
  wml::test_case(test_str_2, pair_gram);
  wml::test_case("[foo][/foo]\n", gram);
  wml::test_case("[foo][bar][/bar][/foo][baz][/baz]\n", gram, false);
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
    gram, false);

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
    gram);

  wml::test_case("[foo]\na=\n[/foo]\n", gram);

  auto node_gram = gram.pair;

  wml::test_case("a=\n", node_gram);

  wml::test_case("[foo]a=b\n[/foo]\n", gram);

  wml::test_case("[foo]x,y=1,2\n[/foo]\n", gram);

  auto check = wml::parse("[foo]x,y=1,2\n[/foo]\n");

  if (!check || !(check->name == "foo") || !(check->children.size() == 2)) {
    std::cerr << "*** Bad parse of multi-assignment in wml node!\n";
  }
}

} // end namespace wml

#undef RULE_DEBUGGING
