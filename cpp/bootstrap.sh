#!/bin/bash
set -eu

# Clean.
rm -rf ./build
rm -rf ./build-wasm

# Install formatting git hook.
HOOKS_DIR=$(git rev-parse --git-path hooks)
# The pre-commit script will live in a barretenberg-specific hooks directory
# That may be just in the top level of this repository,
# or may be in a .git/modules/barretenberg subdirectory when this is actually a submodule
# Either way, running `git rev-parse --show-toplevel` from the hooks directory gives the path to barretenberg
echo "cd \$(git rev-parse --show-toplevel)/cpp && ./format.sh staged" > $HOOKS_DIR/pre-commit
chmod +x $HOOKS_DIR/pre-commit

# Determine system.
if [[ "$OSTYPE" == "darwin"* ]]; then
  OS=macos
elif [[ "$OSTYPE" == "linux-gnu" ]]; then
  OS=linux
else
  echo "Unknown OS: $OSTYPE"
  exit 1
fi

# Download ignition transcripts.
cd ./srs_db
./download_ignition.sh 3
cd ..

# Pick native toolchain file.
ARCH=$(uname -m)
if [ "$OS" == "macos" ]; then
  if [ "$(which brew)" != "" ]; then
    export BREW_PREFIX=$(brew --prefix)

    # Ensure we have toolchain.
    if [ ! "$?" -eq 0 ] || [ ! -f "$BREW_PREFIX/opt/llvm/bin/clang++" ]; then
      echo "Default clang not sufficient. Install homebrew, and then: brew install llvm libomp clang-format"
      exit 1
    fi

    PRESET=homebrew
  else
    PRESET=default
  fi
else
  if [ "$(which clang++-15)" != "" ]; then
    PRESET=clang15
  else
    PRESET=default
  fi
fi

echo "#################################"
echo "# Building with preset: $PRESET"
echo "# When running cmake directly, remember to use: --build --preset $PRESET"
echo "#################################"

ADDED_FLAGS="-DHAVE_THREAD_SAFETY_ATTRIBUTES=0 -DHAVE_STD_REGEX=0 -DHAVE_POSIX_REGEX=0 -DHAVE_STEADY_CLOCK=0 -DTESTING=0"

# Build native.
cmake --preset $PRESET -DCMAKE_BUILD_TYPE=RelWithDebInfo -DMULTITHREADING=ON $ADDED_FLAGS -DOMP_MULTITHREADING=OFF  -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchains/ios.cmake -DPLATFORM=OS64
cmake --build --preset $PRESET ${@/#/--target }

# Install wasi-sdk.
# ./scripts/install-wasi-sdk.sh

# Build WASM.
# cmake --preset wasm
# cmake --build --preset wasm

# Build WASM with new threading.
# cmake --preset wasm-threads
# cmake --build --preset wasm-threads
