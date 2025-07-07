#!/usr/bin/python3

# IMPORTANT:
# 1. The first line must be the interpreter path. Adjust if yours is different.
# 2. This script must have execute permissions: chmod +x basic_test.py

# --- CGI Headers ---
# This tells the web server (and browser) what kind of content to expect.
print("Content-Type: text/html")

# --- Blank Line ---
# THIS IS CRITICAL! It separates the HTTP headers from the body.
# Your C++ server *must* find this `\r\n\r\n` (double newline) to correctly
# parse the CGI's output.
print("")

# --- HTML Body ---
# The actual content of the web page.
print("<!DOCTYPE html>")
print("<html lang='en'>")
print("<head>")
print("    <meta charset='UTF-8'>")
print("    <title>CGI Basic Test</title>")
print("</head>")
print("<body>")
print("    <h1>Hello from Basic Python CGI!</h1>")
print("    <p>If you see this, your C++ server successfully executed the CGI script.</p>")
print("</body>")
print("</html>")