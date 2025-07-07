<<<<<<< HEAD
#!/bin/bash
=======
#!/bin/bash bash


>>>>>>> youness

curl "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent" \
  -H 'Content-Type: application/json' \
  -H 'X-goog-api-key: AIzaSyBdYdPO-RcizI7JGMNFaUwzTxv4ZRgKD3c' \
  -X POST \
  -d '{
    "contents": [
      {
        "parts": [
          {
<<<<<<< HEAD
            "text": "You are a world-class front-end developer and UX/UI designer. Write a single, complete, production-ready HTML file for a modern, responsive AI chat web app inspired by ChatGPT. Include: a collapsible sidebar with placeholder items; a clean, minimalist dark mode UI; an input box, send button labeled '\''hak t9awed'\''; and a display area for conversation messages styled like chat bubbles; and a prominent play button that launches an embedded simple Pong game in the center area. The Pong game must include a visible score display, buttons to increase or decrease the ball speed, and allow the player to control the paddle using the left and right arrow keys. Use only semantic HTML5, CSS, and vanilla JavaScript (no frameworks). Ensure the layout adapts to mobile and desktop, use Flexbox or CSS Grid for responsiveness, and add subtle animations or hover effects for better UX. All CSS must go inside a <style> tag in the <head>; all JavaScript must go inside a single <script> tag at the end of the body. Add placeholder JS functions to simulate sending and receiving messages from AI (no real backend). Output strictly only the raw HTML, CSS, and JS. Do not include any explanations, markdown, or backticks. The response must start directly with <!DOCTYPE html> and contain nothing else."
=======
            "text": "You are a world-class front-end developer. Write a single, complete, production-ready HTML file for a modern Chat with AI web interface like chatgpt add the side bar etc dark mode name the send mode hak t9awed. The page must use HTML, CSS, and vanilla JavaScript only (no frameworks). The chat UI must be clean and responsive, with an input box, send button, and a display area for the conversation. Include minimal styling inside a <style> tag and place all JavaScript inside a <script> tag at the bottom of the HTML. Use placeholder functions for sending messages to the AI (no actual backend). Output strictly only the raw HTML, CSS, and JavaScript. Do not include any explanations, markdown, or backticks — not even ```html. The response must start directly with <!DOCTYPE html> and contain nothing else."
>>>>>>> youness
          }
        ]
      }
    ]
  }' | jq -r '.candidates[0].content.parts[0].text' | sed '/^```/d'
