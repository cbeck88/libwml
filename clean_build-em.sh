#!/bin/bash
set -e

source set_emscripten_env_vars.sh

rm -rf build_em
mkdir build_em
cd build_em

export COMMON_FLAGS="-O3 -s USE_LIBPNG=1 -s USE_ZLIB=1 -s USE_SDL=2 -Wall -Werror"
export DEBUG_FLAGS=""
#export DEBUG_FLAGS="-s SAFE_HEAP=1 -s ALIASING_FUNCTION_POINTERS=0"
export C_FLAGS="$COMMON_FLAGS -Wno-unused -Wno-self-assign -Wno-parentheses-equality"
export CXX_FLAGS="$COMMON_FLAGS -std=c++14 -Wno-unused-local-typedefs -Wno-logical-op-parentheses -s DEMANGLE_SUPPORT=1 --llvm-lto 1 -s TOTAL_MEMORY=500000000 -s NO_EXIT_RUNTIME=1" #$DEBUG_FLAGS"
#export CXX_FLAGS="$COMMON_FLAGS -std=c++11 -Wno-unused-local-typedefs -Wno-logical-op-parentheses -s USE_SDL=2 -s DEMANGLE_SUPPORT=1 -s ASSERTIONS=2 -s ALLOW_MEMORY_GROWTH=1" #$DEBUG_FLAGS"

emcmake cmake .. -DCMAKE_C_FLAGS="$C_FLAGS" -DCMAKE_CXX_FLAGS="$CXX_FLAGS" -DBOOST_DIR="$BOOST_DIR"
emmake make -j3 VERBOSE=1
firefox index.html
cd ..
