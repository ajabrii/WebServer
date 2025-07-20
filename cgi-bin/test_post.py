#!/usr/bin/env python3

import os
import sys
import urllib.parse
import json

def main():
    # Get the request method from environment variable
    request_method = os.environ.get('REQUEST_METHOD', 'GET')
    content_type = os.environ.get('CONTENT_TYPE', '')
    content_length = os.environ.get('CONTENT_LENGTH', '0')
    query_string = os.environ.get('QUERY_STRING', '')
    
    # Output required CGI header
    print("Content-Type: text/html\r\n\r\n")
    
    # Start HTML
    print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CGI POST Test</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            margin: 0;
            padding: 20px;
            color: #333;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background: white;
            padding: 30px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }
        h1 {
            color: #4a5568;
            text-align: center;
            margin-bottom: 30px;
        }
        .method-badge {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            color: white;
            font-weight: bold;
            margin-bottom: 20px;
        }
        .get { background-color: #48bb78; }
        .post { background-color: #ed8936; }
        .env-section, .data-section, .form-section {
            background: #f7fafc;
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 20px;
            border-left: 4px solid #4299e1;
        }
        .env-var, .data-item {
            margin: 10px 0;
            padding: 8px;
            background: white;
            border-radius: 5px;
            border: 1px solid #e2e8f0;
        }
        .key {
            font-weight: bold;
            color: #2d3748;
        }
        .value {
            color: #4a5568;
            margin-left: 10px;
        }
        form {
            margin-top: 20px;
        }
        input[type="text"], input[type="email"], textarea {
            width: 100%;
            padding: 10px;
            margin: 5px 0 15px 0;
            border: 2px solid #e2e8f0;
            border-radius: 5px;
            box-sizing: border-box;
        }
        input[type="submit"] {
            background-color: #4299e1;
            color: white;
            padding: 12px 30px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        }
        input[type="submit"]:hover {
            background-color: #3182ce;
        }
        .json-output {
            background: #1a202c;
            color: #68d391;
            padding: 15px;
            border-radius: 5px;
            font-family: 'Courier New', monospace;
            overflow-x: auto;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 CGI POST Method Test</h1>
        
        <div class="method-badge {}">{}</div>
        
        <div class="env-section">
            <h2>📋 Environment Variables</h2>""".format(
        request_method.lower(), 
        request_method
    ))
    
    # Display important environment variables
    env_vars = [
        'REQUEST_METHOD',
        'CONTENT_TYPE', 
        'CONTENT_LENGTH',
        'QUERY_STRING',
        'PATH_INFO',
        'SCRIPT_NAME',
        'SCRIPT_FILENAME',
        'SERVER_NAME',
        'SERVER_PORT',
        'GATEWAY_INTERFACE',
        'SERVER_PROTOCOL',
        'REDIRECT_STATUS'
    ]
    
    for var in env_vars:
        value = os.environ.get(var, 'Not Set')
        print(f'<div class="env-var"><span class="key">{var}:</span><span class="value">{value}</span></div>')
    
    # Handle POST data
    post_data = ""
    parsed_data = {}
    
    if request_method == 'POST':
        try:
            content_len = int(content_length)
            if content_len > 0:
                post_data = sys.stdin.read(content_len)
                
                print('</div><div class="data-section">')
                print('<h2>📤 POST Data Received</h2>')
                print(f'<div class="data-item"><span class="key">Raw Data:</span><span class="value">{post_data}</span></div>')
                
                # Try to parse as URL-encoded form data
                if 'application/x-www-form-urlencoded' in content_type:
                    parsed_data = urllib.parse.parse_qs(post_data)
                    print('<h3>Parsed Form Data:</h3>')
                    for key, value_list in parsed_data.items():
                        for value in value_list:
                            print(f'<div class="data-item"><span class="key">{key}:</span><span class="value">{value}</span></div>')
                
                # Try to parse as JSON
                elif 'application/json' in content_type:
                    try:
                        parsed_data = json.loads(post_data)
                        print('<h3>Parsed JSON Data:</h3>')
                        print('<div class="json-output">')
                        print(json.dumps(parsed_data, indent=2))
                        print('</div>')
                    except json.JSONDecodeError:
                        print('<div class="data-item"><span class="key">JSON Parse Error:</span><span class="value">Invalid JSON format</span></div>')
                        
        except Exception as e:
            print(f'<div class="data-item"><span class="key">Error reading POST data:</span><span class="value">{str(e)}</span></div>')
    else:
        print('</div><div class="data-section">')
        print('<h2>ℹ️ No POST Data</h2>')
        print('<p>This was a GET request. Use the form below to test POST functionality.</p>')
    
    # Handle query string parameters
    if query_string:
        print('<h3>🔗 Query String Parameters:</h3>')
        query_params = urllib.parse.parse_qs(query_string)
        for key, value_list in query_params.items():
            for value in value_list:
                print(f'<div class="data-item"><span class="key">{key}:</span><span class="value">{value}</span></div>')
    
    print('</div>')
    
    # Test form
    print("""
        <div class="form-section">
            <h2>🧪 Test POST Request</h2>
            <p>Use this form to test POST functionality:</p>
            
            <form method="POST" action="">
                <label for="name">Name:</label>
                <input type="text" id="name" name="name" placeholder="Enter your name">
                
                <label for="email">Email:</label>
                <input type="email" id="email" name="email" placeholder="Enter your email">
                
                <label for="message">Message:</label>
                <textarea id="message" name="message" rows="4" placeholder="Enter your message"></textarea>
                
                <input type="submit" value="Submit POST Request">
            </form>
        </div>
        
        <div class="form-section">
            <h2>📊 JSON Test</h2>
            <p>Test with JSON data using curl:</p>
            <div class="json-output">
curl -X POST http://localhost:8080/cgi-bin/test_post.py \\
  -H "Content-Type: application/json" \\
  -d '{"name": "John Doe", "email": "john@example.com", "message": "Hello from JSON!"}'
            </div>
        </div>
        
        <div class="form-section">
            <h2>🔧 URL-encoded Test</h2>
            <p>Test with form data using curl:</p>
            <div class="json-output">
curl -X POST http://localhost:8080/cgi-bin/test_post.py \\
  -H "Content-Type: application/x-www-form-urlencoded" \\
  -d "name=Jane+Doe&email=jane@example.com&message=Hello+from+form+data"
            </div>
        </div>
    </div>
</body>
</html>""")

if __name__ == "__main__":
    main()
