#!/bin/bash

# KFS PR Coverage Comment Script
# Posts coverage report summary to PR
# Usage: ./scripts/comment_pr_coverage.sh <pr_number> <repo_owner> <repo_name>

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PR_NUMBER=$1
REPO_OWNER=$2
REPO_NAME=$3

if [ -z "$PR_NUMBER" ] || [ -z "$REPO_OWNER" ] || [ -z "$REPO_NAME" ]; then
    echo -e "${YELLOW}Usage: $0 <pr_number> <repo_owner> <repo_name>${NC}"
    echo "Skipping PR comment (not in PR context or missing parameters)"
    exit 0
fi

# Check if coverage report exists
if [ ! -f coverage_report/coverage.info ]; then
    echo -e "${YELLOW}Coverage report not found, skipping PR comment${NC}"
    exit 0
fi

# Extract coverage percentage
COVERAGE_PERCENT=$(lcov --summary coverage_report/coverage.info 2>&1 | \
    grep "lines" | awk '{print $2}' | sed 's/%//' || echo "0")

# Create comment body
COMMENT_BODY="### 📊 Code Coverage Report

✅ Coverage report generated successfully!

**Coverage:** ${COVERAGE_PERCENT}%

📁 Download the full coverage report from the workflow artifacts."

# Use GitHub CLI if available, otherwise use curl
if command -v gh &> /dev/null; then
    echo -e "${YELLOW}Posting comment to PR #${PR_NUMBER} using GitHub CLI...${NC}"
    echo "$COMMENT_BODY" | gh pr comment "$PR_NUMBER" --body-file - --repo "${REPO_OWNER}/${REPO_NAME}"
    echo -e "${GREEN}✓ Comment posted successfully${NC}"
elif [ -n "$GITHUB_TOKEN" ]; then
    echo -e "${YELLOW}Posting comment to PR #${PR_NUMBER} using curl...${NC}"
    COMMENT_JSON=$(jq -n --arg body "$COMMENT_BODY" '{body: $body}')
    
    curl -X POST \
        -H "Authorization: token $GITHUB_TOKEN" \
        -H "Accept: application/vnd.github.v3+json" \
        "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/issues/${PR_NUMBER}/comments" \
        -d "$COMMENT_JSON"
    
    echo -e "${GREEN}✓ Comment posted successfully${NC}"
else
    echo -e "${YELLOW}Neither gh CLI nor GITHUB_TOKEN available, skipping PR comment${NC}"
    exit 0
fi
