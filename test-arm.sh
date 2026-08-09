#!/bin/bash
set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR="$SCRIPT_DIR/build/release-arm"

# Every sample program accepts an optional/required "seconds" argument
# (io_time_from_args, argv[1]); the few that don't take arguments (aesgmc,
# rsa_encrypt, helper_test) simply ignore it. So "1" works uniformly.
RUN_ARG="1"

# Package/binary naming varies by distro: Ubuntu >= 25.10 ships qemu-arm via
# qemu-user-binfmt; Debian and older Ubuntu ship qemu-arm-static via
# qemu-user-static.
QEMU_ARM=""
for candidate in qemu-arm-static qemu-arm; do
    if command -v "$candidate" >/dev/null 2>&1; then
        QEMU_ARM="$candidate"
        break
    fi
done

if [ -z "$QEMU_ARM" ]; then
    echo "No ARM user-mode qemu found. Install one, e.g.:" >&2
    echo "  sudo apt install qemu-user-binfmt   # Ubuntu >= 25.10 (installs qemu-arm)" >&2
    echo "  sudo apt install qemu-user-static   # Debian / older Ubuntu (installs qemu-arm-static)" >&2
    exit 1
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "$BUILD_DIR not found. Run build-deps.sh arm && build-release-arm.sh first." >&2
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
    if (cd "$SCRATCH_DIR" && "$QEMU_ARM" "$bin" "$RUN_ARG" >/dev/null 2>&1); then
        echo "OK"
    else
        echo "FAILED (exit $?)"
        FAILED+=("$name")
    fi
done

echo
if [ ${#FAILED[@]} -eq 0 ]; then
    echo "All ${#BINARIES[@]} ARM binaries ran successfully under $QEMU_ARM."
else
    echo "${#FAILED[@]}/${#BINARIES[@]} failed: ${FAILED[*]}" >&2
    exit 1
fi
