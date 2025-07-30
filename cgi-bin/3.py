#!/usr/bin/env python
import os
import sys

sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("Set-Cookie: user_id=12345; Path=/; HttpOnly\r\n")
sys.stdout.write("\r\n")  # End of headers

# HTML Body
sys.stdout.write("<html>\n")
sys.stdout.write("<body>\n")
sys.stdout.write("<h1>CGI Cookie Test</h1>\n")
sys.stdout.write("<p>Incoming Cookies: %s</p>\n" % os.environ.get('HTTP_COOKIE', ''))
sys.stdout.write("<p>Sent Set-Cookie: user_id=12345</p>\n")
sys.stdout.write("</body>\n")
sys.stdout.write("</html>\n")
