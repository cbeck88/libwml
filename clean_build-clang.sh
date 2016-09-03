#!/bin/bash
set -e
rm -rf build
mkdir build
cd build
export CCACHE_CPP2=yes
export COMMON_FLAGS="-Wall -Werror -Qunused-arguments " #-ftemplate-backtrace-limit=0 -fmacro-backtrace-limit=0 # -O3
export C_FLAGS="$COMMON_FLAGS -Wno-unused -Wno-self-assign"
export CXX_FLAGS="$COMMON_FLAGS -Wno-unused-local-typedefs -Wno-logical-op-parentheses -std=c++14" #-fmacro-backtrace-limit=0 "
cmake -DCMAKE_C_FLAGS="$C_FLAGS" -DCMAKE_CXX_FLAGS="$CXX_FLAGS" -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ ..
make -j3 VERBOSE=1

#./Biff.bin.x86_64 2>../biff.log
./libwml.bin.x86_64 --test
