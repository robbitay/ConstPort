
import sys, os, re

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
		sys.stdout.write("   ")
		justWroteNewLine = True
	#
	else:
	#
		if (justWroteNewLine == False): sys.stdout.write(" ")
		if (justWroteNewLine and ord(newCharacter) <= 0x05):
		#
			sys.stdout.write("%c" % (newCharacter))
		#
		else:
		#
			sys.stdout.write("%02X" % (ord(newCharacter)))
			justWroteNewLine = False
		#
		# sys.stdout.write("%02X" % (ord(newCharacter)))
		# justWroteNewLine = False
	#
	sys.stdout.flush()
#