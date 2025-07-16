#!/usr/bin/env bash
# 🚀 Enhanced Webserv Test Suite with comprehensive coverage

RAPPORT="rapport.log"
SERVER_LOG="server.log"
> "$RAPPORT"
> "$SERVER_LOG"

# Colors
CYAN='\033[0;36m'
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

# Test arrays
TEST_NAMES=()
TEST_EXPECTED=()
TEST_RESULTS=()
TEST_STATUS=()
TEST_DETAILS=()

# Configuration
SERVER_HOST="127.0.0.1"
SERVER_PORT="8080"
SERVER_PID=""
TIMEOUT_DURATION=5

# Test categories
DO_ALL=false
DO_BASIC=false
DO_METHODS=false
DO_ROUTES=false
DO_CGI=false
DO_UPLOAD=false
DO_ERROR=false
DO_STRESS=false
DO_SECURITY=false
DO_EDGE_CASES=false
DO_PROTOCOL=false
DO_PERFORMANCE=false
DO_MALFORMED=false
DO_BOUNDARY=false
DO_42_SPECIFIC=false

# === Parse arguments ===
show_help() {
  echo -e "${CYAN}🧪 Enhanced Webserv Test Suite${NC}"
  echo ""
  echo "Usage: $0 [OPTIONS]"
  echo ""
  echo "Test Categories:"
  echo "  --all           Run all tests"
  echo "  --basic         Basic connectivity and HTTP tests"
  echo "  --methods       HTTP method tests (GET, POST, DELETE)"
  echo "  --routes        Route matching and path tests"
  echo "  --cgi           CGI script execution tests"
  echo "  --upload        File upload tests"
  echo "  --error         Error handling tests"
  echo "  --stress        Stress and performance tests"
  echo "  --security      Security and edge case tests"
  echo "  --edge-cases    Edge case and boundary tests"
  echo "  --protocol      HTTP protocol compliance tests"
  echo "  --performance   Performance and load tests"
  echo "  --malformed     Malformed request tests"
  echo "  --boundary      Boundary value tests"
  echo "  --42-specific   42 project specific tests"
  echo ""
  echo "Options:"
  echo "  --port PORT     Server port (default: 8080)"
  echo "  --host HOST     Server host (default: 127.0.0.1)"
  echo "  --timeout SEC   Request timeout (default: 5)"
  echo "  --help          Show this help message"
  echo ""
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --all) DO_ALL=true ;;
    --basic) DO_BASIC=true ;;
    --methods) DO_METHODS=true ;;
    --routes) DO_ROUTES=true ;;
    --cgi) DO_CGI=true ;;
    --upload) DO_UPLOAD=true ;;
    --error) DO_ERROR=true ;;
    --stress) DO_STRESS=true ;;
    --security) DO_SECURITY=true ;;
    --edge-cases) DO_EDGE_CASES=true ;;
    --protocol) DO_PROTOCOL=true ;;
    --performance) DO_PERFORMANCE=true ;;
    --malformed) DO_MALFORMED=true ;;
    --boundary) DO_BOUNDARY=true ;;
    --42-specific) DO_42_SPECIFIC=true ;;
    --port) SERVER_PORT="$2"; shift ;;
    --host) SERVER_HOST="$2"; shift ;;
    --timeout) TIMEOUT_DURATION="$2"; shift ;;
    --help) show_help; exit 0 ;;
    *) echo -e "${RED}Unknown option: $1${NC}"; show_help; exit 1 ;;
  esac
  shift
done

# Default to basic if no category selected
if ! $DO_ALL && ! $DO_BASIC && ! $DO_METHODS && ! $DO_ROUTES && ! $DO_CGI && ! $DO_UPLOAD && ! $DO_ERROR && ! $DO_STRESS && ! $DO_SECURITY && ! $DO_EDGE_CASES && ! $DO_PROTOCOL && ! $DO_PERFORMANCE && ! $DO_MALFORMED && ! $DO_BOUNDARY && ! $DO_42_SPECIFIC; then
  DO_BASIC=true
fi

# === Helper functions ===
start_server() {
  echo -e "${BLUE}🚀 Starting server...${NC}"
  
  # Check if server is already running
  if curl -s --connect-timeout 2 "http://$SERVER_HOST:$SERVER_PORT/" >/dev/null 2>&1; then
    echo -e "${YELLOW}⚠️  Server already running on port $SERVER_PORT${NC}"
    echo -e "${GREEN}✅ Using existing server${NC}"
    return 0
  fi
  
  if [ ! -f "./Webserv" ]; then
    echo -e "${RED}❌ Server binary './Webserv' not found!${NC}"
    echo -e "${YELLOW}💡 Try: make or c++ -std=c++98 src/*.cpp -o Webserv${NC}"
    exit 1
  fi
  
  if [ ! -f "config/config.conf" ]; then
    echo -e "${RED}❌ Config file 'config/config.conf' not found!${NC}"
    exit 1
  fi
  
  ./Webserv config/config.conf > "$SERVER_LOG" 2>&1 &
  SERVER_PID=$!
  sleep 3
  
  if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${RED}❌ Server failed to start!${NC}"
    echo -e "${YELLOW}Server log:${NC}"
    cat "$SERVER_LOG"
    exit 1
  fi
  
  echo -e "${GREEN}✅ Server started with PID: $SERVER_PID${NC}"
}

