#!/usr/bin/python3

import os
import sys
import urllib.parse

print("Content-Type: text/html\r\n\r\n")

def test_environment_variables():
    """Test basic CGI environment variables"""
    print("<h2>Environment Variables Test</h2>")
    
    required_vars = [
        'REQUEST_METHOD', 'QUERY_STRING', 'CONTENT_TYPE', 
        'CONTENT_LENGTH', 'SERVER_NAME', 'SERVER_PORT',
        'SCRIPT_NAME', 'PATH_INFO', 'HTTP_HOST'
    ]
    
    print("<table border='1'>")
    print("<tr><th>Variable</th><th>Value</th><th>Status</th></tr>")
    
    for var in required_vars:
        value = os.environ.get(var, '')
        status = "✓ SET" if value else "✗ MISSING"
        print(f"<tr><td>{var}</td><td>{value}</td><td>{status}</td></tr>")
    
    print("</table>")

def test_query_string():
    """Test query string parsing"""
    print("<h2>Query String Test</h2>")
    
    query_string = os.environ.get('QUERY_STRING', '')
    if query_string:
        print(f"<p><strong>Raw Query String:</strong> {query_string}</p>")
        
        # Parse query parameters
        params = urllib.parse.parse_qs(query_string)
        print("<h3>Parsed Parameters:</h3>")
        print("<ul>")
        for key, values in params.items():
            for value in values:
                print(f"<li><strong>{key}:</strong> {value}</li>")
        print("</ul>")
    else:
        print("<p>No query string provided. Try: <code>?name=test&value=123</code></p>")

def test_post_data():
    """Test POST data handling"""
    print("<h2>POST Data Test</h2>")
    
    method = os.environ.get('REQUEST_METHOD', '')
    content_length = os.environ.get('CONTENT_LENGTH', '')
    
    if method == 'POST':
        if content_length and content_length.isdigit():
            length = int(content_length)
            if length > 0:
                try:
                    post_data = sys.stdin.read(length)
                    print(f"<p><strong>POST Data ({length} bytes):</strong></p>")
                    print(f"<pre>{post_data}</pre>")
                    
                    # Try to parse as form data
                    try:
                        parsed = urllib.parse.parse_qs(post_data)
                        print("<h3>Parsed POST Parameters:</h3>")
                        print("<ul>")
                        for key, values in parsed.items():
                            for value in values:
                                print(f"<li><strong>{key}:</strong> {value}</li>")
                        print("</ul>")
                    except:
                        print("<p>Could not parse as form data (might be raw data)</p>")
                        
                except Exception as e:
                    print(f"<p>Error reading POST data: {e}</p>")
            else:
                print("<p>Content-Length is 0</p>")
        else:
            print("<p>No valid Content-Length header</p>")
    else:
        print(f"<p>Not a POST request (method: {method})</p>")
        print("<p>To test POST: <code>curl -X POST -d 'key=value' http://your-server/cgi-bin/test_cgi.py</code></p>")

def test_http_headers():
    """Test HTTP headers passed to CGI"""
    print("<h2>HTTP Headers Test</h2>")
    
    print("<table border='1'>")
    print("<tr><th>Header</th><th>Value</th></tr>")
    
    # Common HTTP headers that should be available as HTTP_* environment variables
    for key, value in sorted(os.environ.items()):
        if key.startswith('HTTP_'):
            header_name = key[5:].replace('_', '-').title()
            print(f"<tr><td>{header_name}</td><td>{value}</td></tr>")
    
    print("</table>")

def test_error_handling():
    """Test error scenarios"""
    print("<h2>Error Handling Test</h2>")
    
    # Test if we can write to stderr
    try:
        sys.stderr.write("This is a test error message\n")
        sys.stderr.flush()
        print("<p>✓ Successfully wrote to stderr</p>")
    except Exception as e:
        print(f"<p>✗ Error writing to stderr: {e}</p>")
    
    # Test exit codes (commented out as it would terminate the script)
    # print("<p>Note: Exit code testing would terminate the script</p>")

def main():
    print("<html>")
    print("<head><title>CGI Implementation Test</title></head>")
    print("<body>")
    print("<h1>WebServ CGI Implementation Test</h1>")
    print(f"<p><em>Test executed at: {os.getcwd()}</em></p>")
    
    # Run all tests
    test_environment_variables()
    print("<hr>")
    
    test_query_string()
    print("<hr>")
    
    test_post_data()
    print("<hr>")
    
    test_http_headers()
    print("<hr>")
    
    test_error_handling()
    
    print("<hr>")
    print("<h2>Test Summary</h2>")
    print("<p>If you can see this page with all sections populated correctly, ")
    print("your CGI implementation is working!</p>")
    
    print("<h3>Test Commands:</h3>")
    print("<ul>")
    print("<li>GET with query: <code>curl 'http://localhost:8080/cgi-bin/test_cgi.py?name=test&id=123'</code></li>")
    print("<li>POST with data: <code>curl -X POST -d 'username=john&password=secret' http://localhost:8080/cgi-bin/test_cgi.py</code></li>")
    print("<li>With headers: <code>curl -H 'X-Custom-Header: test' http://localhost:8080/cgi-bin/test_cgi.py</code></li>")
    print("</ul>")
    
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()
