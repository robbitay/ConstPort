
import sys, os, re

print("hex.py started!")
sys.stdout.flush()

counter = 0
justWroteNewLine = True
while(True):
#
	newCharacter = sys.stdin.read(1)
	
	if (newCharacter == '~'): print("")
	sys.stdout.write("[%02X] " % (ord(newCharacter)))
	
	sys.stdout.flush()
#