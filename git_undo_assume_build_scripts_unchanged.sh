#!/bin/bash

set -e

# Cause git to assume that all build scripts are unchanged, so that you can use "git add ." syntax to commit
# without committing local changes to the build scripts

git update-index --no-assume-unchanged *build*.sh
git update-index --no-assume-unchanged Toolchain-*.cmake
git update-index --no-assume-unchanged set_*_env_vars.sh