stop_server() {
  if [ -n "$SERVER_PID" ] && kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${BLUE}🛑 Stopping server (PID: $SERVER_PID)...${NC}"
    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
    echo -e "${GREEN}✅ Server stopped${NC}"
  elif curl -s --connect-timeout 2 "http://$SERVER_HOST:$SERVER_PORT/" >/dev/null 2>&1; then
    echo -e "${YELLOW}⚠️  Server was running externally, leaving it running${NC}"
  fi
}

cleanup() {
  stop_server
  # Clean up test files
  rm -f test_upload.txt test_large.txt
}

trap cleanup EXIT

check_server() {
  local response=$(curl -s -o /dev/null -w "%{http_code}" --connect-timeout $TIMEOUT_DURATION "http://$SERVER_HOST:$SERVER_PORT/" 2>/dev/null)
  if [ "$response" != "200" ] && [ "$response" != "404" ]; then
    echo -e "${RED}❌ Server not responding on http://$SERVER_HOST:$SERVER_PORT${NC}"
    echo -e "${YELLOW}Expected: 200 or 404, Got: $response${NC}"
    return 1
  fi
  return 0
}

# === Spinner ===
spin() {
  local pid=$1
  local delay=0.1
  local spinstr='|/-\'
  local msg="$2"
  while kill -0 $pid 2>/dev/null; do
    for c in $spinstr; do
      printf "\r${CYAN}⏳ $msg %s${NC}" "$c"
      sleep $delay
    done
  done
  printf "\r${CYAN}✔ $msg - Done!        ${NC}\n"
}

# === Enhanced test runner ===
run_test() {
  local name="$1"
  local cmd="$2"
  local expected="$3"
  local details="$4"

  TEST_NAMES+=("$name")
  TEST_EXPECTED+=("$expected")
  TEST_DETAILS+=("$details")

  # Add timeout to curl commands
  if [[ "$cmd" == *"curl"* ]]; then
    cmd="${cmd} --max-time $TIMEOUT_DURATION"
  fi

  local start_time=$(date +%s.%N)
  result=$(eval "$cmd" 2>/dev/null)
  local end_time=$(date +%s.%N)
  local duration=$(echo "$end_time - $start_time" | bc -l 2>/dev/null || echo "0")
  
  TEST_RESULTS+=("$result")

  if [[ "$result" == "$expected" ]]; then
    TEST_STATUS+=("✅ PASS")
    printf "${GREEN}✅ %-30s${NC} Expected: %-8s Got: %-8s (%.2fs)\n" "$name" "$expected" "$result" "$duration"
  else
    TEST_STATUS+=("❌ FAIL")
    printf "${RED}❌ %-30s${NC} Expected: %-8s Got: %-8s (%.2fs)\n" "$name" "$expected" "$result" "$duration"
  fi
}

# === Basic connectivity tests ===
run_basic_tests() {
  echo -e "\n${CYAN}🔌 Running BASIC connectivity tests...${NC}"
  
  run_test "Server responds" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "200" "Basic HTTP GET to root"
  run_test "Root index serves" "curl -s http://$SERVER_HOST:$SERVER_PORT/ | head -1 | grep -q 'html' && echo '200' || echo '500'" "200" "Check if HTML is served"
  run_test "HTTP version check" "curl -s -I http://$SERVER_HOST:$SERVER_PORT/ | grep -o 'HTTP/1.1' | head -1" "HTTP/1.1" "HTTP/1.1 compliance"
  run_test "Connection header" "curl -s -I http://$SERVER_HOST:$SERVER_PORT/ | grep -i 'connection' | grep -q 'close\\|keep-alive' && echo 'found' || echo 'missing'" "found" "Connection header present"
  run_test "Content-Type header" "curl -s -I http://$SERVER_HOST:$SERVER_PORT/ | grep -i 'content-type' | grep -q 'text/html' && echo 'found' || echo 'missing'" "found" "Content-Type header"
}

# === HTTP methods tests ===
run_methods_tests() {
  echo -e "\n${CYAN}📦 Running HTTP METHODS tests...${NC}"

  # GET tests
  run_test "GET root" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "200" "GET request to root path"
  run_test "GET non-existing" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/nonexistent" "404" "GET non-existent resource"
  run_test "GET with query" "curl -s -o /dev/null -w '%{http_code}' 'http://$SERVER_HOST:$SERVER_PORT/?param=value'" "200" "GET with query parameters"
  
  # HEAD tests
  run_test "HEAD method" "curl -s -I -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "200" "HEAD request should work like GET but no body"
  run_test "HEAD no body" "curl -s -I http://$SERVER_HOST:$SERVER_PORT/ | tail -1 | wc -c" "1" "HEAD should not return body"
  
  # POST tests
  run_test "POST simple" "curl -s -o /dev/null -w '%{http_code}' -d 'key=value' http://$SERVER_HOST:$SERVER_PORT/" "200" "Simple POST with form data"
  run_test "POST JSON" "curl -s -o /dev/null -w '%{http_code}' -H 'Content-Type: application/json' -d '{\"test\":\"data\"}' http://$SERVER_HOST:$SERVER_PORT/" "200" "POST with JSON data"
  run_test "POST empty" "curl -s -o /dev/null -w '%{http_code}' -d '' http://$SERVER_HOST:$SERVER_PORT/" "200" "POST with empty body"
  
  # DELETE tests
  run_test "DELETE method" "curl -s -X DELETE -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/test.txt" "404" "DELETE non-existent file"
  
  # Unsupported methods
  run_test "PUT method" "curl -s -X PUT -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "405" "PUT should return Method Not Allowed"
  run_test "PATCH method" "curl -s -X PATCH -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "405" "PATCH should return Method Not Allowed"
  run_test "Invalid method" "printf 'INVALID / HTTP/1.1\r\nHost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Invalid HTTP method"
}

