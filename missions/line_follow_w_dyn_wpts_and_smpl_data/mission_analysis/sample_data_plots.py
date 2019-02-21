#!/bin/python

import scipy
import numpy as np
import csv
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

# imports waypoint data
wp_file = open("../position_output.csv")
row_count = sum(1 for row in wp_file)  # fileObject is your csv.reader
print ("Data file has  ",row_count," lines in it")
data = np.genfromtxt('../position_output.csv', delimiter=',',names=['x', 'y'])

print("Generating graph: ")
fig = plt.figure()  # Generates figure
#ax = Axes3D(fig)    # Can use the 3D module to create graph, will need to add do appropriate conversion for other lines
ax = plt.gca()
ax.plot(data['x'], data['y'], color='r', label='the data')
ax.set_xlabel('x-axis [m]')
ax.set_ylabel('y-axis [m]')

plt.show()
fig.savefig("graph.png")

quit()