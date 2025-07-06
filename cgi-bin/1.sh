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
            "text": "You are a world-class front-end developer. Write a single, complete, production-ready HTML file for a modern Chat with AI web interface like chatgpt add the side bar etc dark mode name the send mode hak t9awed. The page must use HTML, CSS, and vanilla JavaScript only (no frameworks). The chat UI must be clean and responsive, with an input box, send button, and a display area for the conversation. Include minimal styling inside a <style> tag and place all JavaScript inside a <script> tag at the bottom of the HTML. Use placeholder functions for sending messages to the AI (no actual backend). Output strictly only the raw HTML, CSS, and JavaScript. Do not include any explanations, markdown, or backticks — not even ```html. The response must start directly with <!DOCTYPE html> and contain nothing else."
          }
        ]
      }
    ]
  }' | jq -r '.candidates[0].content.parts[0].text' | sed '/^```/d'
