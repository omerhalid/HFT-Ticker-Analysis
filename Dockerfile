FROM ubuntu:22.04

# Install dependencies and build
RUN apt-get update && apt-get install -y \
    build-essential cmake libssl-dev libwebsockets-dev libcurl4-openssl-dev pkg-config git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN ./build.sh

# Create output directory
RUN mkdir -p /app/output

# Default command
ENTRYPOINT ["./build/CoinbaseTickerAnalyzer"]
CMD ["-o", "/app/output/ticker_data.csv"]
