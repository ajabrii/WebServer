#!/bin/bash
# This is 1.sh

echo "Content-Type: text/html"
echo "" # Crucial: empty line separates headers from body
echo "<!DOCTYPE html>"
echo "<html lang=\"en\">"
echo "<head><title>CGI Test</title></head>"
echo "<body>"
echo "<h1>Hello from the clean CGI script!</h1>"
echo "<p>Current time: $(date)</p>"
echo "</body>"
echo "</html>"

exit 0 # Explicitly exit to close stdout pipe