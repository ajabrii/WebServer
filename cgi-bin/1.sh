#!/bin/bash bash

curl "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent" \
  -H 'Content-Type: application/json' \
  -H 'X-goog-api-key: AIzaSyBdYdPO-RcizI7JGMNFaUwzTxv4ZRgKD3c' \
  -X POST \
  -d '{
    "contents": [
      {
        "parts": [
          {
            "text": "give me a script that build html css of a landing page and print it  in python you response should contain only python code no explination"
          }
        ]
      }
    ]
  }' | jq -r '.candidates[0].content.parts[0].text'
