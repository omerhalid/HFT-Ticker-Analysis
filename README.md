# Coinbase Ticker Analyzer

A high-performance C++ application that subscribes to Coinbase ticker data via WebSockets, calculates Exponential Moving Averages (EMAs), and logs data to CSV files.

## Features

- **Real-time WebSocket connection** to Coinbase public ticker feed
- **EMA calculations** for price and mid-price with 5-second intervals
- **Asynchronous CSV logging** with lock-free data structures
- **Multithreaded architecture** optimized for low latency
- **Comprehensive test suite** with 40 unit tests

## Requirements

- CMake 3.16+
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- OpenSSL
- libwebsockets

## Installation

### Docker
```bash
# Run with Docker (no dependencies needed)
./docker-run.sh

# Run with custom options
./docker-run.sh -p ETH-USD -o ticker_data.csv
```

**Prerequisites:** Docker (install from [docker.com](https://docs.docker.com/get-docker/))

### Manual Build
```bash
# Install dependencies: cmake, libssl-dev, libwebsockets-dev, pkg-config
# Then build and run:
./build.sh
./test.sh
./build/CoinbaseTickerAnalyzer
```

## Usage

```bash
./CoinbaseTickerAnalyzer [options]

Options:
  -p, --product <ID>    Product ID to analyze (default: BTC-USD)
  -o, --output <file>   Output CSV filename (default: ticker_data.csv)
  -h, --help           Show help message
```

## Architecture

The application uses a multithreaded architecture with lock-free data structures:

- **WebSocket I/O Thread**: Handles real-time data reception
- **Data Processing Thread**: Calculates EMAs and processes ticker data
- **Async CSV Logging Thread**: Non-blocking file I/O operations
- **Main Thread**: Application control and user interface

## Testing

```bash
# Run all tests
make test

# Run specific test
./tests/tests --gtest_filter="EMACalculatorTest.*"
```

## Documentation

```bash
# Generate HTML documentation
make docs

# Open documentation in browser
open build/docs/html/index.html
```

## Output Format

The application logs all ticker fields plus calculated EMAs to CSV:

```csv
type,sequence,product_id,price,open_24h,volume_24h,low_24h,high_24h,volume_30d,best_bid,best_ask,side,time,trade_id,last_size,price_ema,mid_price_ema,mid_price
ticker,12345,BTC-USD,50000.00,49000.00,1000.5,48000.00,51000.00,30000.0,49999.50,50000.50,buy,2024-01-01T12:00:00.000Z,67890,0.1,49950.00000000,49975.00000000,50000.00000000
```
