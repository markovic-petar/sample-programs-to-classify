#!/bin/bash
set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
LIB_DIR="$SCRIPT_DIR/lib"

OPENSSL_VERSION="openssl-3.6.0"
OPENSSL_DIR="$LIB_DIR/openssl/src"

echo "--- Cloning OpenSSL ${OPENSSL_VERSION} ---"

if [ -d "$OPENSSL_DIR" ]; then
    echo "OpenSSL source already exists, skipping."
else
    git clone \
        --branch "$OPENSSL_VERSION" \
        --depth 1 \
        https://github.com/openssl/openssl.git \
        "$OPENSSL_DIR"
fi

ZLIB_VERSION="v1.3.1"
ZLIB_DIR="$LIB_DIR/zlib/src"

echo "--- Cloning zlib ${ZLIB_VERSION} ---"

if [ -d "$ZLIB_DIR" ]; then
    echo "zlib source already exists, skipping."
else
    git clone \
        --branch "$ZLIB_VERSION" \
        --depth 1 \
        https://github.com/madler/zlib.git \
        "$ZLIB_DIR"
fi

# FFTW is not released via GitHub tags; use the official release tarball.
# (A git clone lacks the pre-generated codelets and would require OCaml/genfft.)
#
# FFTW publishes only an MD5 (https://www.fftw.org/fftw-3.3.10.tar.gz.md5sum ->
# 8ccbf6a5ea78a16dbc3e1306e234cc5c). We pin the stronger sha256 below; it is the
# hash of that same MD5-verified tarball, so it can be re-derived independently.
FFTW_VERSION="3.3.10"
FFTW_SHA256="56c932549852cddcfafdab3820b0200c7742675be92179e59e6215b340e26467"
FFTW_DIR="$LIB_DIR/fftw/src"

echo "--- Downloading FFTW ${FFTW_VERSION} ---"

if [ -d "$FFTW_DIR" ]; then
    echo "FFTW source already exists, skipping."
else
    FFTW_TARBALL="$LIB_DIR/fftw-${FFTW_VERSION}.tar.gz"
    curl -fSL \
        -o "$FFTW_TARBALL" \
        "https://www.fftw.org/fftw-${FFTW_VERSION}.tar.gz"
    echo "${FFTW_SHA256}  ${FFTW_TARBALL}" | sha256sum -c -
    mkdir -p "$FFTW_DIR"
    tar -xzf "$FFTW_TARBALL" -C "$FFTW_DIR" --strip-components=1
    rm -f "$FFTW_TARBALL"
fi
