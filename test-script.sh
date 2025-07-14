#!/bin/bash

# NexWebserv Test Script
# This script performs basic functionality tests

echo "🧪 NexWebserv Test Suite"
echo "========================"

# Configuration
SERVER_HOST="127.0.0.1"
SERVER_PORT="8080"
SERVER_URL="http://${SERVER_HOST}:${SERVER_PORT}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

# Function to run a test
run_test() {
    local test_name="$1"
    local test_command="$2"
    local expected_status="$3"
    
    echo -n "Testing $test_name... "
    
    # Run the test command and capture the HTTP status code
    local status_code
    status_code=$(eval "$test_command" 2>/dev/null | grep -o "HTTP/1.1 [0-9]*" | awk '{print $2}')
    
    # Check if the test passed
    if [ "$status_code" = "$expected_status" ]; then
        echo -e "${GREEN}PASS${NC} ($status_code)"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}FAIL${NC} (expected: $expected_status, got: $status_code)"
        ((TESTS_FAILED++))
    fi
}

# Function to check if server is running
check_server() {
    echo "Checking if server is running on $SERVER_URL..."
    if curl -s --connect-timeout 2 "$SERVER_URL" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ Server is running${NC}"
        return 0
    else
        echo -e "${RED}✗ Server is not running or not accessible${NC}"
        echo "Please start the server with: ./Webserv config/config.conf"
        return 1
    fi
}

# Main test function
main() {
    echo
    
    # Check if server is running
    if ! check_server; then
        exit 1
    fi
    
    echo
    echo "Running HTTP method tests..."
    echo "----------------------------"
    
    # Test GET request to root
    run_test "GET /" "curl -s -I $SERVER_URL/" "200"
    
    # Test GET request to non-existent file
    run_test "GET /nonexistent" "curl -s -I $SERVER_URL/nonexistent" "404"
    
    # Test POST request (if uploads are configured)
    run_test "POST /uploads" "curl -s -I -X POST $SERVER_URL/uploads/" "200"
    
    # Test DELETE request
    run_test "DELETE /test" "curl -s -I -X DELETE $SERVER_URL/test" "404"
    
    echo
    echo "Running performance tests..."
    echo "----------------------------"
    
    # Simple load test
    echo -n "Load test (10 concurrent requests)... "
    if command -v ab > /dev/null 2>&1; then
        ab -n 10 -c 5 "$SERVER_URL/" > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}PASS${NC}"
            ((TESTS_PASSED++))
        else
            echo -e "${RED}FAIL${NC}"
            ((TESTS_FAILED++))
        fi
    else
        echo -e "${YELLOW}SKIPPED${NC} (apache2-utils not installed)"
    fi
    
    echo
    echo "Test Results:"
    echo "============="
    echo -e "Tests Passed: ${GREEN}$TESTS_PASSED${NC}"
    echo -e "Tests Failed: ${RED}$TESTS_FAILED${NC}"
    echo -e "Total Tests:  $(($TESTS_PASSED + $TESTS_FAILED))"
    
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "\n${GREEN}🎉 All tests passed!${NC}"
        exit 0
    else
        echo -e "\n${RED}❌ Some tests failed!${NC}"
        exit 1
    fi
}

# Run main function
main "$@"
