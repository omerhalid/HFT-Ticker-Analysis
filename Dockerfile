FROM ubuntu:22.04

# Install dependencies and build
RUN apt-get update && apt-get install -y \
    build-essential cmake libssl-dev libwebsockets-dev pkg-config git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN ./build.sh

# Default command
CMD ["./build/CoinbaseTickerAnalyzer"]
