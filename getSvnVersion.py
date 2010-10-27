import subprocess
import re

# Determine the versions 
p = subprocess.Popen(['svnversion', '.'], stdout=subprocess.PIPE)

# Find the latest version
m = re.search(r'(?:\d+:)?(\d+)(?:[MS]+)?', p.stdout.readline())

if m:
    print m.group(1)
else:
    print 0
