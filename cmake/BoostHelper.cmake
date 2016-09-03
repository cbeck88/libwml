# This file tries to help FindBoost.cmake to work correctly, esp. with emscripten

set(PACKAGE_FIND_NAME Boost)
set(PACKAGE_FIND_VERSION 1.53)
set(PACKAGE_VERSION_MAJOR 1)
set(PACKAGE_VERSION_MINOR 53)
set(PACKAGE_VERSION_COUNT 2)

set(Boost_FIND_VERSION ${PACKAGE_FIND_VERSION})
set(Boost_VERSION_MAJOR ${PACKAGE_VERSION_MAJOR})
set(Boost_VERSION_MINOR ${PACKAGE_VERSION_MINOR})
set(Boost_VERSION_COUNT ${PACKAGE_VERSION_COUNT})

#OPTION(Boost_INCLUDE_DIRS "Manually specify the directory of the boost includes (required to do this for emscripten build!)" OFF)
#Todo: can we manually create path variables?

set(expected_dir /external/boost_root)
set(expected_dir_abs ${CMAKE_CURRENT_SOURCE_DIR}${expected_dir})

if(IS_DIRECTORY ${expected_dir_abs})
  MESSAGE(STATUS "Found folder ${expected_dir}, loading boost from there")
  if(EXISTS ${expected_dir_abs}/boost/version.hpp)
    #MESSAGE(STATUS "Found boost: version = TODO") # actually its now done below...
    set(Boost_INCLUDE_DIRS ${expected_dir_abs})

    # Extract Boost_VERSION and Boost_LIB_VERSION from version.hpp
    set(Boost_VERSION 0)
    set(Boost_LIB_VERSION "")
    file(STRINGS "${Boost_INCLUDE_DIRS}/boost/version.hpp" _boost_VERSION_HPP_CONTENTS REGEX "#define BOOST_(LIB_)?VERSION ")
    set(_Boost_VERSION_REGEX "([0-9]+)")
    set(_Boost_LIB_VERSION_REGEX "\"([0-9_]+)\"")
    foreach(v VERSION LIB_VERSION)
      if("${_boost_VERSION_HPP_CONTENTS}" MATCHES "#define BOOST_${v} ${_Boost_${v}_REGEX}")
        set(Boost_${v} "${CMAKE_MATCH_1}")
      endif()
    endforeach()
    unset(_boost_VERSION_HPP_CONTENTS)

    math(EXPR Boost_MAJOR_VERSION "${Boost_VERSION} / 100000")
    math(EXPR Boost_MINOR_VERSION "${Boost_VERSION} / 100 % 1000")
    math(EXPR Boost_SUBMINOR_VERSION "${Boost_VERSION} % 100")

    set(Boost_ERROR_REASON
      "${Boost_ERROR_REASON}Boost version: ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}\nBoost include path: ${Boost_INCLUDE_DIRS}")
    if(Boost_DEBUG)
      message(STATUS "[ ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} ] "
                     "version.hpp reveals boost "
                     "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
    endif()

    MESSAGE(STATUS "Found boost: version = ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")

    # Set Boost_FOUND based on requested version.
    set(Boost_VERSION "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
    if("${Boost_VERSION}" VERSION_LESS "${Boost_FIND_VERSION}")
      set(Boost_FOUND 0)
    else()
      set(Boost_FOUND 1)
    endif()

    # Make it a fatal error if we can't find boost
    if(NOT Boost_FOUND)
      set(Boost_ERROR_REASON
        "${Boost_ERROR_REASON}\nDetected version of Boost is too old. Requested version was ${Boost_FIND_VERSION} (or newer)")
      MESSAGE(FATAL_ERROR ${Boost_ERROR_REASON})
    endif ()
  else()
    MESSAGE(FATAL_ERROR "Did not find boost in ${expected_dir}, check your install."
"
Could not find ${expected_dir}/boost/version.hpp.")
  endif()
else() # Use find boost
  MESSAGE(STATUS "Did not find a folder: " ${expected_dir_abs})
  MESSAGE(STATUS "Falling back to FindBoost.cmake")
  if(EMSCRIPTEN)
    MESSAGE(FATAL_ERROR "To build with emscripten, you MUST place a boost install at ${expected_dir} inside this repo, or a symlink to one."
"
Only the headers are required. See docs in external/README.md")
  else()
    find_package(Boost 1.53)

    if (Boost_FOUND)
      MESSAGE(STATUS "Found boost: " ${Boost_INCLUDE_DIRS})
    else()
      MESSAGE(FATAL_ERROR "Boost was not found!"
"
If you are using wesbuild, install boost to /external/boost_root or place a symlink there to it."
"
Otherwise -- to help cmake find boost, inspect the 'FindBoost.cmake' file and give it a hint that it recognizes, via an environment variable or something.")
    endif()
  endif()
endif()
