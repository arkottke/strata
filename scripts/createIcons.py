#!/usr/bin/python

import os

files = [ 'resources/images/application-icon',
		'resources/images/file-data']

sizes = [ 128, 64, 32, 16]

# Create the png files
for file in files:
	pngFiles = ""
	for size in sizes:
		pngFile = "temp-%i.png" % (size)
		os.system( "inkscape --without-gui --export-png=%s --export-dpi=72 --export-background-opacity=0 --export-width=%i --export-height=%i %s.svg > /dev/null" % ( pngFile, size, size, file ) )
		pngFiles += pngFile + " "

	# Combine them into an ico container
	os.system( "convert %s -background white -depth 8 %s.ico" % ( pngFiles, file ))

	# Delete the temporary files
	#os.system( "rm %s" % (pngFiles) )

