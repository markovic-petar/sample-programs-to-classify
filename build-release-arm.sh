#!/bin/bash
set -e

cmake --preset release-arm
cmake --build --preset release-arm -j$(nproc)