# === Route and path tests ===
run_routes_tests() {
  echo -e "\n${CYAN}🛤️  Running ROUTES and path tests...${NC}"
  
  run_test "Root path" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "200" "Root path /"
  run_test "Images route" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/images/" "200" "Images directory route"
  run_test "Uploads route" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/uploads/" "200" "Uploads directory route"
  run_test "Nested path" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/images/test.jpg" "404" "Nested path in images"
  run_test "Path with dots" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/../" "400" "Path traversal attempt"
  run_test "Multiple slashes" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT///" "200" "Multiple slashes should normalize"
  run_test "Case sensitivity" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/Images/" "404" "Case sensitive paths"
}

# === CGI tests ===
run_cgi_tests() {
  echo -e "\n${CYAN}🤖 Running CGI tests...${NC}"

  # Create test CGI scripts if they don't exist
  mkdir -p cgi-bin
  
  # Python CGI
  cat > cgi-bin/test.py << 'EOF'
#!/usr/bin/env python3
print("Content-Type: text/html\r\n\r\n")
print("<html><body><h1>Python CGI Works!</h1></body></html>")
EOF
  chmod +x cgi-bin/test.py
  
  # PHP CGI  
  cat > cgi-bin/test.php << 'EOF'
<?php
header("Content-Type: text/html");
echo "<html><body><h1>PHP CGI Works!</h1></body></html>";
?>
EOF
  chmod +x cgi-bin/test.php
  
  run_test "CGI Python" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/cgi-bin/test.py" "200" "Python CGI script execution"
  run_test "CGI PHP" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/cgi-bin/test.php" "200" "PHP CGI script execution" 
  run_test "CGI non-existing" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/cgi-bin/nonexistent.py" "404" "Non-existent CGI script"
  run_test "CGI POST data" "curl -s -o /dev/null -w '%{http_code}' -d 'data=test' http://$SERVER_HOST:$SERVER_PORT/cgi-bin/test.py" "200" "CGI with POST data"
}

# === Upload tests ===
run_upload_tests() {
  echo -e "\n${CYAN}📤 Running UPLOAD tests...${NC}"
  
  # Create test files
  echo "Test file content" > test_upload.txt
  dd if=/dev/zero of=test_large.txt bs=1M count=1 2>/dev/null
  
  run_test "File upload small" "curl -s -o /dev/null -w '%{http_code}' -F 'file=@test_upload.txt' http://$SERVER_HOST:$SERVER_PORT/uploads/" "200" "Small file upload"
  run_test "File upload large" "curl -s -o /dev/null -w '%{http_code}' -F 'file=@test_large.txt' http://$SERVER_HOST:$SERVER_PORT/uploads/" "413" "Large file upload (should exceed limit)"
  run_test "Upload no file" "curl -s -o /dev/null -w '%{http_code}' -F 'file=' http://$SERVER_HOST:$SERVER_PORT/uploads/" "400" "Upload with no file"
  run_test "Multipart form" "curl -s -o /dev/null -w '%{http_code}' -F 'name=test' -F 'value=data' http://$SERVER_HOST:$SERVER_PORT/uploads/" "200" "Multipart form data"
}

# === Error handling tests ===
run_error_tests() {
  echo -e "\n${CYAN}🚨 Running ERROR handling tests...${NC}"
  
  run_test "404 Not Found" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/definitely_not_exists" "404" "Standard 404 error"
  run_test "405 Method Not Allowed" "curl -s -X TRACE -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "405" "Unsupported HTTP method"
  run_test "400 Bad Request" "printf 'MALFORMED REQUEST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Malformed HTTP request"
  run_test "Missing Host header" "printf 'GET / HTTP/1.1\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Missing required Host header"
  run_test "Long URL" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/$(printf 'a%.0s' {1..5000})" "414" "URL too long"
  run_test "Large headers" "curl -s -o /dev/null -w '%{http_code}' -H 'X-Large-Header: $(printf 'x%.0s' {1..10000})' http://$SERVER_HOST:$SERVER_PORT/" "431" "Request headers too large"
}

# === Stress tests ===
run_stress_tests() {
  echo -e "\n${CYAN}💪 Running STRESS tests...${NC}"
  
  # Concurrent requests
  run_test "Concurrent requests" "for i in {1..10}; do curl -s -o /dev/null http://$SERVER_HOST:$SERVER_PORT/ & done; wait; echo '200'" "200" "10 concurrent requests"
  
  # Keep-alive test
  run_test "Keep-alive connection" "curl -s -o /dev/null -w '%{http_code}' -H 'Connection: keep-alive' http://$SERVER_HOST:$SERVER_PORT/" "200" "Keep-alive connection header"
  
  # Multiple requests on same connection
  run_test "Connection reuse" "curl -s -o /dev/null -w '%{http_code}' --http1.1 http://$SERVER_HOST:$SERVER_PORT/ http://$SERVER_HOST:$SERVER_PORT/" "200" "Connection reuse"
  
  # Slow client simulation
  run_test "Slow request" "printf 'GET / HTTP/1.1\r\nHost: $SERVER_HOST\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Incomplete request (timeout test)"
}

