#!/bin/bash
set -e

cmake --preset debug-arm
cmake --build --preset debug-arm -j$(nproc)
