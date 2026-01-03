#!/bin/bash
CUR_DIR=$(pwd)
BUILD_DIR="$CUR_DIR/../build"

rm -rf "$BUILD_DIR"
cmake   -G "Unix Makefiles" \
        -B "$BUILD_DIR" \
        -S . \
        -DCMAKE_BUILD_TYPE=Debug

cd "$BUILD_DIR"
make -j$(nproc)
# make install
cd $CUR_DIR
