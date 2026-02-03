#!/bin/bash

# KFS Build Script
# Builds the kernel project
# Usage: ./scripts/build.sh

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Building KFS Project ===${NC}"
echo ""

# Check if Makefile exists
if [ ! -f Makefile ]; then
    echo -e "${RED}ERROR: Makefile not found${NC}"
    exit 1
fi

# Clean previous build
echo "Cleaning previous build..."
make clean

echo ""
echo "Building project with $(nproc) parallel jobs..."
make -j$(nproc)

echo ""
echo -e "${GREEN}✓ Build completed successfully!${NC}"
