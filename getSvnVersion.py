#!/usr/bin/env python
# encoding: utf-8

from __future__ import print_function

import re
import subprocess
import sys

# Prefix for the printing
if len(sys.argv) > 1:
    print(sys.argv[1], end=' ')

# Determine the versions
resp = subprocess.check_output(['svnversion', '.'])
resp = resp.decode('utf-8')

# Find the latest version
m = re.search(r'(?:\d+:)?(\d+)(?:[MS]+)?', resp)

if m:
    print(m.group(1))
else:
    print(0)
