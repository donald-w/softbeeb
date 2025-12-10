#!/usr/bin/env bash
set -euo pipefail

# Build and run the SDL shim test app.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$BUILD_DIR"
"$BUILD_DIR/sdltest"