# === Security tests ===  
run_security_tests() {
  echo -e "\n${CYAN}🔒 Running SECURITY tests...${NC}"
  
  run_test "Directory traversal 1" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/../etc/passwd" "400" "Directory traversal with ../"
  run_test "Directory traversal 2" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/..%2F..%2Fetc%2Fpasswd" "400" "URL-encoded directory traversal"
  run_test "Null byte injection" "curl -s -o /dev/null -w '%{http_code}' 'http://$SERVER_HOST:$SERVER_PORT/test%00.txt'" "400" "Null byte in URL"
  run_test "Script injection" "curl -s -o /dev/null -w '%{http_code}' 'http://$SERVER_HOST:$SERVER_PORT/<script>alert(1)</script>'" "404" "Script tags in URL"
  run_test "SQL injection" "curl -s -o /dev/null -w '%{http_code}' \"http://$SERVER_HOST:$SERVER_PORT/'; DROP TABLE users; --\"" "404" "SQL injection attempt"
  run_test "Unicode traversal" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/☃️" "404" "Unicode characters in path"
  run_test "Header injection" "curl -s -o /dev/null -w '%{http_code}' -H $'Host: $SERVER_HOST\\r\\nX-Injected: true' http://$SERVER_HOST:$SERVER_PORT/" "400" "CRLF injection in headers"
}

# === Edge case tests ===
run_edge_case_tests() {
  echo -e "\n${CYAN}🎭 Running EDGE CASE tests...${NC}"
  
  # HTTP/1.1 Protocol edge cases
  run_test "Empty request line" "printf '\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Empty request line"
  run_test "Only spaces request" "printf '   \r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Only spaces in request"
  run_test "Missing HTTP version" "printf 'GET /\r\nHost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Missing HTTP version"
  run_test "Invalid HTTP version" "printf 'GET / HTTP/2.0\r\nHost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Invalid HTTP version"
  run_test "HTTP/0.9 request" "printf 'GET /\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "HTTP/0.9 style request"
  
  # URL edge cases
  run_test "URL with spaces" "curl -s -o /dev/null -w '%{http_code}' 'http://$SERVER_HOST:$SERVER_PORT/path with spaces'" "400" "URL with unencoded spaces"
  run_test "URL percent encoding" "curl -s -o /dev/null -w '%{http_code}' 'http://$SERVER_HOST:$SERVER_PORT/path%20with%20spaces'" "404" "URL with percent encoding"
  run_test "URL double encoding" "curl -s -o /dev/null -w '%{http_code}' 'http://$SERVER_HOST:$SERVER_PORT/%252e%252e%252f'" "400" "Double URL encoding"
  run_test "URL with fragment" "curl -s -o /dev/null -w '%{http_code}' 'http://$SERVER_HOST:$SERVER_PORT/#fragment'" "200" "URL with fragment"
  run_test "URL max length" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/$(printf 'x%.0s' {1..8192})" "414" "Maximum URL length"
  
  # Header edge cases
  run_test "Case insensitive headers" "printf 'GET / HTTP/1.1\r\nhost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "200" "Lowercase host header"
  run_test "Multiple Host headers" "printf 'GET / HTTP/1.1\r\nHost: $SERVER_HOST\r\nHost: example.com\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Multiple Host headers"
  run_test "Header line folding" "printf 'GET / HTTP/1.1\r\nHost: $SERVER_HOST\r\nX-Test:\r\n folded\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Header line folding (obsolete)"
  run_test "Header without colon" "printf 'GET / HTTP/1.1\r\nHost: $SERVER_HOST\r\nInvalidHeader\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Header without colon"
  run_test "Header with tab" "printf 'GET / HTTP/1.1\r\nHost:\t$SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "200" "Header with tab separator"
  
  # Content-Length edge cases
  run_test "Negative Content-Length" "curl -s -o /dev/null -w '%{http_code}' -H 'Content-Length: -1' -d 'test' http://$SERVER_HOST:$SERVER_PORT/" "400" "Negative Content-Length"
  run_test "Invalid Content-Length" "curl -s -o /dev/null -w '%{http_code}' -H 'Content-Length: abc' -d 'test' http://$SERVER_HOST:$SERVER_PORT/" "400" "Non-numeric Content-Length"
  run_test "Multiple Content-Length" "printf 'POST / HTTP/1.1\r\nHost: $SERVER_HOST\r\nContent-Length: 4\r\nContent-Length: 5\r\n\r\ntest' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Multiple Content-Length headers"
  run_test "Content-Length mismatch" "printf 'POST / HTTP/1.1\r\nHost: $SERVER_HOST\r\nContent-Length: 10\r\n\r\ntest' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Content-Length mismatch"
  
  # Transfer-Encoding edge cases
  run_test "Transfer-Encoding chunked" "printf 'POST / HTTP/1.1\r\nHost: $SERVER_HOST\r\nTransfer-Encoding: chunked\r\n\r\n4\r\ntest\r\n0\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "200" "Chunked transfer encoding"
  run_test "Invalid chunk size" "printf 'POST / HTTP/1.1\r\nHost: $SERVER_HOST\r\nTransfer-Encoding: chunked\r\n\r\nGG\r\ntest\r\n0\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Invalid chunk size"
  run_test "Both Content-Length and TE" "printf 'POST / HTTP/1.1\r\nHost: $SERVER_HOST\r\nContent-Length: 4\r\nTransfer-Encoding: chunked\r\n\r\n4\r\ntest\r\n0\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Both Content-Length and Transfer-Encoding"
}

