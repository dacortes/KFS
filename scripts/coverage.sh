#!/bin/bash

# KFS Code Coverage Script
# Generates code coverage report for unit tests using gcov and lcov
# Usage: ./coverage.sh

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
COVERAGE_DIR="coverage_report"
MIN_COVERAGE=80

echo -e "${YELLOW}=== KFS Code Coverage Generator ===${NC}"

# Clean previous coverage report
echo -e "${YELLOW}[1/4] Cleaning previous coverage reports...${NC}"
if [ -d "$COVERAGE_DIR" ]; then
    rm -rf "$COVERAGE_DIR"
fi

# Check for lcov
echo -e "${YELLOW}[2/4] Checking dependencies...${NC}"
if ! command -v lcov &> /dev/null; then
    echo -e "${RED}ERROR: lcov is not installed${NC}"
    echo "Install with: sudo apt-get install lcov"
    exit 1
fi
if ! command -v gcov &> /dev/null; then
    echo -e "${RED}ERROR: gcov is not installed${NC}"
    echo "Install with: sudo apt-get install gcov"
    exit 1
fi

# Build with coverage instrumentation
echo -e "${YELLOW}[3/4] Building with coverage instrumentation...${NC}"

make clean
COVERAGE=1 make test-runner

# Run tests
echo -e "${YELLOW}[4/4] Running tests and generating coverage...${NC}"
if [ -f "./test_runner" ]; then
    ./test_runner || {
        echo -e "${RED}ERROR: Tests failed${NC}"
        exit 1
    }
else
    echo -e "${RED}ERROR: test_runner executable not found${NC}"
    exit 1
fi

# Generate coverage report
mkdir -p "$COVERAGE_DIR"

# Initialize lcov
lcov --directory . --capture --output-file "$COVERAGE_DIR/coverage.info" \
    --rc lcov_branch_coverage=1 --no-external 2>/dev/null || true

# Filter out test files and system headers
lcov --remove "$COVERAGE_DIR/coverage.info" \
    '*/test*' \
    '/usr/*' \
    --output-file "$COVERAGE_DIR/coverage.info" \
    --rc lcov_branch_coverage=1 2>/dev/null || true

# Generate HTML report
genhtml "$COVERAGE_DIR/coverage.info" \
    --output-directory "$COVERAGE_DIR/html" \
    --rc lcov_branch_coverage=1 \
    --title "KFS Code Coverage Report" 2>/dev/null || true

# Extract coverage percentage
COVERAGE_PERCENT=$(lcov --summary "$COVERAGE_DIR/coverage.info" 2>&1 | \
    grep "lines" | awk '{print $2}' | sed 's/%//' || echo "0")

# Print summary
echo ""
echo -e "${GREEN}=== Coverage Report Generated ===${NC}"
echo "Report location: $COVERAGE_DIR/"
echo "HTML report: $COVERAGE_DIR/html/index.html"
echo ""

# Display coverage percentage
# Use awk for comparison to avoid bc syntax errors
if [ -n "$COVERAGE_PERCENT" ] && awk "BEGIN {exit !($COVERAGE_PERCENT >= $MIN_COVERAGE)}"; then
    echo -e "${GREEN}Overall Coverage: ${COVERAGE_PERCENT}% (Target: ${MIN_COVERAGE}%)${NC}"
else
    echo -e "${RED}Overall Coverage: ${COVERAGE_PERCENT}% (Target: ${MIN_COVERAGE}%)${NC}"
    echo -e "${RED}WARNING: Coverage below minimum threshold!${NC}"
fi

echo ""
echo "To view HTML report:"
echo "  firefox $COVERAGE_DIR/html/index.html"
echo "  or open it in your browser"
echo ""

# Detailed coverage by file
echo -e "${YELLOW}=== Coverage by File ===${NC}"
lcov --summary "$COVERAGE_DIR/coverage.info" 2>/dev/null || true

# Cleanup: keep reports but remove intermediate build artifacts
echo ""
echo -e "${YELLOW}Cleaning up intermediate files...${NC}"
find . -name "*.gcda" -delete
find . -name "*.gcno" -delete

echo -e "${GREEN}Coverage generation complete!${NC}"

# Post to PR if running in CI with PR context
if [ -n "$PR_NUMBER" ] && [ -n "$REPO_OWNER" ] && [ -n "$REPO_NAME" ]; then
    echo ""
    echo -e "${YELLOW}Posting coverage to PR...${NC}"
    if [ -f ./scripts/comment_pr_coverage.sh ]; then
        chmod +x ./scripts/comment_pr_coverage.sh
        ./scripts/comment_pr_coverage.sh "$PR_NUMBER" "$REPO_OWNER" "$REPO_NAME" "$COVERAGE_PERCENT" || echo "Failed to post to PR, continuing..."
    fi
fi

exit 0
