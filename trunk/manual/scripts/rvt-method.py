#!/usr/bin/python

from pyx import *
from numpy import *
import sys

# Plot sizes
plotHeight = 4
plotWidth = 10

dataFile = open('data/rvt-inversion-data.csv', 'r')
tmp = [[],[]]

for line in dataFile:
	if line.startswith("#"):
		continue

	parts = line.split(',')

	tmp[0].append( float(parts[12]) )
	tmp[1].append( float(parts[13]) )

freq = array( tmp[0] )
fas = array( tmp[1] )

g = graph.graphxy(height= plotHeight, width= plotWidth,
		x=graph.axis.log(min=0.1, max=100., title="Frequency (Hz)"),
		y=graph.axis.log(min=1e-4, max=1e0, title="Fourier Amp. (g-s)") )
g.plot( graph.data.values( x=freq, y=fas), 
		[graph.style.line([color.rgb.blue])])

g.text( plotWidth - 1, plotHeight - 0.5, "(a)")

g.writePDFfile("rvt-rock-fas")


# Compute the transfer function
soilHeight = 50.
soilDensity = 1.93
soilVs = 350.
soilDamping = 0.07

rockDensity = 2.24
rockVs = 1500.
rockDamping = 0.01

# Impedence Ratio
impRatio = ( soilDensity * soilVs * ( 1 + 1j * soilDamping ) ) / ( rockDensity * rockVs * ( 1 + 1j * rockDamping ) )
# Wave number
waveNo = (2 * pi * freq ) / ( soilVs * ( 1 + 1j * soilDamping ) )

tf = 1 / ( 0.5 * ( 1 + impRatio ) * exp( 1j * waveNo * soilHeight ) + 0.5 * (1 - impRatio) * exp( -1j * waveNo * soilHeight ) )

g = graph.graphxy(height= plotHeight, width= plotWidth,
		x=graph.axis.log(min=0.1, max=100., title="Frequency (Hz)"),
		y=graph.axis.log(min=0.01, max=10., title="$|TF|$") )
g.plot( graph.data.values( x=freq, y=abs(tf) ), 
		[graph.style.line([color.rgb.blue])])
g.text( plotWidth - 1, plotHeight - 0.5, "(b)")

g.writePDFfile("rvt-rock-surface-tf")


# Apply the transfer function to the motion
surfFas = abs(tf) * fas

g = graph.graphxy(height= plotHeight, width= plotWidth,
		x=graph.axis.log(min=0.1, max=100, title="Frequency (Hz)"),
		y=graph.axis.log(min=1e-4, max=1e0, title="Fourier Amp. (g-s)") )
g.plot( graph.data.values( x=freq, y=surfFas ), 
		[graph.style.line([color.rgb.blue])])
g.text( plotWidth - 1, plotHeight - 0.5, "(c)")

g.writePDFfile("rvt-surface-fas")

def rvtCalc(freq, fas):
	m0 = 2 * trapz( x=freq, y=(2.*pi*freq)**0 * fas**2)
	m2 = 2 * trapz( x=freq, y=(2.*pi*freq)**2 * fas**2)
	m4 = 2 * trapz( x=freq, y=(2.*pi*freq)**4 * fas**2)

	bandWidth = sqrt((m2*m2)/(m0 * m4))
	Tgm = 8.2
	numExtrema = sqrt(m4/m2)*Tgm/pi

	z = arange( 0, 10, 0.0001)
	peakFactor = sqrt(2.) * trapz( x=z, y=(1. - (1. - bandWidth * exp(-z * z)) ** numExtrema ))

	arms = sqrt(m0/Tgm)

	print "m0: %g m2: %g m4: %g" % (m0, m2, m4)
	print "Bandwidth:", bandWidth
	print "Number of Extrema:", numExtrema
	print "Peakfactor:", peakFactor
	print "arms:", arms
	print "Expected max:", peakFactor * arms

# Compute the moments
print "Input FAS"
rvtCalc(freq, fas)

print "Sufrace FAS"
rvtCalc(freq, surfFas)
