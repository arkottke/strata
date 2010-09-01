#!/usr/bin/env python

import cgi
import os

# Add debug support
import cgitb
cgitb.enable()

strata_dir = '/var/www/html/pub/'
strata_url = 'http://accipter.org/pub/'

# Request is made with: http://accipter.org/cgi-bin/strata.cgi?action=query&revision=2
form = cgi.FieldStorage()
action = form.getfirst('action', 'download').lower()

# Create a list of Strata versions
revisions = {}
latestRev = None

for f in os.listdir(strata_dir):
	 if f.startswith('Strata-rev-'):
	 	rev = f[11:-4]
		revisions[rev] = f
		# Track the latest revision
		if (latestRev is None) or rev > latestRev:
			latestRev = rev

if action == 'query':
	print 'Content-type: text/html\r\n'
	print latestRev
elif action == 'download':
	requestedRev = form.getfirst('revision', 'latest').lower()
	
	if requestedRev == 'latest':
		requestedRev = latestRev

	if revisions.has_key(requestedRev):
		url = strata_url + revisions[requestedRev]
		print 'Location: %s\r\n' % url
	else:
		print 'Content-type: text/html\r\n'
		print 'Revision %s not found on the server.' % requestedRev
