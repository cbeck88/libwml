#!/bin/bash

# Note: Some info here: http://stackoverflow.com/questions/6077414/cmake-how-to-set-the-ldflags-in-cmakelists-txt
# and here https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/4/html/Using_ld_the_GNU_Linker/win32.html

set -e

source set_mingw_env_vars.sh

rm -rf build_mingw
mkdir build_mingw
cd build_mingw

export CCACHE_CPP2=true
export COMMON_FLAGS="-O3 -Wall -Werror"
export C_FLAGS="$COMMON_FLAGS -Wno-unused-function -Wno-maybe-uninitialized"
export CXX_FLAGS="$COMMON_FLAGS -std=c++14 -Wno-unused-local-typedefs -Wno-strict-aliasing"

cmake -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++ -static" -DCMAKE_C_FLAGS="$C_FLAGS" -DCMAKE_CXX_FLAGS="$CXX_FLAGS" -DMINGW_DEP_ROOT="$DEP_ROOT" -DBOOST_DIR="$BOOST_DIR" -DCMAKE_TOOLCHAIN_FILE="Toolchain-mingw.cmake" ..
make -j3 VERBOSE=1

cp "$DEP_ROOT/bin/SDL2.dll" ./

wine ./Biff.exe
