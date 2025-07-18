#!/bin/bash

# Output headers with proper CRLF
printf "Status: 200 OK\r\n\r\n"
printf "Content-Type: text/plain\r\n"
printf "Content-Length: 25\r\n"
printf "\r\n"

# Output body
printf "Hello from CGI script!\n"
