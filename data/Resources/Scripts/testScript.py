#This script is designed to be python 2.7 and 3.3 compatible

import sys, os, time

print("Hello from Python!")
sys.stdout.flush()

value = 0
while (1):
#
	time.sleep(1)
	print("Value = %u" % value)
	value += 1
	sys.stdout.flush()
	
	if (value >= 4): break
#