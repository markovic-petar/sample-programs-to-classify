#!/bin/bash
set -e

CUR_DIR=$(pwd)
BUILD_DIR=$CUR_DIR/../build_arm_static

rm -rf $BUILD_DIR
cmake   -G "Unix Makefiles" \
        -B $BUILD_DIR \
        -S . \
        -DCMAKE_TOOLCHAIN_FILE=toolchain-arm.cmake

cd $BUILD_DIR
make -j$(nproc)
# make install
cd $CUR_DIR
# cmake --build $BUILD_DIR
