#!/usr/bin/env python3

# arg1: X from videoX
# arg2: test time (s)

from cv2 import VideoCapture
from sys import argv
from os import system

capture = VideoCapture(int(argv[1]))
capture.read()
system("sleep " + argv[2])
capture.release()
