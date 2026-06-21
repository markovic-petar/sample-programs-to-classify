#!/bin/bash
set -e

cmake --preset debug-x86
cmake --build --preset debug-x86 -j$(nproc)
