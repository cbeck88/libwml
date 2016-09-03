#pragma once

#include <fstream>
#include <iterator>
#include <string>
#include <utility>

#include <cassert>
#include <cstdio>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

/***
 * Minimalistic filesystem api, for unix and windows
 * Single header
 */

namespace filesystem {

inline std::string
open(const std::string & filename) {
  std::ifstream in(filename, std::ios_base::in);

  if (!in) {
    fprintf(stderr, "Could not open input file: %s", filename.c_str());
    assert(false && "Failed to open a file");
  }

  std::string storage;         // We will read the contents here.
  in.unsetf(std::ios::skipws); // No white space skipping!
  std::copy(std::istream_iterator<char>(in), std::istream_iterator<char>(),
            std::back_inserter(storage));
  return storage;
}

inline bool
exists_file(const std::string & path) {
  struct stat path_stat;
  stat(path.c_str(), &path_stat);
  return S_ISREG(path_stat.st_mode);
}

inline bool
exists_dir(const std::string & path) {
  struct stat path_stat;
  stat(path.c_str(), &path_stat);
  return S_ISDIR(path_stat.st_mode);
}

#ifndef WIN32
#define PATH_SEPARATOR "/"
#else
#define PATH_SEPARATOR "\\"
#endif

inline std::string
join_path(const std::string & dir, const std::string & file) {
  std::string ret = dir;
  char last_c = ret[ret.length() - 1];
  if (last_c != '/' && last_c != '\\') { ret += PATH_SEPARATOR; }
  if (file[0] == '/' || file[0] == '\\') {
    ret += file.substr(1);
  } else {
    ret += file;
  }
  return ret;
}

// RAII object representing a DIR pointer
struct directory {
  DIR * ptr;

  operator DIR *() const { return ptr; }

  ~directory() {
    if (ptr) { (void)closedir(ptr); }
  }
};

// Power function: Visit directory
// Expects a valid path to a directory, and two callables, of form
//
//   void(const std::string &)
//
// The file visitor is called with the full path to each file.
// The directory visitor is called with the full path to each subdirectory.

template <typename FV, typename DV>
void
visit_directory(const std::string & dir_path, FV && file_visitor, DV && dir_visitor) {
  directory dp{opendir(dir_path.c_str())};
  if (!dp) {
    fprintf(stderr, "Could not open directory: %s", dir_path.c_str());
    assert(false && "Failed to open a directory");
  }

  struct dirent * ep;
  while ((ep = readdir(dp))) {
    std::string entry_name{ep->d_name};
    if (entry_name == "." || entry_name == "..") { continue; }
    std::string entry_path = filesystem::join_path(dir_path, entry_name);

    {
      struct stat path_stat;
      stat(entry_path.c_str(), &path_stat);
      if (S_ISREG(path_stat.st_mode)) {
        std::forward<FV>(file_visitor)(entry_path);
      } else if (S_ISDIR(path_stat.st_mode)) {
        std::forward<DV>(dir_visitor)(entry_path);
      }
    }
  }
}

} // end namespace filesystem
