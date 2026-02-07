#!/bin/bash

# KFS Static Analysis
# Runs cppcheck on source files
# Usage: ./scripts/static_analysis.sh

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== KFS Static Analysis (cppcheck) ===${NC}"
echo ""

# Check if cppcheck is installed
if ! command -v cppcheck &> /dev/null; then
    echo -e "${RED}ERROR: cppcheck is not installed${NC}"
    echo "Install with: sudo apt-get install cppcheck"
    exit 1
fi

# Check if src directory exists
if [ ! -d src ]; then
    echo -e "${RED}ERROR: src/ directory not found${NC}"
    exit 1
fi

echo "Running cppcheck on src/ directory..."
echo ""

# Run cppcheck with appropriate flags
if cppcheck --enable=warning,style,performance,portability --error-exitcode=1 src/; then
    echo ""
    echo -e "${GREEN}✓ Static analysis passed!${NC}"
    exit 0
else
    echo ""
    echo -e "${RED}✗ Static analysis found issues${NC}"
    exit 1
fi
