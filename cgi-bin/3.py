import os
import http.cookies  # For Python 3
print("Content-Type: text/html\r\n\r\n")
print("Cookies:", os.environ.get("HTTP_COOKIE"))
# from Cookie import SimpleCookie  # For Python 2

cookie = http.cookies.SimpleCookie()
cookie["session_id"] = "123ABC456DEF"
cookie["session_id"]["path"] = "/"
# Optional attributes:
cookie["session_id"]["httponly"] = True
cookie["session_id"]["max-age"] = 3600  # 1 hour

print("Content-Type: text/html")
print(cookie.output())  # Automatically prints multiple Set-Cookie headers if needed
print()  # Empty line separating headers from body
print("<html><body>Cookie set!</body></html>")
