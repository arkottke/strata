#!/usr/bin/python

from pyx import *
from numpy import *
from numpy.fft import *

fileName = "/home/albert/docs/school/research/thesis/motions/analysis/KOBE/NIS090.AT2"

# Plot sizes
plotHeight = 2
plotWidth = 12


# Open the motion and read the data
# Load the file
try:
	fin = open(fileName, 'r')
except:
	print "Unable to open file:", fileName
	exit(2)

# Read the data.  First three lines are header lines
for i in range(3):
	fin.readline()

line = fin.readline().split()
pointCount = int(line[0])
timeStep = float(line[1])

# Read in the acceleration data
data = []
for line in fin:
	for value in line.split():
		data.append( float(value) )

time = timeStep * arange(pointCount)
accel = array(data)

print "Input PGA:", max(abs(accel))

g = graph.graphxy(height= plotHeight, width= plotWidth,
		x=graph.axis.linear(min=5, max=20, title="Time (s)"),
		y=graph.axis.linear(min=-1, max=1, title="Accel (g)"))
g.plot( graph.data.values(x=time, y=accel ), 
		[graph.style.line([color.rgb.blue, style.linewidth.Thin])])

g.text( plotWidth - 1, plotHeight - 0.5, "(a)")

g.writePDFfile("td-rock-accel-ts")

# Compute the  Fourier amplitude spectrum
fas = fft(accel)
freq = fftfreq( pointCount, timeStep )
n = len(fas)

g = graph.graphxy(height= plotHeight, width= plotWidth,
		x=graph.axis.linear(min=0., max=15., title="Frequency (Hz)"),
		y=graph.axis.linear(min=0., max=75., title="Fourier Amp. (g-s)") )
g.plot( graph.data.values( x=freq[0:n/2], y=abs(fas[0:n/2]) ), 
		[graph.style.line([color.rgb.blue, style.linewidth.Thin])])

g.text( plotWidth - 1, plotHeight - 0.5, "(b)")

g.writePDFfile("td-rock-fas")


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
		x=graph.axis.linear(min=0, max=15, title="Frequency (Hz)"),
		y=graph.axis.linear(min=0, max=4, title="$|TF|$") )
g.plot( graph.data.values( x=freq[0:n/2], y=abs(tf[0:n/2]) ), 
		[graph.style.line([color.rgb.blue, style.linewidth.Thin])])
g.text( plotWidth - 1, plotHeight - 0.5, "(c)")

g.writePDFfile("td-rock-surface-tf")


# Apply the transfer function to the motion
surfFas = tf * fas

g = graph.graphxy(height= plotHeight, width= plotWidth,
		x=graph.axis.linear(min=0, max=15, title="Frequency (Hz)"),
		y=graph.axis.linear(min=0, max=75, title="Fourier Amp. (g-s)") )
g.plot( graph.data.values( x=freq[0:n/2], y=abs(surfFas[0:n/2]) ), 
		[graph.style.line([color.rgb.blue, style.linewidth.Thin])])
g.text( plotWidth - 1, plotHeight - 0.5, "(d)")

g.writePDFfile("td-surface-fas")


# Compute the time series
surfAccelTs = real(ifft(surfFas))

print "Surface PGA:", max(abs(surfAccelTs))

g = graph.graphxy(height= plotHeight, width= plotWidth,
		x=graph.axis.linear(min=5, max=20, title="Time (s)"),
		y=graph.axis.linear(min=-1, max=1, title="Accel (g)") )
g.plot( graph.data.values( x=time, y=surfAccelTs ), 
		[graph.style.line([color.rgb.blue, style.linewidth.Thin])])
g.text( plotWidth - 1, plotHeight - 0.5, "(e)")

g.writePDFfile("td-surface-accel-ts")