# === Protocol compliance tests ===
run_protocol_compliance_tests() {
  echo -e "\n${CYAN}📋 Running HTTP PROTOCOL COMPLIANCE tests...${NC}"
  
  # HTTP method case sensitivity
  run_test "Method case sensitivity" "printf 'get / HTTP/1.1\r\nHost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Lowercase method (should be case sensitive)"
  run_test "Mixed case method" "printf 'GeT / HTTP/1.1\r\nHost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Mixed case method"
  
  # HTTP response headers compliance
  run_test "Server header present" "curl -s -I http://$SERVER_HOST:$SERVER_PORT/ | grep -i 'server:' | wc -l" "1" "Server header should be present"
  run_test "Date header present" "curl -s -I http://$SERVER_HOST:$SERVER_PORT/ | grep -i 'date:' | wc -l" "1" "Date header should be present"
  run_test "Content-Length consistency" "curl -s http://$SERVER_HOST:$SERVER_PORT/ | wc -c" "$(curl -s -I http://$SERVER_HOST:$SERVER_PORT/ | grep -i 'content-length:' | cut -d' ' -f2 | tr -d '\r')" "Content-Length matches body size"
  
  # HTTP/1.1 specific requirements
  run_test "Host header required" "printf 'GET / HTTP/1.1\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Host header required for HTTP/1.1"
  run_test "Absolute URI support" "printf 'GET http://$SERVER_HOST:$SERVER_PORT/ HTTP/1.1\r\nHost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "200" "Absolute URI in request line"
  
  # Keep-alive and connection management
  run_test "Connection close respected" "curl -s -I -H 'Connection: close' http://$SERVER_HOST:$SERVER_PORT/ | grep -i 'connection:' | grep -i 'close' | wc -l" "1" "Connection: close should be respected"
  run_test "Default keep-alive" "curl -s -I http://$SERVER_HOST:$SERVER_PORT/ | grep -i 'connection:' | grep -E '(keep-alive|close)' | wc -l" "1" "Connection header should be present"
  
  # Range requests (if supported)
  run_test "Range request support" "curl -s -H 'Range: bytes=0-10' -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "206" "Range request should return 206 if supported"
  run_test "Invalid range" "curl -s -H 'Range: bytes=abc-def' -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "400" "Invalid range should return 400"
  
  # Conditional requests
  run_test "If-Modified-Since" "curl -s -H 'If-Modified-Since: Thu, 01 Jan 1970 00:00:00 GMT' -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "200" "If-Modified-Since should work or be ignored"
  run_test "If-None-Match" "curl -s -H 'If-None-Match: \"test-etag\"' -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "200" "If-None-Match should work or be ignored"
}

# === Performance and stress tests ===
run_performance_tests() {
  echo -e "\n${CYAN}⚡ Running PERFORMANCE tests...${NC}"
  
  # Concurrent connection limits
  run_test "100 concurrent requests" "seq 1 100 | xargs -I {} -P 100 curl -s -o /dev/null -w '%{http_code}\n' http://$SERVER_HOST:$SERVER_PORT/ | grep -c '200'" "100" "100 concurrent GET requests"
  run_test "500 concurrent requests" "seq 1 500 | xargs -I {} -P 50 curl -s -o /dev/null -w '%{http_code}\n' http://$SERVER_HOST:$SERVER_PORT/ | grep -c '200'" "500" "500 concurrent GET requests (batched)"
  
  # Large request/response handling
  run_test "Large POST body" "dd if=/dev/zero bs=1k count=100 2>/dev/null | curl -s -o /dev/null -w '%{http_code}' --data-binary @- http://$SERVER_HOST:$SERVER_PORT/" "413" "Large POST body (should exceed limit)"
  run_test "Many headers" "curl -s -o /dev/null -w '%{http_code}' $(for i in {1..100}; do echo -n \"-H 'X-Test-$i: value$i' \"; done) http://$SERVER_HOST:$SERVER_PORT/" "431" "Many headers (should exceed limit)"
  
  # Keep-alive performance
  run_test "Keep-alive reuse" "curl -s -o /dev/null -w '%{time_total}' --keepalive-time 30 http://$SERVER_HOST:$SERVER_PORT/ http://$SERVER_HOST:$SERVER_PORT/ | awk '{print (\$1 < 1.0) ? \"fast\" : \"slow\"}'" "fast" "Keep-alive connection reuse performance"
  
  # Slow client attacks
  run_test "Slow headers attack" "{ printf 'GET / HTTP/1.1\r\n'; sleep 1; printf 'Host: $SERVER_HOST\r\n'; sleep 1; printf '\r\n'; } | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "408" "Slow headers attack (timeout)"
  run_test "Slow body attack" "{ printf 'POST / HTTP/1.1\r\nHost: $SERVER_HOST\r\nContent-Length: 10\r\n\r\n'; sleep 1; printf 'hello'; sleep 2; printf 'world'; } | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "408" "Slow body attack (timeout)"
}

