#!/usr/bin/env python3

import datetime

def main():
    print("Content-Type: text/html\r\n\r\n")
    print("<h1>PYTHONE!!!!!!</h1>")
    now = datetime.datetime.now()
    print(f"<p>Current date and time: {now.strftime('%Y-%m-%d %H:%M:%S')}</p>")

if __name__ == "__main__":
    main()