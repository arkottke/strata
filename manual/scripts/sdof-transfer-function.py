#!/usr/bin/python

from pyx import *
from numpy import *


def sdofTf( natFreq, damping, freq ):
	return  -natFreq ** 2 / (( freq ** 2 - natFreq ** 2 ) - 2.j * damping * natFreq * freq )

freq = logspace( -1, 1, num=1024)
damping = 0.05

tf = array( 
		[ sdofTf( 0.5, damping, freq ),
		  sdofTf( 5.0, damping, freq ) ] )

g = graph.graphxy(height=5, width=7,
		x=graph.axis.log(title="Frequency (Hz)"),
		y=graph.axis.linear(title="$|TF|$") )
g.plot( graph.data.values( x=freq, y=abs(tf[0]) ), 
		[graph.style.line([color.rgb.blue, style.linewidth.Thin])])
g.plot( graph.data.values( x=freq, y=abs(tf[1]) ), 
		[graph.style.line([color.rgb.blue, style.linewidth.Thin])])

g.writePDFfile("sdof-transfer-function")