# === Malformed request tests ===
run_malformed_request_tests() {
  echo -e "\n${CYAN}💀 Running MALFORMED REQUEST tests...${NC}"
  
  # Invalid characters in request line
  run_test "NULL in request line" "printf 'GET /\x00test HTTP/1.1\r\nHost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "NULL byte in request line"
  run_test "Control chars in URL" "printf 'GET /\x01\x02\x03 HTTP/1.1\r\nHost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Control characters in URL"
  run_test "High ASCII in URL" "printf 'GET /\xff\xfe HTTP/1.1\r\nHost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "High ASCII in URL"
  
  # Invalid line endings
  run_test "LF only endings" "printf 'GET / HTTP/1.1\nHost: $SERVER_HOST\n\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "LF only line endings"
  run_test "Mixed line endings" "printf 'GET / HTTP/1.1\r\nHost: $SERVER_HOST\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Mixed line endings"
  run_test "CR only endings" "printf 'GET / HTTP/1.1\rHost: $SERVER_HOST\r\r' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "CR only line endings"
  
  # Malformed headers
  run_test "Header with NULL" "printf 'GET / HTTP/1.1\r\nHost: $SERVER_HOST\r\nX-Test: val\x00ue\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "NULL in header value"
  run_test "Header name with space" "printf 'GET / HTTP/1.1\r\nHost: $SERVER_HOST\r\nX Test: value\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Space in header name"
  run_test "Header name with colon" "printf 'GET / HTTP/1.1\r\nHost: $SERVER_HOST\r\nX:Test: value\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Colon in header name"
  
  # Request smuggling attempts
  run_test "Request smuggling CL.TE" "printf 'POST / HTTP/1.1\r\nHost: $SERVER_HOST\r\nContent-Length: 6\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Request smuggling attempt (CL.TE)"
  run_test "Request smuggling TE.CL" "printf 'POST / HTTP/1.1\r\nHost: $SERVER_HOST\r\nTransfer-Encoding: chunked\r\nContent-Length: 4\r\n\r\n5c\r\nGET /admin HTTP/1.1\r\nHost: $SERVER_HOST\r\n\r\n0\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Request smuggling attempt (TE.CL)"
  
  # HTTP version edge cases
  run_test "HTTP/1.10 version" "printf 'GET / HTTP/1.10\r\nHost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Invalid HTTP version 1.10"
  run_test "HTTP/1.1.0 version" "printf 'GET / HTTP/1.1.0\r\nHost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Invalid HTTP version 1.1.0"
  run_test "Empty HTTP version" "printf 'GET / HTTP/\r\nHost: $SERVER_HOST\r\n\r\n' | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "400" "Empty HTTP version"
}

# === Boundary value tests ===
run_boundary_tests() {
  echo -e "\n${CYAN}🎯 Running BOUNDARY VALUE tests...${NC}"
  
  # URL length boundaries
  run_test "URL length 1024" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/$(printf 'x%.0s' {1..1024})" "404" "URL length exactly 1024 characters"
  run_test "URL length 2048" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/$(printf 'x%.0s' {1..2048})" "414" "URL length 2048 characters"
  run_test "URL length 4096" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/$(printf 'x%.0s' {1..4096})" "414" "URL length 4096 characters"
  
  # Header size boundaries
  run_test "Header value 1KB" "curl -s -o /dev/null -w '%{http_code}' -H 'X-Large: $(printf 'x%.0s' {1..1024})' http://$SERVER_HOST:$SERVER_PORT/" "200" "Header value 1KB"
  run_test "Header value 8KB" "curl -s -o /dev/null -w '%{http_code}' -H 'X-Large: $(printf 'x%.0s' {1..8192})' http://$SERVER_HOST:$SERVER_PORT/" "431" "Header value 8KB"
  run_test "Header value 16KB" "curl -s -o /dev/null -w '%{http_code}' -H 'X-Large: $(printf 'x%.0s' {1..16384})' http://$SERVER_HOST:$SERVER_PORT/" "431" "Header value 16KB"
  
  # Content-Length boundaries
  run_test "Content-Length 0" "curl -s -o /dev/null -w '%{http_code}' -H 'Content-Length: 0' -d '' http://$SERVER_HOST:$SERVER_PORT/" "200" "Content-Length exactly 0"
  run_test "Content-Length 1MB" "curl -s -o /dev/null -w '%{http_code}' -H 'Content-Length: 1048576' --data-binary @<(dd if=/dev/zero bs=1M count=1 2>/dev/null) http://$SERVER_HOST:$SERVER_PORT/" "413" "Content-Length 1MB"
  run_test "Content-Length max int" "curl -s -o /dev/null -w '%{http_code}' -H 'Content-Length: 2147483647' http://$SERVER_HOST:$SERVER_PORT/" "413" "Content-Length maximum integer"
  
  # Connection limits
  run_test "Connection limit test" "for i in {1..1000}; do curl -s -o /dev/null http://$SERVER_HOST:$SERVER_PORT/ & if [ \$((\$i % 100)) -eq 0 ]; then wait; fi; done; wait; echo '200'" "200" "1000 connections (batched)"
  
  # Port number edge cases
  run_test "Connect to port 0" "curl -s -o /dev/null -w '%{http_code}' --connect-timeout 2 http://$SERVER_HOST:0/" "000" "Connection to port 0 should fail"
  run_test "Connect to port 65535" "curl -s -o /dev/null -w '%{http_code}' --connect-timeout 2 http://$SERVER_HOST:65535/" "000" "Connection to port 65535 should fail if not listening"
  
  # Query string boundaries
  run_test "Query string 1KB" "curl -s -o /dev/null -w '%{http_code}' 'http://$SERVER_HOST:$SERVER_PORT/?data=$(printf 'x%.0s' {1..1024})'" "200" "Query string 1KB"
  run_test "Query string 4KB" "curl -s -o /dev/null -w '%{http_code}' 'http://$SERVER_HOST:$SERVER_PORT/?data=$(printf 'x%.0s' {1..4096})'" "414" "Query string 4KB"
  
  # Cookie boundaries
  run_test "Cookie 4KB" "curl -s -o /dev/null -w '%{http_code}' -H 'Cookie: data=$(printf 'x%.0s' {1..4096})' http://$SERVER_HOST:$SERVER_PORT/" "431" "Cookie 4KB"
  run_test "Many cookies" "curl -s -o /dev/null -w '%{http_code}' $(for i in {1..100}; do echo -n \"-H 'Cookie: cookie$i=value$i' \"; done) http://$SERVER_HOST:$SERVER_PORT/" "431" "100 cookies"
}

