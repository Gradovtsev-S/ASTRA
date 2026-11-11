#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
build_dir=$(mktemp -d)
trap 'rm -rf "$build_dir"' EXIT
cc -std=c11 -O2 -DASTRONOMY_ENGINE_WHOLE_SECOND -c src/logic/astronomy.c -o "$build_dir/astronomy.o"
c++ -std=c++11 -Wall -Wextra -Werror -Isrc/logic test/native/pointing.cpp \
    src/logic/handler.cpp src/logic/orientation.cpp "$build_dir/astronomy.o" -lm -o "$build_dir/pointing"
"$build_dir/pointing"
echo "Pointing tests passed"
