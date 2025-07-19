#!/bin/bash

printf "Content-Type: text/html\r\n\r\n"

# Read POST data
read POST_DATA

# Optional: escape double quotes if needed for JSON
ESCAPED_POST_DATA=$(printf '%s' "$POST_DATA" | sed 's/"/\\"/g')

# Make the curl call
curl -s "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent" \
  -H 'Content-Type: application/json' \
  -H 'X-goog-api-key:AIzaSyBdYdPO-RcizI7JGMNFaUwzTxv4ZRgKD3c' \
  -X POST \
  -d '{
    "contents": [
      {
        "parts": [
          {
            "text": "'"$ESCAPED_POST_DATA"'"
          }
        ]
      }
    ]
  }' | jq -r '.candidates[0].content.parts[0].text' | sed '/^```/d'