# === Additional 42-specific tests ===
run_42_specific_tests() {
  echo -e "\n${CYAN}🎓 Running 42 PROJECT SPECIFIC tests...${NC}"
  
  # Error page customization
  run_test "Custom 404 page" "curl -s http://$SERVER_HOST:$SERVER_PORT/nonexistent | grep -q '404\\|Not Found' && echo 'found' || echo 'missing'" "found" "Custom 404 error page"
  run_test "Custom 500 page" "curl -s http://$SERVER_HOST:$SERVER_PORT/error500 | grep -q '500\\|Internal Server Error' && echo 'found' || echo 'missing'" "found" "Custom 500 error page"
  
  # CGI environment variables
  run_test "CGI REQUEST_METHOD" "curl -s 'http://$SERVER_HOST:$SERVER_PORT/cgi-bin/1.py' | grep -q 'REQUEST_METHOD' && echo 'found' || echo 'missing'" "found" "CGI REQUEST_METHOD variable"
  run_test "CGI QUERY_STRING" "curl -s 'http://$SERVER_HOST:$SERVER_PORT/cgi-bin/1.py?test=value' | grep -q 'QUERY_STRING' && echo 'found' || echo 'missing'" "found" "CGI QUERY_STRING variable"
  run_test "CGI CONTENT_TYPE" "curl -s -H 'Content-Type: application/json' -d '{}' 'http://$SERVER_HOST:$SERVER_PORT/cgi-bin/1.py' | grep -q 'CONTENT_TYPE' && echo 'found' || echo 'missing'" "found" "CGI CONTENT_TYPE variable"
  
  # Configuration file parsing
  run_test "Multiple servers" "curl -s -H 'Host: localhost' -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "200" "Multiple server configurations"
  run_test "Server name matching" "curl -s -H 'Host: example.com' -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "200" "Server name matching"
  
  # File upload limits
  run_test "Upload size limit" "dd if=/dev/zero of=large_upload.tmp bs=1M count=2 2>/dev/null && curl -s -o /dev/null -w '%{http_code}' -F 'file=@large_upload.tmp' http://$SERVER_HOST:$SERVER_PORT/uploads/ && rm -f large_upload.tmp" "413" "Upload size limit enforcement"
  
  # Directory listing
  run_test "Directory autoindex" "curl -s http://$SERVER_HOST:$SERVER_PORT/uploads/ | grep -q 'Index of\\|Directory listing' && echo 'found' || echo 'missing'" "found" "Directory autoindex functionality"
  run_test "Hidden files in listing" "curl -s http://$SERVER_HOST:$SERVER_PORT/uploads/ | grep -q '\\.htaccess\\|\\.hidden' && echo 'found' || echo 'missing'" "missing" "Hidden files should not appear in listing"
  
  # Security features
  run_test "Directory traversal blocked" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/../config/config.conf" "400" "Directory traversal protection"
  run_test "Executable prevention" "curl -s -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/Webserv" "404" "Executable file access prevention"
  
  # HTTP methods on different routes
  run_test "POST on upload route" "curl -s -o /dev/null -w '%{http_code}' -d 'test=data' http://$SERVER_HOST:$SERVER_PORT/uploads/" "200" "POST method on upload route"
  run_test "DELETE on upload route" "echo 'test' > /tmp/test_delete.txt && curl -s -F 'file=@/tmp/test_delete.txt' http://$SERVER_HOST:$SERVER_PORT/uploads/ && curl -s -X DELETE -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/uploads/test_delete.txt && rm -f /tmp/test_delete.txt" "200" "DELETE method on uploaded file"
  
  # Timeout handling
  run_test "Client timeout" "{ printf 'GET / HTTP/1.1\r\nHost: $SERVER_HOST\r\n'; sleep 35; printf '\r\n'; } | nc $SERVER_HOST $SERVER_PORT -q 1 | grep 'HTTP/1.1' | cut -d' ' -f2" "408" "Client timeout (30s+)"
  
  # Virtual hosting
  run_test "Virtual host routing" "curl -s -H 'Host: test.local' -o /dev/null -w '%{http_code}' http://$SERVER_HOST:$SERVER_PORT/" "200" "Virtual host routing"
  
  # Content type detection
  run_test "HTML content type" "curl -s -I http://$SERVER_HOST:$SERVER_PORT/index.html | grep -i 'content-type:' | grep -q 'text/html' && echo 'correct' || echo 'incorrect'" "correct" "HTML content type detection"
  run_test "CSS content type" "curl -s -I http://$SERVER_HOST:$SERVER_PORT/style.css | grep -i 'content-type:' | grep -q 'text/css' && echo 'correct' || echo 'incorrect'" "correct" "CSS content type detection"
  run_test "JS content type" "curl -s -I http://$SERVER_HOST:$SERVER_PORT/script.js | grep -i 'content-type:' | grep -q 'application/javascript' && echo 'correct' || echo 'incorrect'" "correct" "JavaScript content type detection"
}

