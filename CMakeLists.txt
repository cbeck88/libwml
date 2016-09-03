cmake_minimum_required(VERSION 3.2)
Project(libwml)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    message(FATAL_ERROR "You must set your binary directory different from your source")
endif()

# default to Release build (it's what most people will use, except cegui devs)
if (NOT CMAKE_BUILD_TYPE)
    set( CMAKE_BUILD_TYPE Release CACHE STRING "Sets the configuration to build (Release, Debug, etc...)")
endif()

### pull in boilerplate cmake
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(BoilerPlate)
include(UtilityFunctions)
include(AssetUtilities)
include(FindCcache)

##############
# Find Boost #
##############

include(BoostHelper)

#######################
# Find strict-variant #
#######################

set(STRICT_VARIANT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/strict_variant_0.3/include )
MESSAGE(STATUS "Using strict-variant: " ${STRICT_VARIANT_INCLUDE_DIR})


# Make the emscripten preload package from the assets folder
if(EMSCRIPTEN)
    EmscriptenCreatePackage(${PROJECT_NAME} DATA_FILE
        PRELOAD ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets
    )
endif()

MESSAGE(STATUS "BOOST_DIR = " ${BOOST_DIR})

CreateProgram(${PROJECT_NAME}
DIRS
    src
INCLUDES
    src
    include/
    ${Boost_INCLUDE_DIRS}
    ${STRICT_VARIANT_INCLUDE_DIR}
#FLAGS
#    $<$<CXX_COMPILER_ID:GNU>:-std=c++11>
#    $<$<CXX_COMPILER_ID:Clang>:-std=c++11>
#    $<$<CXX_COMPILER_ID:AppleClang>:-std=c++11>
PRE_JS
    ${DATA_FILE}
#JS_LIBS
#    lib/humble/library_cloudfs.jslib
#    lib/humble/library_humble.jslib
PROPERTIES
    FOLDER Executables
    MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME} Demo"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.{PROJECT_NAME}"
    MACOSX_BUNDLE_COPYRIGHT "© 2016 Chris Beck "
)
source_group("Source" REGULAR_EXPRESSION "\\.(cpp|hpp|h)$")

if(EMSCRIPTEN)
    set(EM_TARGET_JS ${PROJECT_NAME}.js)
    configure_file(shell.html index.html @ONLY)
endif()

## this auto copies the needed libraries / frameworks over
CopyDependentLibs(${PROJECT_NAME})

if(NOT EMSCRIPTEN)
    CreateFolderMarkerFile(${PROJECT_NAME} asset_path.txt ${CMAKE_CURRENT_SOURCE_DIR}/assets)
endif()
