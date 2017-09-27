
import sys, os, re
import msvcrt

print("asciiHex.py started!")
sys.stdout.flush()

counter = 0
justWroteNewLine = True
while(True):
#
	newCharacter = sys.stdin.read(1)
	# print("Char \'%s\'" % (newCharacter))
	if (newCharacter == '\n'):
	#
		print("")
		justWroteNewLine = True
	#
	elif (newCharacter == ' '):
	#
		print("   ", end="")
		justWroteNewLine = True
	#
	else:
	#
		if (justWroteNewLine == False): print(" ", end="")
		if (justWroteNewLine and ord(newCharacter) <= 0x05):
		#
			print("%c" % (newCharacter), end="")
		#
		else:
		#
			print("%02X" % (ord(newCharacter)), end="")
			justWroteNewLine = False
		#
	#
	sys.stdout.flush()
#