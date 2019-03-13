#!/bin/python

import scipy
import numpy as np
import csv
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

# imports waypoint data
pos_file = open("../position_output.csv")
row_count = sum(1 for row in pos_file)  # fileObject is your csv.reader
print ("Data file has  ",row_count," lines in it")
data = np.genfromtxt('../position_output.csv', delimiter=',',names=['x', 'y'])

# Farm version #1 (~horizontal lines)
#wpx = [0,100,200,200,100,100,225,225,100];
#wpy = [0,-50,-50,-75,-100,-125,-175,-200,-200];
# Farm version #2 (~vertical lines)
wpx = [50,50,75,75,110,125,150,175];
wpy = [-50,-150,-175,-25,-25,-175,-175,-50];


print("Generating graph: ")
fig = plt.figure()  # Generates figure
#ax = Axes3D(fig)    # Can use the 3D module to create graph, will need to add do appropriate conversion for other lines
ax = plt.gca()
ax.scatter(wpx,wpy)
ax.plot(data['x'], data['y'], color='r', label='the data')
ax.set_xlabel('x-axis [m]')
ax.set_ylabel('y-axis [m]')

plt.show()
fig.savefig("graph.png")
