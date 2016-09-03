#############
# Find SDL2 #
#############
# TODO: Replace with https://github.com/dolphin-emu/dolphin/blob/master/CMakeTests/FindSDL2.cmake?name=async-dvd

include(FindPackageHandleStandardArgs)

find_path(SDL2_INCLUDE_DIR
  NAMES SDL2/SDL.h
  PATH_SUFFIXES SDL2
)

find_library(SDL2_LIBRARY
  NAMES SDL2
)

mark_as_advanced(SDL2_LIBRARY, SDL2_INCLUDE_DIR)

find_package_handle_standard_args(SDL2 DEFAULT_MSG SDL2_LIBRARY SDL2_INCLUDE_DIR)

if (SDL2_FOUND)
  SET( SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
  SET( SDL2_LIBRARIES ${SDL2_LIBRARY})
else()
  SET( SDL2_INCLUDE_DIRS )
  SET( SDL2_LIBRARIES )
endif()
