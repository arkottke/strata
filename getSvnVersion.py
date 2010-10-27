import re
import subprocess
import sys

# Prefix for the printing
if len(sys.argv) > 1:
    print sys.argv[1],

# Determine the versions 
p = subprocess.Popen(['svnversion', '.'], stdout=subprocess.PIPE)

# Find the latest version
m = re.search(r'(?:\d+:)?(\d+)(?:[MS]+)?', p.stdout.readline())

if m:
    print m.group(1)
else:
    print 0
