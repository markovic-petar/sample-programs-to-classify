# Build environment

This records the toolchain this project is built and verified against, so the
build can be reproduced later on a different machine.

## Verified on

- OS: Ubuntu 26.04 LTS (`resolute`)
- glibc: 2.43 (Ubuntu GLIBC 2.43-2ubuntu2.3)
- cmake: 4.2.3
- gcc / g++ (host, x86_64): 15.2.0 (`build-essential`)
- GNU make: 4.4.1
- ARM cross toolchain: `gcc-arm-linux-gnueabihf` / `g++-arm-linux-gnueabihf` 15.2.0,
  targeting `armv7-a` (Cortex-A8, hard-float) via [toolchain-arm.cmake](toolchain-arm.cmake)
- perl: 5.40.1 (required by OpenSSL's `Configure`)
- git: 2.53.0, curl: 8.18.0 (used by [download-deps.sh](download-deps.sh))
- qemu-arm (user-mode) 10.2.1, from the `qemu-user-binfmt` package (used by
  [test-arm.sh](test-arm.sh) to run the cross-compiled ARM binaries directly
  on the x86_64 host)

## Target hardware

The ARM build targets an [Olimex A13-OLinuXino-MICRO](https://www.olimex.com/Products/OLinuXino/A13/A13-OLinuXino-MICRO/open-source-hardware)
(Allwinner A13, single-core Cortex-A8). [toolchain-arm.cmake](toolchain-arm.cmake)'s
flags (`armv7-a`, `vfpv3-d16`, hard-float) have already been validated by
compiling and running on the physical board, so they're left as-is here.

The build host's OS does *not* need to match whatever OS image is flashed on
the board (see the [Olimex Linux wiki page](https://www.olimex.com/wiki/A13-OLinuXino-MICRO#Linux)
for the options there): the ARM build is fully static (OpenSSL `no-shared`,
zlib's `.so` deleted post-install, FFTW `BUILD_SHARED_LIBS=OFF`), so the
resulting binaries don't dynamically link against the board's libc at
runtime — only the kernel syscall ABI has to match, which is stable across
armhf Linux distributions. This would stop being true if the project ever
switched to dynamic linking for the ARM build, at which point the board's
actual glibc version would need to match what the cross-compiler links
against.

One untested, optional tuning idea for closer instruction-scheduling fidelity
to the Cortex-A8 specifically (rather than generic armv7-a): adding
`-mtune=cortex-a8` to `CMAKE_C_FLAGS_INIT` in `toolchain-arm.cmake`. Not
applied here since the current flags are already hardware-validated and
changing a working cross-compilation setup isn't worth the risk without a
concrete reason.

## Pinned third-party library versions

Set in [download-deps.sh](download-deps.sh):

- OpenSSL 3.6.0
- zlib 1.3.1
- FFTW 3.3.10 (tarball, sha256-pinned — see [verify-fftw-checksum.sh](verify-fftw-checksum.sh))

## Installing prerequisites (Ubuntu / Debian)

```bash
sudo apt update
sudo apt install -y build-essential cmake git curl perl \
    gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

On another distro, install the equivalent packages: a C/C++ toolchain, CMake
>= 3.23, git, curl, perl, and an `arm-linux-gnueabihf-` cross-compiler
(e.g. Fedora: `dnf install gcc gcc-c++ cmake git curl perl arm-linux-gnu-gcc`,
package name for the ARM cross toolchain varies by distro).

To also run `test-arm.sh`, install ARM user-mode QEMU. The package/binary
name varies by distro version:

```bash
sudo apt install -y qemu-user-binfmt   # Ubuntu >= 25.10 — installs /usr/bin/qemu-arm
sudo apt install -y qemu-user-static   # Debian, older Ubuntu — installs /usr/bin/qemu-arm-static
```

(`qemu-user-static` is a virtual/transitional package name on Ubuntu 26.04 —
`apt install qemu-user-static` fails there with "no installation candidate";
`test-arm.sh` checks for both binary names so either package works.)

## Full build, from a clean checkout

```bash
./download-deps.sh       # clones/downloads OpenSSL, zlib, FFTW sources into lib/*/src
./build-deps.sh x86       # builds+installs them into lib/*/out/x64
./build-debug-x86.sh      # configures+builds the project (CMake preset debug-x86)

# ARM cross build:
./build-deps.sh arm       # builds+installs them into lib/*/out/arm-static
./build-release-arm.sh    # configures+builds the project (CMake preset release-arm)
./test-arm.sh             # smoke-tests every ARM binary under qemu-arm
```

Both dependency and project builds were run end-to-end on the environment
above: the x86 build produces dynamically-linked executables that run, and
the ARM build produces fully static ARM EABI5 executables (verified with
`file`). `test-arm.sh` ran all 22 ARM binaries under `qemu-arm` — all pass.

## First-time VS Code setup

`.vscode/settings.json` points `ms-vscode.cpptools` at `ms-vscode.cmake-tools`
as its `configurationProvider`, so IntelliSense gets its include paths and
defines from the CMake configure step instead of guessing. On a fresh
clone/open, if IntelliSense is showing red squiggles on standard includes:

1. Ctrl+Shift+P → `CMake: Configure` (cmake-tools doesn't always run this
   automatically on first open, e.g. if it's waiting on a kit selection).
2. Ctrl+Shift+P → `Developer: Reload Window` (cpptools caches the provider
   handshake per-session; a stale cache won't pick up a configure that ran
   after the extension already loaded).

## Known quirks

- FFTW 3.3.10's `CMakeLists.txt` pins `cmake_minimum_required(VERSION 3.0)`,
  which CMake >= 4.0 refuses to configure. `build-deps.sh` passes
  `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` to work around this — if FFTW's build
  step fails with a `cmake_minimum_required` error, this is why.
- OpenSSL builds in-source (`./Configure` + `make` run directly in
  `lib/openssl/src`), unlike zlib/FFTW which use out-of-tree CMake build
  directories per architecture. `build-deps.sh` runs `make clean` before
  reconfiguring so switching between `x86` and `arm` targets doesn't mix
  object files from the other architecture.
- zlib 1.3.1's `CMakeLists.txt` always builds and installs both `libz.so`
  and `libz.a` (no shared/static toggle in this version). For the ARM
  target, `build-deps.sh` deletes the installed `libz.so*` afterward so
  `find_package(ZLIB)` can only resolve the static library, keeping ARM
  binaries fully static.
- Several samples (`crypto_rsa_decrypt`, `crypto_aes_encrypt`,
  `crypto_aes_decrypt`, ...) read/write files relative to the current
  working directory and expect fixtures from [program_files/](program_files/)
  to be present there (e.g. `plaintext.txt`, `encrypted_rsa.txt`). Running
  them from an unrelated directory doesn't just fail cleanly — some (e.g.
  `crypto_aes_encrypt`) segfault, since a failed read isn't checked before
  the buffer is used. `test-arm.sh` runs every binary from a scratch copy of
  `program_files/` (never the tracked directory itself) for exactly this
  reason.
