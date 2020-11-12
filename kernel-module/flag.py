#!/usr/bin/python3

"""
flag.py

For sending flags to the server from the kernel module.
You may add additional code/checks here.
"""

import requests
result = requests.get(f"http://localhost:295/?mode=generate&ip=&challenge_name={sys.argv[1]}&flag={sys.argv[2]}").text
