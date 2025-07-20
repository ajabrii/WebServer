#!/bin/bash

printf "Content-Type: text/html\r\n\r\n"

POST_DATA=$(cat)

cat <<EOF
<html>
<head><title>POST Data</title></head>
<body>
  <h1>Received POST Data</h1>
  <pre>$POST_DATA</pre>
</body>
</html>
EOF
