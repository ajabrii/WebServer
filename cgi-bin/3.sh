#!/bin/bash

printf "Content-Type: text/html\r\n\r\n"

# Read POST data from stdin
read POST_DATA

echo "<html><body>"
echo "<h1>CGI POST Test</h1>"
echo "<p>Received POST data:</p>"
echo "<pre>$POST_DATA</pre>"
echo "</body></html>"