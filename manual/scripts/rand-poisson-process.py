#!/usr/bin/python

import math
from pyx import *
from numpy import *
from numpy.random import exponential

def stepify( series ):
	index = [1]
	value = [0]

	for s in series:
		# Right side point
		index.append(index[-1])
		value.append(s)

		# Increment the index for the left side point
		index.append(index[-1]+1)
		value.append(s)

	# Use 0.5 as the last index
	index[-1] -= 0.5

	return array(index), array(value)

def plotSteps( title, series ):
	x,y = stepify(series)
	g = graph.graphxy(height=6., width=6.,
			x=graph.axis.lin(title="Layer Number", min=0, max=math.ceil(x[-1]), density=2),
			y=graph.axis.lin(title="Depth", min=0, reverse=1) )

	x,y =  stepify(series)
	g.plot( graph.data.values( x=x, y=y), 
			[graph.style.line([color.rgb.blue])])

	g.writePDFfile(title)

def plotTransform( title, unit_pp, series, function, axis_title ):
	g = graph.graphxy(height=6., width=8.,
			y=graph.axis.linear(title="Depth ($\lambda=1$)"),
			x=graph.axis.linear(title=axis_title, min=0, max=1.1*max(series)))

	g.plot(graph.data.function(function))
	g.finish()

	for i in arange(len(unit_pp)):
		x0, y0 = g.pos( 0., unit_pp[i] )
		x1, y1 = g.pos( series[i], unit_pp[i] )
		x2, y2 = g.pos( series[i], 0. )

		g.stroke(path.line( x0, y0, x1, y1), [deco.earrow.normal])
		g.stroke(path.line( x1, y1, x2, y2), [deco.earrow.normal])

	g.writePDFfile(title)


# Create homogenous Poisson process
unit_pp = cumsum(exponential( scale=1.0, size=10))
plotSteps( 'poisson-unit-pp', unit_pp )


# Create plot warping a constant occurance rate of 0.20
rate = 0.20
pp = unit_pp / rate
plotSteps( 'poisson-pp', pp )
plotTransform( 'poisson-pp-warp', unit_pp, pp, 'y(x)=0.20*x', 'Depth ($\lambda=0.2$)')


# Convert warp from homogenous into nonhomogenous
a = 1.98
b = 10.86
c = -0.89
npp = ( ( c * unit_pp ) / a + unit_pp / a + b ** (c+1) ) ** (1/(c+1)) - b

plotSteps( 'poisson-npp', npp )
plotTransform( 'poisson-npp-warp', unit_pp, npp, 
	'y(x)=1.98*((x+10.86)**(-0.89+1.) / (-0.89+1.) - 10.86**(-0.89+1.)/(-0.89+1.))',
	'Depth ($\lambda(d)$)')
