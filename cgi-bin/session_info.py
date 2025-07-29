#!/usr/bin/env python3
import os
import json
import datetime

def main():
    print("Content-Type: application/json\r\n\r\n")
    
    session_dir = "../session"
    response = {
        "status": "success",
        "message": "Session directory information",
        "session_directory": session_dir,
        "directory_exists": os.path.exists(session_dir),
        "active_sessions": [],
        "timestamp": datetime.datetime.now().isoformat()
    }
    
    if os.path.exists(session_dir):
        try:
            session_files = [f for f in os.listdir(session_dir) if not f.startswith('.')]
            response["session_count"] = len(session_files)
            
            for session_file in session_files[:10]:  # Show first 10 sessions
                session_path = os.path.join(session_dir, session_file)
                if os.path.isfile(session_path):
                    try:
                        with open(session_path, 'r') as f:
                            session_data = json.load(f)
                        
                        response["active_sessions"].append({
                            "session_id": session_file,
                            "created": session_data.get("created", "Unknown"),
                            "last_accessed": session_data.get("last_accessed", "Unknown"),
                            "username": session_data.get("data", {}).get("username", "Unknown")
                        })
                    except:
                        response["active_sessions"].append({
                            "session_id": session_file,
                            "status": "corrupted or unreadable"
                        })
        except Exception as e:
            response["error"] = f"Error reading session directory: {str(e)}"
    else:
        response["session_count"] = 0
    
    print(json.dumps(response, indent=2))

if __name__ == "__main__":
    main()
