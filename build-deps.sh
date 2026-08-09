#!/bin/bash
set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
LIB_DIR="$SCRIPT_DIR/lib"
TOOLCHAIN_FILE="$SCRIPT_DIR/toolchain-arm.cmake"
JOBS=$(nproc)

TARGET="${1:-}"
if [[ "$TARGET" != "x86" && "$TARGET" != "arm" ]]; then
    echo "Usage: $0 {x86|arm}" >&2
    exit 1
fi

if [ "$TARGET" == "x86" ]; then
    OUT_SUBDIR="x64"
else
    OUT_SUBDIR="arm-static"
    if ! command -v arm-linux-gnueabihf-gcc >/dev/null 2>&1; then
        echo "arm-linux-gnueabihf-gcc not found. Install it, e.g.:" >&2
        echo "  sudo apt install gcc-arm-linux-gnueabihf" >&2
        exit 1
    fi
fi

for dep in openssl zlib fftw; do
    if [ ! -d "$LIB_DIR/$dep/src" ]; then
        echo "$LIB_DIR/$dep/src not found. Run download-deps.sh first." >&2
        exit 1
    fi
done

#########################################################
###   OPENSSL                                         ###
#########################################################

echo "--- Building OpenSSL (${TARGET}) ---"

OPENSSL_SRC="$LIB_DIR/openssl/src"
OPENSSL_OUT="$LIB_DIR/openssl/out/${OUT_SUBDIR}"

pushd "$OPENSSL_SRC" >/dev/null

# In-source build shared between architectures: clear any previous
# configuration before reconfiguring for a (possibly different) target.
make clean >/dev/null 2>&1 || true

if [ "$TARGET" == "x86" ]; then
    ./Configure linux-x86_64 --prefix="$OPENSSL_OUT"
else
    CROSS_COMPILE=arm-linux-gnueabihf- \
    CFLAGS='-march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=hard -O2' \
    ./Configure linux-armv4 \
        no-shared no-dso no-async no-tests no-asm \
        --cross-compile-prefix=arm-linux-gnueabihf- \
        --prefix="$OPENSSL_OUT"
fi

make -j"$JOBS"
make install_sw

popd >/dev/null

#########################################################
###   ZLIB                                            ###
#########################################################

echo "--- Building zlib (${TARGET}) ---"

ZLIB_SRC="$LIB_DIR/zlib/src"
ZLIB_BUILD="$LIB_DIR/zlib/build/${OUT_SUBDIR}"
ZLIB_INSTALL="$LIB_DIR/zlib/out/${OUT_SUBDIR}/install"

ZLIB_CMAKE_ARGS=(
    -S "$ZLIB_SRC"
    -B "$ZLIB_BUILD"
    -DCMAKE_INSTALL_PREFIX="$ZLIB_INSTALL"
    -DZLIB_BUILD_EXAMPLES=OFF
)
if [ "$TARGET" == "arm" ]; then
    ZLIB_CMAKE_ARGS+=(-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE")
fi

cmake "${ZLIB_CMAKE_ARGS[@]}"
cmake --build "$ZLIB_BUILD" -j"$JOBS"
cmake --install "$ZLIB_BUILD"

if [ "$TARGET" == "arm" ]; then
    # zlib always builds+installs both libz.so and libz.a (no toggle in this
    # version); drop the shared one so find_package(ZLIB) can't pick it for
    # the static ARM build.
    rm -f "$ZLIB_INSTALL"/lib/libz.so*
fi

#########################################################
###   FFTW                                            ###
#########################################################

echo "--- Building FFTW (${TARGET}) ---"

FFTW_SRC="$LIB_DIR/fftw/src"
FFTW_BUILD="$LIB_DIR/fftw/build/${OUT_SUBDIR}"
FFTW_INSTALL="$LIB_DIR/fftw/out/${OUT_SUBDIR}/install"

FFTW_CMAKE_ARGS=(
    -S "$FFTW_SRC"
    -B "$FFTW_BUILD"
    -DCMAKE_INSTALL_PREFIX="$FFTW_INSTALL"
    -DBUILD_TESTS=OFF
    # FFTW 3.3.10's CMakeLists pins `cmake_minimum_required(VERSION 3.0)`,
    # which CMake >= 4.0 refuses outright. This tells CMake to evaluate it
    # under 3.5 policy behavior instead of rejecting it.
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5
)
if [ "$TARGET" == "arm" ]; then
    FFTW_CMAKE_ARGS+=(-DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE")
fi

cmake "${FFTW_CMAKE_ARGS[@]}"
cmake --build "$FFTW_BUILD" -j"$JOBS"
cmake --install "$FFTW_BUILD"

echo "--- Done: dependencies for ${TARGET} installed under lib/*/out/${OUT_SUBDIR} ---"
