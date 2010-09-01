#!/usr/bin/python

from pyx import *
from numpy import *

dataFile = open("data/rvt-inversion-data.csv", 'r')

target = { 'period': [], 'sa': [] }
ratio = { 'period': [], 'sa': [], 'error': [], 'freq': [], 'fas': [] }
extrap = { 'period': [], 'sa': [], 'error': [], 'freq': [], 'fas': [] }
forced = { 'period': [], 'sa': [], 'error': [], 'freq': [], 'fas': [] }

# Load the data
for line in dataFile:
	if line.startswith("#"):
		continue

	parts = line.split(',')

	col = 0
	if parts[col]:
		target['period'].append(float(parts[col]))
	
	col += 1	
	if parts[col]:
		target['sa'].append(float(parts[col]))

	for d in [ ratio, extrap, forced ]:
		col += 1
		if parts[col]:
			d['period'].append(float(parts[col]))
		col += 1
		if parts[col]:
			d['sa'].append(float(parts[col]))
		col += 1
		if parts[col]:
			d['freq'].append(float(parts[col]))
		col += 1
		if parts[col]:
			d['fas'].append(float(parts[col]))

# Plot the response spectra
g = graph.graphxy(height=6, width=10,
		x=graph.axis.log(title="Period (s)"),
		y=graph.axis.log(title="Spectral Accel. (g)"),
		key = graph.key.key(pos="bl", dist=0.1) )
g.plot( graph.data.values( title="Ratio Corrected", x=ratio['period'], y=ratio['sa'] ), 
	[graph.style.line([color.rgb.green, style.linewidth.Thick])] )
g.plot( graph.data.values( title="Ratio \& Extrapolated", x=extrap['period'], y=extrap['sa'] ), 
	[graph.style.line([color.rgb.blue, style.linewidth.Thick])] )
g.plot( graph.data.values( title="Ratio, Extrap., \& Slope Forced", x=forced['period'], y=forced['sa'] ), 
	[graph.style.line([color.rgb.red, style.linewidth.Thick])] )
g.plot( graph.data.values( title="Target", x=target['period'], y=target['sa'] ), 
	[graph.style.symbol( graph.style.symbol.circle, size=0.15, symbolattrs=[color.rgb.black])])

g.writePDFfile("irvt-respSpec.pdf")

# Plot the FAS
g = graph.graphxy(height=6, width=10,
		x=graph.axis.log(title="Frequency (Hz)"),
		y=graph.axis.log(title="$|$FAS$|$ (g-s)"),
		key = graph.key.key(pos="bl", dist=0.1) )
g.plot( graph.data.values( title="Ratio Corrected", x=ratio['freq'], y=ratio['fas'] ), 
	[graph.style.line([color.rgb.green, style.linewidth.Thick])] )
g.plot( graph.data.values( title="Ratio \& Extrapolated", x=extrap['freq'], y=extrap['fas'] ), 
	[graph.style.line([color.rgb.blue, style.linewidth.Thick])] )
g.plot( graph.data.values( title="Ratio, Extrap., \& Slope Forced", x=forced['freq'], y=forced['fas'] ), 
	[graph.style.line([color.rgb.red, style.linewidth.Thick])] )

g.writePDFfile("irvt-fas.pdf")
		
# Compute error
for i in range(len(target['period'])):
	for d in [ratio, extrap, forced ]:
		d['error'].append( 100. * (d['sa'][i] - target['sa'][i]) / target['sa'][i] )

g = graph.graphxy(height=6, width=10,
		x=graph.axis.log(title="Period (s)"),
		y=graph.axis.linear(title="Relative Error (\%)"),
		key = graph.key.key(pos="br", dist=0.1) )
g.plot( graph.data.values( title="Ratio Corrected", x=ratio['period'], y=ratio['error'] ), 
	[graph.style.line([color.rgb.green, style.linewidth.Thick])] )
g.plot( graph.data.values( title="Ratio \& Extrapolated", x=extrap['period'], y=extrap['error'] ), 
	[graph.style.line([color.rgb.blue, style.linewidth.Thick])] )
g.plot( graph.data.values( title="Ratio, Extrap., \& Slope Forced", x=forced['period'], y=forced['error'] ), 
	[graph.style.line([color.rgb.red, style.linewidth.Thick])] )

g.writePDFfile("irvt-error.pdf")
