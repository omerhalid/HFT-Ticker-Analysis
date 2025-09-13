#!/bin/bash

# Example script to run the Coinbase Ticker Analyzer
# This script demonstrates different usage scenarios

set -e

echo "=== Coinbase Ticker Analyzer Examples ==="

# Check if executable exists
if [ ! -f "build/CoinbaseTickerAnalyzer" ]; then
    echo "Error: Executable not found. Please run ./build.sh first."
    exit 1
fi

cd build

echo ""
echo "Example 1: Monitor BTC-USD (default)"
echo "Command: ./CoinbaseTickerAnalyzer"
echo "Press Ctrl+C to stop after a few seconds..."
echo ""
read -p "Press Enter to start Example 1..."

timeout 10s ./CoinbaseTickerAnalyzer || true

echo ""
echo "Example 2: Monitor ETH-USD with custom output file"
echo "Command: ./CoinbaseTickerAnalyzer -p ETH-USD -o eth_ticker_data.csv"
echo "Press Ctrl+C to stop after a few seconds..."
echo ""
read -p "Press Enter to start Example 2..."

timeout 10s ./CoinbaseTickerAnalyzer -p ETH-USD -o eth_ticker_data.csv || true

echo ""
echo "Example 3: Show help"
echo "Command: ./CoinbaseTickerAnalyzer --help"
echo ""
./CoinbaseTickerAnalyzer --help

echo ""
echo "Examples completed!"
echo ""
echo "Check the generated CSV files:"
ls -la *.csv 2>/dev/null || echo "No CSV files generated (examples were too short)"

echo ""
echo "To run the application continuously, use:"
echo "  ./CoinbaseTickerAnalyzer -p BTC-USD -o ticker_data.csv"
echo ""
echo "Available trading pairs include:"
echo "  BTC-USD, ETH-USD, LTC-USD, BCH-USD"
echo "  BTC-EUR, ETH-EUR, LTC-EUR"
echo "  And many more..."
