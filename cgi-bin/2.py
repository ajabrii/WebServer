#!/usr/bin/python3

import sys

# Always flush header separately
sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("\r\n")
sys.stdout.flush()  # FLUSH NOW to ensure headers go before body

# Body starts here
print("<!DOCTYPE html>")
print("<html lang='en'>")
print("<head><meta charset='UTF-8'><title>CGI OK</title></head>")
print("<body><h1>Hello from Python CGI!</h1></body>")
print("</html>")
