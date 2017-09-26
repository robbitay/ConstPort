
import sys, os, re
import msvcrt

print("Hello Colby! :D")

print("Arguments:", sys.argv)

if (len(sys.argv) >= 2):
#
	print("Argument1: { ", end="")
	arg1 = sys.argv[1]
	for cIndex in range(0, len(arg1)):
	#
		print("%02X " % ord(arg1[cIndex]), end="")
	#
	print("}")
#
sys.stdout.flush()

counter = 0
while(True):
#
	newCharacter = sys.stdin.read(1)
	print("Char \'%s\'" % (newCharacter))
	sys.stdout.flush()
#