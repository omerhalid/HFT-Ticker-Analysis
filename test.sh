#!/bin/bash

# Test script for Coinbase Ticker Analyzer

set -e

echo "Running tests for Coinbase Ticker Analyzer..."

# Build if not already built
if [ ! -d "build" ]; then
    echo "Building project..."
    ./build.sh
fi

# Run tests
cd build
make test

echo "All tests passed!"
