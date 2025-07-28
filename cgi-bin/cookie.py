import os
print("Content-Type: text/plain\r\n\r\n")
print("Cookies:", os.environ.get("HTTP_COOKIE"))
print("Environment Variables:")
for key, value in os.environ.items():
    print(f"{key}: {value}")
print("Request Method:", os.environ.get("REQUEST_METHOD"))
print("Query String:", os.environ.get("QUERY_STRING"))
print("Path Info:", os.environ.get("PATH_INFO"))
print("Script Name:", os.environ.get("SCRIPT_NAME"))
print("Remote Address:", os.environ.get("REMOTE_ADDR"))
print("Server Protocol:", os.environ.get("SERVER_PROTOCOL"))
print("Server Software:", os.environ.get("SERVER_SOFTWARE"))
print("HTTP Headers:")
for key, value in os.environ.items():
    if key.startswith("HTTP_"):
        print(f"{key[5:]}: {value}")
print("Content Length:", os.environ.get("CONTENT_LENGTH"))
print("Content Type:", os.environ.get("CONTENT_TYPE"))
print("User Agent:", os.environ.get("HTTP_USER_AGENT"))
print("Accept:", os.environ.get("HTTP_ACCEPT"))
print("Accept Language:", os.environ.get("HTTP_ACCEPT_LANGUAGE"))
print("Accept Encoding:", os.environ.get("HTTP_ACCEPT_ENCODING"))
print("Connection:", os.environ.get("HTTP_CONNECTION"))
print("Cookie Headers:")
cookies = os.environ.get("HTTP_COOKIE", "")
if cookies:
    for cookie in cookies.split(';'):
        name, value = cookie.strip().split('=', 1)
        print(f"{name}: {value}")
print("Custom Headers:")
for key, value in os.environ.items():
    if key.startswith("HTTP_X_"):
        print(f"{key[8:]}: {value}")
print("End of CGI script output.")