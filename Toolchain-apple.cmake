# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Apple)
SET(APPLE on)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER clang++)

#SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
# TODO: Put apple libtool here?

SET(CMAKE_CXX_ARCHIVE_CREATE "/usr/bin/libtool -v -o <TARGET> <LINK_FLAGS> <OBJECTS>")
SET(CMAKE_C_ARCHIVE_CREATE "/usr/bin/libtool -v -o <TARGET> <LINK_FLAGS> <OBJECTS>")

SET(CMAKE_FIND_ROOT_PATH /opt/local/)

# here is the target environment located
# *** Edit this for your machine! ***
#SET(CMAKE_FIND_ROOT_PATH  /home/mrule/desktop/)
#

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

