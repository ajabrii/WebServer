#!/usr/bin/env python3
# Output required CGI header and blank line (MUST be CRLF CRLF)
print("Content-Type: text/html")
print("\r\n\r\n")

// read POST DATA

import sys
import cgi
# Read the POST data if any
form = cgi.FieldStorage()
# Check if there is any POST data
if form:
    # Process the POST data
    post_data = {key: form.getvalue(key) for key in form.keys()}
    # You can use post_data as needed, e.g., print it for debugging
    print("<p>Received POST data:</p>")
    print("<pre>")
    for key, value in post_data.items():
        print(f"{key}: {value}")
    print("</pre>")
else:
    print("<p>No POST data received.</p>")
