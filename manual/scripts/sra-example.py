#!/usr/bin/python

from numpy import *
import csv, sys

def loadCsvData( filename ):
	reader = csv.reader(open(filename))
		
	reference = []
	values = []
	median = []
	stdev = []
	hasStats = 0

	try:
		for row in reader:
			if row[0].startswith('#'):
				continue
			
			if not len(values):
				if len(row) == 2:
					values = []
				else:
					values = [[] for i in range(len(row)-3)]
					hasStats = 1

			reference.append(float(row[0]))

			if len(row) == 2:
				# No average and standard deviation
				values.append(float(row[1]))
			else:
				# Replace empty locations with a zero
				for i in range(1, len(row)-2):
					if row[i] == '':
						row[i] = "0."
				# invidividual values
				for v,p in zip(values,row[1:-2]):
					v.append(float(p))
				
				# Average and standard deviation
				median.append(float(row[-2]))
				stdev.append(float(row[-1]))

		if not hasStats:
			return array(reference), array(values)
		else:
			return array(reference), array(values), array(stdev), array(median)

	except csv.Error, e:
		sys.exit('file %s, line %d: %s' % (filename, reader.line_num, e))

if __name__ == "__main__":
	from pyx import *

	# Transfer functions
	freq1, tf1 = loadCsvData( "data/strata-example/Bedrock (Outcrop) to 0 (Outcrop)-transFunc.csv" )
	freq2, tf2 = loadCsvData( "data/strata-example/Bedrock (Within) to 0 (Outcrop)-transFunc.csv" )
	
	g = graph.graphxy(width=8,
		x=graph.axis.linear(title=r"Frequency (Hz)"),
		y=graph.axis.linear(title=r"$|$TF$|$"),
		key=graph.key.key(pos="tr"))
		
	g.plot( [ 
		graph.data.values( x=freq1, y=tf1, title=r"Surface / Outcrop"),
		graph.data.values( x=freq2, y=tf2, title=r"Surface / Within")],
		[ graph.style.line([
			attr.changelist([ style.linestyle.solid, style.linestyle.dashed ] ), 
			attr.changelist([ color.cmyk.Blue, color.cmyk.BrickRed])])])
	
	g.writePDFfile("sra-transFunc")
	#g.writeEPSfile("sra-transFunc")
	
	
	# Strain time history
	time, strain = loadCsvData( "data/strata-example/25 (Outcrop)-strainTs.csv" )

	effStrain = 0.65 * max(abs(strain))

	g = graph.graphxy(width=8,
		x=graph.axis.linear(title=r"Time (s)"),
		y=graph.axis.linear(title=r"Shear Strain (\%)"),
		key=graph.key.key(pos="br"))
		
	g.plot( [ 
		graph.data.values( x=time, y=strain, title=r"Time Series" ),
		graph.data.values( x=[time[0],time[-1]], y=[effStrain,effStrain], title=r"Effective Strain")],
		[ graph.style.line([
			attr.changelist([ style.linestyle.solid, style.linestyle.dashed ] ), 
			attr.changelist([ color.cmyk.Blue, color.cmyk.BrickRed])])])

	g.writePDFfile("sra-strain-ts")
	#g.writeEPSfile("sra-strain-ts")

	

