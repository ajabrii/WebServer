#!/usr/bin/env python3
import os
import sys
import datetime
import urllib.parse
import hashlib
import json
import random
import string

# Session storage directory (relative to cgi-bin)
SESSION_DIR = "../session"

def generate_session_id():
    """Generate a unique session ID"""
    random_string = ''.join(random.choices(string.ascii_uppercase + string.digits, k=16))
    timestamp = str(datetime.datetime.now().timestamp())
    return hashlib.md5((random_string + timestamp).encode()).hexdigest().upper()

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

def set_cookie(name, value, expires=None, path='/', secure=False, httponly=True):
    """Generate Set-Cookie header"""
    cookie = f"{name}={urllib.parse.quote(value)}"
    
    if expires:
        cookie += f"; Expires={expires.strftime('%a, %d %b %Y %H:%M:%S GMT')}"
    
    if path:
        cookie += f"; Path={path}"
    
    if secure:
        cookie += "; Secure"
    
    if httponly:
        cookie += "; HttpOnly"
    
    return f"Set-Cookie: {cookie}\r\n"

def save_session(session_id, data):
    """Save session data to file"""
    try:
        session_file = os.path.join(SESSION_DIR, session_id)
        session_data = {
            'data': data,
            'created': datetime.datetime.now().isoformat(),
            'last_accessed': datetime.datetime.now().isoformat()
        }
        
        # Ensure session directory exists
        os.makedirs(SESSION_DIR, exist_ok=True)
        
        with open(session_file, 'w') as f:
            json.dump(session_data, f, indent=2)
        
        return True
    except Exception as e:
        return False

def load_session(session_id):
    """Load session data from file"""
    try:
        session_file = os.path.join(SESSION_DIR, session_id)
        if os.path.exists(session_file):
            with open(session_file, 'r') as f:
                session_data = json.load(f)
            
            # Update last accessed time
            session_data['last_accessed'] = datetime.datetime.now().isoformat()
            with open(session_file, 'w') as f:
                json.dump(session_data, f, indent=2)
            
            return session_data['data']
        return None
    except Exception as e:
        return None

def delete_session(session_id):
    """Delete session file"""
    try:
        session_file = os.path.join(SESSION_DIR, session_id)
        if os.path.exists(session_file):
            os.remove(session_file)
            return True
        return False
    except Exception as e:
        return False

def main():
    method = os.environ.get('REQUEST_METHOD', 'GET')
    cookies = get_cookies()
    current_session_id = cookies.get('WEBSERV_SESSION_ID')
    
    # Set content type
    print("Content-Type: application/json\r\n", end='')
    
    if method == 'POST':
        # Parse POST data
        post_data = parse_post_data()
        action = post_data.get('action', [''])[0]
        username = post_data.get('username', [''])[0]
        
        if action == 'create_session':
            # Create new session
            session_id = generate_session_id()
            session_data = {
                'username': username or 'anonymous',
                'login_time': datetime.datetime.now().isoformat(),
                'page_views': 1,
                'user_agent': os.environ.get('HTTP_USER_AGENT', 'Unknown'),
                'ip_address': os.environ.get('REMOTE_ADDR', 'Unknown')
            }
            
            if save_session(session_id, session_data):
                # Set session cookie (expires in 1 hour)
                expires = datetime.datetime.now() + datetime.timedelta(hours=1)
                print(set_cookie('WEBSERV_SESSION_ID', session_id, expires), end='')
                
                # End headers
                print("\r\n", end='')
                
                response = {
                    "status": "success",
                    "message": "Session created successfully",
                    "session_id": session_id,
                    "session_data": session_data,
                    "timestamp": datetime.datetime.now().isoformat()
                }
            else:
                # End headers
                print("\r\n", end='')
                response = {
                    "status": "error",
                    "message": "Failed to create session",
                    "timestamp": datetime.datetime.now().isoformat()
                }
        
        elif action == 'destroy_session':
            if current_session_id and delete_session(current_session_id):
                # Clear session cookie
                past_date = datetime.datetime.now() - datetime.timedelta(days=1)
                print(set_cookie('WEBSERV_SESSION_ID', '', past_date), end='')
                
                # End headers
                print("\r\n", end='')
                
                response = {
                    "status": "success",
                    "message": "Session destroyed",
                    "session_id": current_session_id,
                    "timestamp": datetime.datetime.now().isoformat()
                }
            else:
                # End headers
                print("\r\n", end='')
                response = {
                    "status": "error",
                    "message": "No active session to destroy",
                    "timestamp": datetime.datetime.now().isoformat()
                }
        
        else:
            # End headers
            print("\r\n", end='')
            response = {
                "status": "error",
                "message": "Unknown action",
                "available_actions": ["create_session", "destroy_session"],
                "timestamp": datetime.datetime.now().isoformat()
            }
    
    elif method == 'GET':
        # End headers
        print("\r\n", end='')
        
        if current_session_id:
            # Load existing session
            session_data = load_session(current_session_id)
            if session_data:
                # Update page views
                session_data['page_views'] = session_data.get('page_views', 0) + 1
                session_data['last_page_view'] = datetime.datetime.now().isoformat()
                save_session(current_session_id, session_data)
                
                response = {
                    "status": "success",
                    "message": "Session found and updated",
                    "session_id": current_session_id,
                    "session_data": session_data,
                    "timestamp": datetime.datetime.now().isoformat()
                }
            else:
                response = {
                    "status": "error",
                    "message": "Session ID found but session data is invalid or expired",
                    "session_id": current_session_id,
                    "timestamp": datetime.datetime.now().isoformat()
                }
        else:
            response = {
                "status": "info",
                "message": "No active session found",
                "suggestion": "Use POST with action=create_session to create a new session",
                "available_cookies": list(cookies.keys()),
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
    print(json.dumps(response, indent=2))

if __name__ == "__main__":
    main()