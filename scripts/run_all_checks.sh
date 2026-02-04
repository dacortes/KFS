#!/bin/bash

# KFS All Checks Runner
# Runs all quality checks: style, static analysis, tests, and coverage
# Usage: ./scripts/run_all_checks.sh

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Change to project root
cd "$PROJECT_ROOT"

# Track overall success
OVERALL_SUCCESS=true

echo -e "${BLUE}╔══════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║         KFS Quality Checks - Running All Checks          ║${NC}"
echo -e "${BLUE}╚══════════════════════════════════════════════════════════╝${NC}"
echo ""

# Function to run a check
run_check() {
    local name="$1"
    local script="$2"
    
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}Running: ${name}${NC}"
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    if [ -f "$script" ]; then
        if bash "$script"; then
            echo -e "${GREEN}✓ ${name} passed${NC}"
            echo ""
            return 0
        else
            echo -e "${RED}✗ ${name} failed${NC}"
            echo ""
            OVERALL_SUCCESS=false
            return 1
        fi
    else
        echo -e "${RED}✗ Script not found: $script${NC}"
        echo ""
        OVERALL_SUCCESS=false
        return 1
    fi
}

# Run all checks
run_check "Style Check" "./scripts/style_check.sh" || true
run_check "Static Analysis" "./scripts/static_analysis.sh" || true
run_check "Build" "./scripts/build.sh" || true
run_check "Code Coverage" "./scripts/coverage.sh" || true

# Final summary
echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
if [ "$OVERALL_SUCCESS" = true ]; then
    echo -e "${GREEN}╔══════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║                  ALL CHECKS PASSED ✓                     ║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════════════════╝${NC}"
    exit 0
else
    echo -e "${RED}╔══════════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║              SOME CHECKS FAILED ✗                        ║${NC}"
    echo -e "${RED}╚══════════════════════════════════════════════════════════╝${NC}"
    exit 1
fi
