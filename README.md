# Coinbase Ticker Analyzer

A high-performance C++ application that subscribes to Coinbase ticker data via WebSockets, calculates Exponential Moving Averages (EMAs), and logs the data to CSV files.

## Features

- **Real-time Data**: WebSocket connection to Coinbase public ticker feed
- **EMA Calculations**: 5-second interval EMAs for price and mid-price
- **Multithreaded**: Concurrent data processing for optimal performance
- **CSV Logging**: Thread-safe logging with proper field escaping
- **Comprehensive Testing**: Full unit test coverage
- **Documentation**: Complete Doxygen API documentation

## Requirements

- CMake 3.16 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Git (for fetching dependencies)

## Quick Start

### Build the Project

```bash
# Clone and navigate to project directory
cd sparkland

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make -j$(nproc)

# Run tests
make test

# Generate documentation
make docs
```

### Run the Application

```bash
# Default usage (BTC-USD)
./CoinbaseTickerAnalyzer

# Custom product and output file
./CoinbaseTickerAnalyzer -p ETH-USD -o eth_data.csv

# Help
./CoinbaseTickerAnalyzer --help
```

## Architecture

The application is built with a modular, object-oriented design:

- **CoinbaseTickerAnalyzer**: Main application orchestrator
- **WebSocketClient**: Handles real-time WebSocket connections
- **JSONParser**: Processes incoming ticker messages
- **EMACalculator**: Calculates 5-second interval EMAs
- **AsyncCSVLogger**: Asynchronous CSV file logging
- **TickerData**: Data structure for ticker information

## Multithreading

The application uses multiple threads for optimal performance:

- **Main Thread**: User interface and application lifecycle
- **WebSocket I/O Thread**: Network communication
- **Data Processing Thread**: EMA calculations and logging

## Data Output

The application logs all ticker fields plus calculated EMAs to CSV format:

```csv
type,sequence,product_id,price,open_24h,volume_24h,low_24h,high_24h,volume_30d,best_bid,best_ask,side,time,trade_id,last_size,price_ema,mid_price_ema,mid_price
ticker,12345,BTC-USD,50000.00,49000.00,1000.5,48000.00,51000.00,30000.0,49999.50,50000.50,buy,2024-01-01T12:00:00.000Z,67890,0.1,49950.00000000,49975.00000000,50000.00000000
```

## Testing

Run the comprehensive test suite:

```bash
cd build
make test
```

Tests cover:
- Unit tests for all components
- Thread safety verification
- Error handling scenarios
- Performance benchmarks

## Documentation

Generate API documentation:

```bash
cd build
make docs
```

Documentation will be available in `build/docs/html/index.html`.

## Configuration

### Command Line Options

- `-p, --product <ID>`: Product ID to analyze (default: BTC-USD)
- `-o, --output <file>`: Output CSV filename (default: ticker_data.csv)
- `-h, --help`: Show help message

### Supported Products

Any valid Coinbase trading pair:
- BTC-USD, ETH-USD, LTC-USD
- BTC-EUR, ETH-EUR
- And many more...

## Performance

The application is optimized for high-frequency trading scenarios:

- **Low Latency**: Sub-second data processing
- **High Throughput**: Efficient multithreaded processing
- **Memory Efficient**: Minimal memory footprint
- **CPU Optimized**: Lock-free operations where possible

## Error Handling

Robust error handling throughout:

- **Network Issues**: Automatic reconnection attempts
- **Data Corruption**: Graceful handling of malformed JSON
- **File I/O**: Proper error reporting for disk issues
- **Resource Cleanup**: RAII principles for automatic cleanup

## Development

### Project Structure

```
sparkland/
├── include/           # Header files
├── src/              # Source files
├── tests/            # Unit tests
├── third_party/      # External dependencies
├── CMakeLists.txt    # Build configuration
├── Doxyfile.in       # Documentation config
└── README.md         # This file
```

### Adding New Features

1. Add new classes to `include/` and `src/`
2. Update `CMakeLists.txt` with new sources
3. Add unit tests in `tests/`
4. Update documentation with Doxygen comments

## License

This project is developed as part of an HFT developer exercise.

## Contributing

1. Follow the existing code style
2. Add comprehensive tests for new features
3. Update documentation
4. Ensure thread safety for concurrent operations

## Troubleshooting

### Common Issues

**Build fails with dependency errors:**
```bash
# Ensure you have internet connection for dependency fetching
# Try clearing build directory and rebuilding
rm -rf build && mkdir build && cd build && cmake .. && make
```

**WebSocket connection fails:**
- Check internet connectivity
- Verify Coinbase WebSocket endpoint is accessible
- Ensure firewall allows WebSocket connections

**CSV file not created:**
- Check write permissions in output directory
- Verify disk space availability
- Check for file path issues

### Performance Tuning

- Adjust EMA interval in `EMACalculator` constructor
- Modify queue size in `CoinbaseTickerAnalyzer`
- Tune CSV flush frequency in `AsyncCSVLogger`

For more detailed information, see `IMPLEMENTATION_STEPS.txt`.
