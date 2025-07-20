#!/bin/bash

printf "Content-Type: text/html\r\n\r\n"

time_num=10123821873912

sleep $time_num
printf "<h1>Child Done sleeping for $time_num seconds </h1>\n"
printf "<h1>Hello, World hamza! sleeped for $time_num </h1>\n"

