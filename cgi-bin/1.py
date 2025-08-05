#!/usr/bin/env python3
# Output required CGI header and blank line (MUST be CRLF CRLF)
print("HTTP/1.0 200 OK")
print("Content-Type: text/html")

print("\r\n\r\n")
import time

# Output HTML content
print("<html>")
print("<head><title>CGI Test</title></head>")
print("<body>")
print("<h1>CGI Test Page</h1>")
print("<p>This is a test page for CGI scripts.</p>")
print("<p>Current time: " + time.strftime("%Y-%m-%d %H:%M:%S") + "</p>")
print("</body>")
print("</html>")
