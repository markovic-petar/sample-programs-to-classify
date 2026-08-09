#!/bin/bash
#
# Documents how FFTW_SHA256 in download-deps.sh was obtained.
# Run this to re-derive it; it should print the same value that is pinned there.
#
curl -fSL https://www.fftw.org/fftw-3.3.10.tar.gz | sha256sum
