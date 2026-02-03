#!/bin/bash

# KFS Code Style Checker
# Runs checkpatch.pl on all C source files
# Usage: ./style_check.sh

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== KFS Code Style Checker ===${NC}"
echo ""

# Check if checkpatch.pl exists
if [ ! -f ./scripts/checkpatch.pl ]; then
    echo -e "${RED}ERROR: checkpatch.pl not found in scripts/ directory${NC}"
    exit 1
fi

# Make checkpatch.pl executable
chmod +x ./scripts/checkpatch.pl

# Find all C files
C_FILES=$(find src -name "*.c" 2>/dev/null)

if [ -z "$C_FILES" ]; then
    echo -e "${YELLOW}No C files found in src/ directory${NC}"
    exit 0
fi

# Track if any errors were found
HAS_ERRORS=0

# Check each file
echo -e "${YELLOW}Checking C source files:${NC}"
for file in $C_FILES; do
    echo ""
    echo -e "${YELLOW}Checking: $file${NC}"
    if ./scripts/checkpatch.pl --no-tree --file "$file"; then
        echo -e "${GREEN}✓ $file passed${NC}"
    else
        echo -e "${RED}✗ $file has style issues${NC}"
        HAS_ERRORS=1
    fi
done

echo ""
echo -e "${YELLOW}=== Style Check Complete ===${NC}"

if [ $HAS_ERRORS -eq 0 ]; then
    echo -e "${GREEN}All files passed style checks!${NC}"
    exit 0
else
    echo -e "${RED}Some files have style issues. Please fix them.${NC}"
    exit 1
fi
