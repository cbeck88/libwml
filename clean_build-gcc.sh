#!/bin/bash
set -e
rm -rf build
mkdir build
cd build
export COMMON_FLAGS="-Wall -Werror"
export C_FLAGS="$COMMON_FLAGS -Wno-unused"
export CXX_FLAGS="$COMMON_FLAGS -std=c++14 -Wno-unused-local-typedefs"
cmake -DCMAKE_C_FLAGS="$C_FLAGS" -DCMAKE_CXX_FLAGS="$CXX_FLAGS" -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++ ..
#cmake -DCMAKE_CXX_FLAGS="-std=c++14 -fpermissive" -DCMAKE_C_COMPILER="/usr/bin/clang-3.6" -DCMAKE_CXX_COMPILER="/usr/bin/clang++-3.6" ..
make -j3 VERBOSE=1

./libwml.bin.x86_64
