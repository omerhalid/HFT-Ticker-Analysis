#!/bin/bash

# Coinbase Ticker Analyzer Build Script
# This script automates the build process for the project

set -e  # Exit on any error

echo "=== Coinbase Ticker Analyzer Build Script ==="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root directory."
    exit 1
fi

# Check for required tools
print_status "Checking build requirements..."

if ! command -v cmake &> /dev/null; then
    print_error "CMake is not installed. Please install CMake 3.16 or higher."
    exit 1
fi

if ! command -v make &> /dev/null && ! command -v ninja &> /dev/null; then
    print_error "Neither make nor ninja build system found. Please install one of them."
    exit 1
fi

if ! command -v git &> /dev/null; then
    print_error "Git is not installed. Required for fetching dependencies."
    exit 1
fi

# Check C++ compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    print_error "No C++ compiler found. Please install GCC or Clang."
    exit 1
fi

print_success "All build requirements satisfied"

# Create build directory
print_status "Creating build directory..."
if [ -d "build" ]; then
    print_warning "Build directory already exists. Cleaning..."
    rm -rf build
fi
mkdir build
cd build

# Configure with CMake
print_status "Configuring project with CMake..."
if command -v ninja &> /dev/null; then
    cmake .. -G Ninja
    BUILD_TOOL="ninja"
else
    cmake ..
    BUILD_TOOL="make"
fi

print_success "CMake configuration completed"

# Build the project
print_status "Building project..."
if [ "$BUILD_TOOL" = "ninja" ]; then
    ninja
else
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
fi

print_success "Build completed successfully"

# Run tests
print_status "Running unit tests..."
if [ "$BUILD_TOOL" = "ninja" ]; then
    ninja test
else
    make test
fi

print_success "All tests passed"

# Generate documentation
print_status "Generating documentation..."
if [ "$BUILD_TOOL" = "ninja" ]; then
    ninja docs
else
    make docs
fi

print_success "Documentation generated"

# Display build results
echo ""
echo "=== Build Summary ==="
print_success "Project built successfully!"
echo ""
echo "Executable location: $(pwd)/CoinbaseTickerAnalyzer"
echo "Documentation location: $(pwd)/docs/html/index.html"
echo ""
echo "Usage examples:"
echo "  ./CoinbaseTickerAnalyzer                    # Default (BTC-USD)"
echo "  ./CoinbaseTickerAnalyzer -p ETH-USD         # Monitor ETH-USD"
echo "  ./CoinbaseTickerAnalyzer -p BTC-USD -o btc.csv  # Custom output file"
echo "  ./CoinbaseTickerAnalyzer --help             # Show help"
echo ""

# Check if executable was created
if [ -f "CoinbaseTickerAnalyzer" ]; then
    print_success "Executable created successfully"
    
    # Show executable info
    echo "Executable information:"
    ls -lh CoinbaseTickerAnalyzer
    echo ""
    
    # Test help command
    print_status "Testing help command..."
    ./CoinbaseTickerAnalyzer --help
else
    print_error "Executable not found. Build may have failed."
    exit 1
fi

print_success "Build script completed successfully!"
echo ""
echo "You can now run the application with:"
echo "  cd build && ./CoinbaseTickerAnalyzer"
