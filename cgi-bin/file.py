#!/usr/bin/env python3

import datetime

def main():
    print("PYTHONE!!!!!!\n")
    now = datetime.datetime.now()
    print("Current date and time:", now.strftime("%Y-%m-%d %H:%M:%S"))

if __name__ == "__main__":
    main()