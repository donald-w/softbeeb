#!/bin/bash
# Clean build and run softbeeb emulator

set -e  # Exit on error

echo "🧹 Cleaning build directory..."
rm -rf build

echo "🔧 Configuring with CMake..."
cmake -S . -B build

echo "🔨 Building softbeeb..."
cmake --build build

echo "🚀 Running softbeeb..."
echo ""
./build/softbeeb
