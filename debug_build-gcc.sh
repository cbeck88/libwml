#!/bin/bash
set -e
rm -rf build_debug
mkdir build_debug
cd build_debug
export CCACHE_CPP2=yes
export COMMON_FLAGS="-Wall -Werror -O0 -g3"
export C_FLAGS="$COMMON_FLAGS -Wno-unused -Wno-self-assign"
export CXX_FLAGS="$COMMON_FLAGS -Wno-unused-local-typedefs -Wno-logical-op-parentheses -std=c++14" #-fmacro-backtrace-limit=0 "
cmake -DCMAKE_C_FLAGS="$C_FLAGS" -DCMAKE_CXX_FLAGS="$CXX_FLAGS" -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++ ..
make -j3 VERBOSE=1

gdb -batch -return-child-result -ex "run" -ex "thread apply all bt" -ex "quit" ./libwml.bin.x86_64
#valgrind ./libwml.bin.x86_64 2> ../valgrind.log
