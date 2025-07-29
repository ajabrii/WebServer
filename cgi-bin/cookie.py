#!/usr/bin/env python3
import os
import sys
import datetime
import urllib.parse

def parse_post_data():
    """Parse POST data from stdin"""
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        return urllib.parse.parse_qs(post_data)
    return {}

def get_cookies():
    """Parse cookies from HTTP_COOKIE environment variable"""
    cookies = {}
    cookie_header = os.environ.get('HTTP_COOKIE', '')
    if cookie_header:
        for cookie in cookie_header.split(';'):
            if '=' in cookie:
                name, value = cookie.strip().split('=', 1)
                cookies[name] = urllib.parse.unquote(value)
    return cookies

def set_cookie(name, value, expires=None, path='/', domain=None, secure=False, httponly=False):
    """Generate Set-Cookie header"""
    cookie = f"{name}={urllib.parse.quote(value)}"
    
    if expires:
        cookie += f"; Expires={expires.strftime('%a, %d %b %Y %H:%M:%S GMT')}"
    
    if path:
        cookie += f"; Path={path}"
    
    if domain:
        cookie += f"; Domain={domain}"
    
    if secure:
        cookie += "; Secure"
    
    if httponly:
        cookie += "; HttpOnly"
    
    return f"Set-Cookie: {cookie}\r\n"

def main():
    method = os.environ.get('REQUEST_METHOD', 'GET')
    existing_cookies = get_cookies()
    
    # Set content type
    print("Content-Type: application/json\r\n", end='')
    
    if method == 'GET':
        # Set test cookies
        expires = datetime.datetime.now() + datetime.timedelta(hours=1)
        print(set_cookie('test_cookie', 'hello_world', expires), end='')
        print(set_cookie('session_test', 'active', expires), end='')
        print(set_cookie('user_pref', 'dark_mode', expires), end='')
        print(set_cookie('temp_cookie', 'temporary', datetime.datetime.now() + datetime.timedelta(minutes=5)), end='')
        
        # End headers
        print("\r\n", end='')
        
        # JSON response
        response = {
            "status": "success",
            "message": "Cookies have been set",
            "cookies_set": [
                {"name": "test_cookie", "value": "hello_world", "expires": "1 hour"},
                {"name": "session_test", "value": "active", "expires": "1 hour"},
                {"name": "user_pref", "value": "dark_mode", "expires": "1 hour"},
                {"name": "temp_cookie", "value": "temporary", "expires": "5 minutes"}
            ],
            "existing_cookies": existing_cookies,
            "timestamp": datetime.datetime.now().isoformat()
        }
        
    elif method == 'POST':
        # Parse POST data
        post_data = parse_post_data()
        action = post_data.get('action', [''])[0]
        
        # End headers
        print("\r\n", end='')
        
        if action == 'read':
            response = {
                "status": "success",
                "message": "Cookie reading test completed",
                "cookies_received": existing_cookies,
                "cookie_count": len(existing_cookies),
                "timestamp": datetime.datetime.now().isoformat()
            }
        
        elif action == 'clear':
            # Set cookies with past expiration to clear them
            past_date = datetime.datetime.now() - datetime.timedelta(days=1)
            for cookie_name in existing_cookies.keys():
                print(set_cookie(cookie_name, '', past_date), end='')
            
            response = {
                "status": "success",
                "message": "Cookies cleared",
                "cleared_cookies": list(existing_cookies.keys()),
                "timestamp": datetime.datetime.now().isoformat()
            }
        
        else:
            response = {
                "status": "error",
                "message": "Unknown action",
                "available_actions": ["read", "clear"],
                "timestamp": datetime.datetime.now().isoformat()
            }
    
    else:
        # End headers
        print("\r\n", end='')
        response = {
            "status": "error",
            "message": "Method not allowed",
            "allowed_methods": ["GET", "POST"],
            "timestamp": datetime.datetime.now().isoformat()
        }
    
    # Output JSON response
    import json
    print(json.dumps(response, indent=2))

if __name__ == "__main__":
    main()