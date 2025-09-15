#!/bin/bash

# Docker run script 

set -e

# Create output directory
mkdir -p output

# Build and run
docker build -t coinbase-ticker-analyzer .
docker run --rm -v "$(pwd)/output:/app/output" coinbase-ticker-analyzer "$@"
