#!/usr/bin/python3

"""
generate_flag.py

For sending flags to the server from the kernel module.
You may add additional code/checks here.
"""

import requests

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        result = requests.get(f"http://localhost:295/?mode=generate&ip=&challenge_name={sys.argv[1]}&flag={sys.argv[2]}").text
	# Add more
    else:
        print("[*] Usage: python3 generate_flag.py (challenge_name) (flag)")
