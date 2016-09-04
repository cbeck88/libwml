#include "filesystem.hpp"
#include "unit_test.hpp"
#include <libwml/wml_parser.hpp>

#include <iostream>

// test if we can parse a file as wml
int
test_file(const std::string & filename) {
  // bool verbose = false
  if (filename.substr(filename.size() - 4) != ".cfg") { return 0; }

  std::cout << "Scanning file '" << filename << "'" << std::endl;

  std::string storage{filesystem::open(filename)};

  // if (!wml::test_preprocessor(storage)) {
  //   std::cout << "Preprocessor: error parsing file \"" << filename << "\"." << std::endl;
  // }

  if (auto wml_body = wml::parse_document(storage, filename)) {
    static_cast<void>(wml_body);
    // std::cerr << filename << ": PARSED_VALID_WML:" << std::endl;

    // test_ast(*wml_body, verbose);

    return 0;
  } else {
    std::cerr << filename << ": ERROR" << std::endl;
    std::cerr << wml_body.err() << std::endl;
    return 1;
  }
}

///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////

int
test_wml_dir(const std::string & dir_path) {
  std::cout << "Scanning dir '" << dir_path << "'" << std::endl;
  filesystem::visit_directory(dir_path, test_file, test_wml_dir);
  return 0;
}

// Open subdirs of the asset dir and test wml found in those
int
scan_asset_dir(const std::string & asset_dir) {
  std::cout << "Search root: '" << asset_dir << "'" << std::endl;
  filesystem::visit_directory(asset_dir, test_file, test_wml_dir);
  return 0;
}

void
print_usage(const std::string & argv_zero) {
  std::cerr << "Usage:\n\t" << argv_zero << " --test" << std::endl;
  std::cerr << "\t" << argv_zero << " --test" << std::endl;
  std::cerr << "\t" << argv_zero << " file" << std::endl;
  std::cerr << "\t" << argv_zero << " directory" << std::endl;
  std::cerr << std::endl;
}

#define CANT_RUN(X)                                                                                \
  do {                                                                                             \
    std::cerr << X << std::endl << std::endl;                                                      \
    print_usage(argv[0]);                                                                          \
    return 1;                                                                                      \
  } while (0)

int
main(int argc, char ** argv) {
  test::run_tests();

  if (argc > 1) {
    std::string arg{argv[1]};
    if (arg == "--test") { return 0; }
    if (filesystem::exists_file(arg)) {
      return test_file(argv[1]);
    } else if (filesystem::exists_dir(arg)) {
      return scan_asset_dir(arg);
    } else {
      CANT_RUN("Argument '" << arg << "' was not a known flag, file or directory");
    }
  }

  // No arguments. Try to locate asset_path.txt to find assets dir.
  const char * asset_fname = "asset_path.txt";
  if (filesystem::exists_file(asset_fname)) {
    std::string assets_path = filesystem::open(asset_fname);
    if (filesystem::exists_dir(assets_path)) {
      return scan_asset_dir(assets_path);
    } else {
      CANT_RUN("No input file provided, and could not find asset dir: '" << assets_path << "'");
    }
  }

#ifdef EMSCRIPTEN
  const char * asset_dir = "/assets";
  if (filesystem::exists_dir(asset_dir)) {
    return scan_asset_dir(asset_dir);
  } else {
    CANT_RUN("No input file provided, and could not find '" << asset_dir << "'");
  }
#endif

  CANT_RUN("No input file provided, and could not find '" << asset_fname << "'");
}
