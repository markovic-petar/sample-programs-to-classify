#!/bin/bash
set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR="$SCRIPT_DIR/build/debug-x86"

# Every sample program accepts an optional/required "seconds" argument
# (io_time_from_args, argv[1]); the few that don't take arguments (aesgmc,
# rsa_encrypt, helper_test) simply ignore it. So "1" works uniformly.
RUN_ARG="1"

if [ ! -d "$BUILD_DIR" ]; then
    echo "$BUILD_DIR not found. Run build-deps.sh x86 && build-debug-x86.sh first." >&2
    exit 1
fi

mapfile -t BINARIES < <(find "$BUILD_DIR" -maxdepth 1 -type f -executable | sort)

if [ ${#BINARIES[@]} -eq 0 ]; then
    echo "No executables found in $BUILD_DIR." >&2
    exit 1
fi

# Some samples (e.g. crypto_rsa_decrypt) read/write files relative to cwd
# and expect fixtures from program_files/ (e.g. encrypted_rsa.txt, produced
# by crypto_rsa_encrypt). Run against a scratch copy so the tracked fixtures
# in program_files/ are never mutated by a test run.
SCRATCH_DIR=$(mktemp -d)
trap 'rm -rf "$SCRATCH_DIR"' EXIT
if [ -d "$SCRIPT_DIR/program_files" ]; then
    cp -r "$SCRIPT_DIR/program_files/." "$SCRATCH_DIR/"
fi

FAILED=()

for bin in "${BINARIES[@]}"; do
    name=$(basename "$bin")
    printf '%-24s' "$name"
    if (cd "$SCRATCH_DIR" && "$bin" "$RUN_ARG" >/dev/null 2>&1); then
        echo "OK"
    else
        echo "FAILED (exit $?)"
        FAILED+=("$name")
    fi
done

echo
if [ ${#FAILED[@]} -eq 0 ]; then
    echo "All ${#BINARIES[@]} x86_64 binaries ran successfully."
else
    echo "${#FAILED[@]}/${#BINARIES[@]} failed: ${FAILED[*]}" >&2
    exit 1
fi
