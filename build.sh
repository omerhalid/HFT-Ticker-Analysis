#!/bin/bash

# Cross-platform build script for Coinbase Ticker Analyzer

set -e

echo "Building Coinbase Ticker Analyzer..."

# Detect OS
OS="unknown"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
fi

echo "Detected OS: $OS"

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Detect number of CPU cores for parallel build
if command -v nproc >/dev/null 2>&1; then
    CORES=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=4
fi

echo "Building with $CORES parallel jobs..."

# Build the project
make -j$CORES

echo "Build completed successfully!"
echo "Run: ./build/CoinbaseTickerAnalyzer"
echo "Test: ./test.sh"