#!/bin/bash

# KFS PR Coverage Comment Script
# Posts coverage report summary to PR
# Usage: ./scripts/comment_pr_coverage.sh <pr_number> <repo_owner> <repo_name> <coverage_percent>

# Don't exit on error - we want to try all methods
set +e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PR_NUMBER=$1
REPO_OWNER=$2
REPO_NAME=$3
COVERAGE_PERCENT=$4

if [ -z "$PR_NUMBER" ] || [ -z "$REPO_OWNER" ] || [ -z "$REPO_NAME" ]; then
    echo -e "${YELLOW}Usage: $0 <pr_number> <repo_owner> <repo_name> <coverage_percent>${NC}"
    echo "Skipping PR comment (not in PR context or missing parameters)"
    exit 0
fi

# Use provided coverage or default to N/A
if [ -z "$COVERAGE_PERCENT" ]; then
    COVERAGE_PERCENT="N/A"
fi

echo "Coverage to post: ${COVERAGE_PERCENT}%"

# Create comment body
if [ "$COVERAGE_PERCENT" = "N/A" ]; then
    COMMENT_BODY="### 📊 Code Coverage Report

✅ Coverage report generated successfully!

**Coverage:** Unable to extract percentage (see artifacts)

📁 Download the full coverage report from the workflow artifacts."
else
    COMMENT_BODY="### 📊 Code Coverage Report

✅ Coverage report generated successfully!

**Coverage:** ${COVERAGE_PERCENT}%

📁 Download the full coverage report from the workflow artifacts."
fi

# Use GitHub CLI if available, otherwise use curl
if command -v gh &> /dev/null; then
    echo -e "${YELLOW}Posting comment to PR #${PR_NUMBER} using GitHub CLI...${NC}"
    if echo "$COMMENT_BODY" | gh pr comment "$PR_NUMBER" --body-file - --repo "${REPO_OWNER}/${REPO_NAME}"; then
        echo -e "${GREEN}✓ Comment posted successfully via gh CLI${NC}"
        exit 0
    else
        echo -e "${RED}Failed to post via gh CLI, trying curl...${NC}"
    fi
fi

if [ -n "$GITHUB_TOKEN" ]; then
    echo -e "${YELLOW}Posting comment to PR #${PR_NUMBER} using curl...${NC}"
    
    # Escape the comment body for JSON
    COMMENT_JSON=$(jq -n --arg body "$COMMENT_BODY" '{body: $body}')
    
    RESPONSE=$(curl -s -w "\n%{http_code}" -X POST \
        -H "Authorization: Bearer $GITHUB_TOKEN" \
        -H "Accept: application/vnd.github.v3+json" \
        "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/issues/${PR_NUMBER}/comments" \
        -d "$COMMENT_JSON")
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    
    if [ "$HTTP_CODE" -eq 201 ]; then
        echo -e "${GREEN}✓ Comment posted successfully via curl (HTTP $HTTP_CODE)${NC}"
        exit 0
    else
        echo -e "${RED}Failed to post comment via curl (HTTP $HTTP_CODE)${NC}"
        echo "$RESPONSE"
        exit 1
    fi
else
    echo -e "${RED}ERROR: Neither gh CLI nor GITHUB_TOKEN available${NC}"
    exit 1
fi
