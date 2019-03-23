#!/bin/python

from numpy import genfromtxt
import scipy.misc

image_array = genfromtxt('../csv_image_import.csv',delimiter=',')
scipy.misc.toimage(image_array, cmin= 0.0, cmax = 255).save('image_from_csv.jpg')
