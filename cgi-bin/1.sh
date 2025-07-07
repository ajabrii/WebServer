#!/bin/bash

curl "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent" \
  -H 'Content-Type: application/json' \
  -H 'X-goog-api-key: AIzaSyBdYdPO-RcizI7JGMNFaUwzTxv4ZRgKD3c' \
  -X POST \
  -d '{
    "contents": [
      {
        "parts": [
          {
            "text": "You are a world-class front-end developer and UX/UI designer. Write a single, complete, production-ready HTML file for a modern, responsive AI chat web app inspired by ChatGPT. Include: a collapsible sidebar with placeholder items; a clean, minimalist dark mode UI; an input box, send button labeled '\''hak t9awed'\''; and a display area for conversation messages styled like chat bubbles; and a prominent play button that launches an embedded simple Pong game in the center area. The Pong game must include a visible score display, buttons to increase or decrease the ball speed, and allow the player to control the paddle using the left and right arrow keys. Use only semantic HTML5, CSS, and vanilla JavaScript (no frameworks). Ensure the layout adapts to mobile and desktop, use Flexbox or CSS Grid for responsiveness, and add subtle animations or hover effects for better UX. All CSS must go inside a <style> tag in the <head>; all JavaScript must go inside a single <script> tag at the end of the body. Add placeholder JS functions to simulate sending and receiving messages from AI (no real backend). Output strictly only the raw HTML, CSS, and JS. Do not include any explanations, markdown, or backticks. The response must start directly with <!DOCTYPE html> and contain nothing else."
          }
        ]
      }
    ]
  }' | jq -r '.candidates[0].content.parts[0].text' | sed '/^```/d'
