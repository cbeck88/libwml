#!/bin/bash
BIN=~/llvm-src/llvm/build/bin/clang-format
"$BIN" --style=file -i `find src/ -name *.?pp`
"$BIN" --style=file -i `find include/ -name *.?pp`
