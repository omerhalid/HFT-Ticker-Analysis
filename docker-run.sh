#!/bin/bash

# Docker run script 

set -e

# Build and run
docker build -t coinbase-ticker-analyzer .
docker run --rm coinbase-ticker-analyzer "$@"
