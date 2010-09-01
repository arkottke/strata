#!/usr/bin/python

import os, subprocess, sys

def usage():
	print 'Usage: %s [path or file]' % sys.argv[0]
	sys.exit(1)

def convertFile(file, root = ''):
	if len(root):
		absname = os.path.join(root,file[0:-4])
	else:
		absname = file[0:-4]

	print 'Coverting: %s.pdf' % absname
	subprocess.call('convert -density 600x600 -quality 90 -size 1024 %s.pdf %s.png' % ( absname, absname), shell=True )

if len(sys.argv) == 2:
	input = sys.argv[1]

	if os.path.isfile(input):
		convertFile(input)
	elif os.path.isdir(input):
		for root, dirs, files in os.walk(input):
			# Don't process hidden root folders
			if root.count( os.path.sep + '.' ):
				continue

			for f in files:
				if f.endswith('pdf'):
					convertFile(f, root)
	else:
		usage()
else:
	usage()

