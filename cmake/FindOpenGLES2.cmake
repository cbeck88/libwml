################################################################################
# Custom cmake module for CEGUI to find OpenGL ES (1.1) 
#
# Placeholder module to create some vars we can manually set.
# Later will create a proper tests (or steal them, anyway)
################################################################################
include(FindPackageHandleStandardArgs)

find_path(OPENGLES2_H_PATH NAMES GLES2/gl2.h)

if(APPLE)
#  STATUS(MESSAGE "searching for OS X compiled mesa from homebrew...")
  find_library(OPENGLES2_LIB NAMES libGL GL libGL.dylib)
elseif(MINGW)
  find_library(OPENGLES2_LIB NAMES opengl32.dll)
else()
  find_library(OPENGLES2_LIB NAMES GLESv2)
endif()

mark_as_advanced(OPENGLES2_H_PATH OPENGLES2_LIB OPENGLES2_LIB_DBG)

find_package_handle_standard_args(OPENGLES2 DEFAULT_MSG OPENGLES2_LIB OPENGLES2_H_PATH)

# set up output vars
if (OPENGLES2_FOUND)
    set (OPENGLES2_INCLUDE_DIR ${OPENGLES2_H_PATH})
    set (OPENGLES2_LIBRARIES ${OPENGLES2_LIB})
    if (OPENGLES2_LIB_DBG)
        set (OPENGLES2_LIBRARIES_DBG ${OPENGLES2_LIB_DBG})
    endif()
else()
    set (OPENGLES2_INCLUDE_DIR)
    set (OPENGLES2_LIBRARIES)
    set (OPENGLES2_LIBRARIES_DBG)
endif()

