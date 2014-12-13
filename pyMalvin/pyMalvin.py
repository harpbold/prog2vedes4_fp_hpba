#!/usr/bin/env python
from imposm.parser import OSMParser
import operator

class Bus(object):
	def __init__(self, name):
		self.name = name
		self.nodes = []
		
class Node(object):
	def __init__(self, lat, lon):
		self.lat = lat
		self.lon = lon
		
buses = {}
nds = {}

class BusHandler(object):
	highways = 0

	def ways(self, ways):
		for osmid, tags, refs in ways:
			if 'highway' in tags:
				self.highways += 1
				
	def nodes(self, nodes):
		for osmid, tags, refs in nodes:
			nds[osmid] = Node(refs[0], refs[1])
			
	def relations(self, relations):
		for osmid, tags, refs in relations:
			if 'route' in tags and tags['route'] == 'bus':
				if 'ref' in tags:
					name = tags['ref']
				else:
					name = tags['name']
				buses[osmid] = Bus(name)
				for x in refs:
					if 'stop' and 'node' in x:
						#print x
						buses[osmid].nodes.append(x[0])
				
	def printBuses(self):
		with open("/home/hpba/pyMalvin.dat", "w") as f:
			for busId in sorted(buses, key=buses.get):
				cnt = 0
				f.write(buses[busId].name)
				f.write("\n")
				for node in buses[busId].nodes:
					f.write("#" + str(cnt))
					cnt += 1
					f.write(" Lat: " + str(nds[node].lat))
					f.write(" Lon: " + str(nds[node].lon))
					f.write("\n")
			

handler = BusHandler()
p = OSMParser(concurrency=4, nodes_callback=handler.nodes, \
							 ways_callback=handler.ways, \
							 relations_callback=handler.relations)
p.parse('/home/hpba/Downloads/debrecen.osm')
handler.printBuses()