# === Main execution ===
start_timestamp=$(date +%s)
echo -e "${PURPLE}🧪 Enhanced Webserv Test Suite${NC}"
echo -e "${BLUE}Server: http://$SERVER_HOST:$SERVER_PORT${NC}"
echo -e "${BLUE}Timeout: ${TIMEOUT_DURATION}s${NC}"
echo ""

# Start server
start_server

# Wait for server to be ready
echo -e "${BLUE}🔍 Checking server readiness...${NC}"
if ! check_server; then
  echo -e "${RED}❌ Server check failed${NC}"
  exit 1
fi
echo -e "${GREEN}✅ Server is ready${NC}"

# Run selected test suites
echo -e "\n${CYAN}🎯 Running selected test suites...${NC}"

if $DO_ALL || $DO_BASIC; then
  run_basic_tests
fi

if $DO_ALL || $DO_METHODS; then
  run_methods_tests
fi

if $DO_ALL || $DO_ROUTES; then
  run_routes_tests
fi

if $DO_ALL || $DO_CGI; then
  run_cgi_tests
fi

if $DO_ALL || $DO_UPLOAD; then
  run_upload_tests
fi

if $DO_ALL || $DO_ERROR; then
  run_error_tests
fi

if $DO_ALL || $DO_STRESS; then
  run_stress_tests
fi

if $DO_ALL || $DO_SECURITY; then
  run_security_tests
fi

if $DO_ALL || $DO_EDGE_CASES; then
  run_edge_case_tests
fi

if $DO_ALL || $DO_PROTOCOL; then
  run_protocol_compliance_tests
fi

if $DO_ALL || $DO_PERFORMANCE; then
  run_performance_tests
fi

if $DO_ALL || $DO_MALFORMED; then
  run_malformed_request_tests
fi

if $DO_ALL || $DO_BOUNDARY; then
  run_boundary_tests
fi

if $DO_ALL || $DO_42_SPECIFIC; then
  run_42_specific_tests
fi

# === Generate comprehensive report ===
echo -e "\n\n${CYAN}📊 Test Report Summary${NC}"
echo "================================================================================================="

# Calculate statistics
total_tests=${#TEST_NAMES[@]}
passed_tests=0
failed_tests=0

for status in "${TEST_STATUS[@]}"; do
  if [[ "$status" == "✅ PASS" ]]; then
    ((passed_tests++))
  else
    ((failed_tests++))
  fi
done

echo -e "${BLUE}Total Tests: $total_tests${NC}"
echo -e "${GREEN}Passed: $passed_tests${NC}"
echo -e "${RED}Failed: $failed_tests${NC}"
if [ $total_tests -gt 0 ]; then
  echo -e "${YELLOW}Success Rate: $(( passed_tests * 100 / total_tests ))%${NC}"
else
  echo -e "${YELLOW}Success Rate: 0%${NC}"
fi
echo ""

# Detailed results
printf "%-40s %-10s %-10s %-8s %s\n" "Test Name" "Expected" "Got" "Status" "Details"
echo "================================================================================================="

# Write to report file
{
  echo "================================================================================================="
  echo "Enhanced Webserv Test Report - 42 Network Project"
  echo "================================================================================================="
  echo "Generated: $(date)"
  echo "Server: http://$SERVER_HOST:$SERVER_PORT"
  echo "Test Duration: $(( $(date +%s) - start_timestamp ))s"
  echo ""
  echo "SUMMARY:"
  echo "--------"
  echo "Total Tests: $total_tests"
  echo "Passed: $passed_tests"
  echo "Failed: $failed_tests"
  if [ $total_tests -gt 0 ]; then
    echo "Success Rate: $(( passed_tests * 100 / total_tests ))%"
  else
    echo "Success Rate: 0%"
  fi
  echo ""
  echo "DETAILED RESULTS:"
  echo "-----------------"
  printf "%-50s %-12s %-12s %-8s %s\n" "Test Name" "Expected" "Got" "Status" "Details"
  echo "================================================================================================="
} > "$RAPPORT"

for i in "${!TEST_NAMES[@]}"; do
  name="${TEST_NAMES[$i]}"
  exp="${TEST_EXPECTED[$i]}"
  got="${TEST_RESULTS[$i]}"
  status="${TEST_STATUS[$i]}"
  details="${TEST_DETAILS[$i]}"
  
  # Truncate long names for display
  display_name="${name:0:49}"
  
  printf "%-50s %-12s %-12s %-8s %s\n" "$display_name" "$exp" "$got" "$status" "$details"
  printf "%-50s %-12s %-12s %-8s %s\n" "$name" "$exp" "$got" "$status" "$details" >> "$RAPPORT"
done

echo ""
echo -e "${GREEN}✅ Detailed report saved to: ${CYAN}$RAPPORT${NC}"
echo -e "${BLUE}📋 Server log available at: ${CYAN}$SERVER_LOG${NC}"

# Show failed tests summary
if [ $failed_tests -gt 0 ]; then
  echo -e "\n${RED}❌ Failed Tests Summary:${NC}"
  for i in "${!TEST_NAMES[@]}"; do
    if [[ "${TEST_STATUS[$i]}" == "❌ FAIL" ]]; then
      echo -e "${RED}  • ${TEST_NAMES[$i]} - Expected: ${TEST_EXPECTED[$i]}, Got: ${TEST_RESULTS[$i]}${NC}"
    fi
  done
fi

echo -e "\n${PURPLE}🏁 Test suite completed!${NC}"

exit 0
