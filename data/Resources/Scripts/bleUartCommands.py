
import sys, os, re

BleModCmd_GetBridgeInfo        = 0x01
BleModCmd_GetStatus            = 0x02
BleModCmd_Pair                 = 0x03
BleModCmd_Unpair               = 0x04
BleModCmd_UpdateStart          = 0x05
BleModCmd_FlashWriteRow        = 0x06
BleModCmd_UpdateComplete       = 0x07
BleModCmd_ButtonHeldDone       = 0x08
BleModCmd_SetRadioSettings     = 0x09
BleModCmd_GetRadioSettings     = 0x0A
BleModCmd_SetQosConfig         = 0x0B
BleModCmd_GetQosConfig         = 0x0C
BleModCmd_RadioUpdateStart     = 0x0D
BleModCmd_RadioFlashWriteRow   = 0x0E
BleModCmd_RadioUpdateComplete  = 0x0F
BleModCmd_BleConnected         = 0x10
BleModCmd_BleDisconnected      = 0x11
BleModCmd_Register             = 0x12
BleModCmd_Reset                = 0x13
BleModCmd_GetAllState          = 0x14
BleModCmd_GetRegistrationInfo  = 0x15
BleModCmd_GetVoltageLevels     = 0x16
BleModCmd_Deploy               = 0x17
BleModCmd_ForceNextPair        = 0x18
BleModCmd_SetFailSafeOption    = 0x19
BleModCmd_GetFailSafeOption    = 0x1A
BleModCmd_GetOperatingValues   = 0x1B
BleModCmd_GetResetCauses       = 0x1C
BleModCmd_ClearResetCauses     = 0x1D
BleModCmd_SetDebugModeEnabled  = 0x1E
BleModCmd_GetDebugModeEnabled  = 0x1F
BleModCmd_GetLastPacketTime    = 0x20
BleModCmd_GetAllVersions       = 0x21
BleModCmd_GetRadioUpdateStatus = 0x22
BleModCmd_GetHoppingTable      = 0x23
BleModCmd_SendPacket           = 0x24
BleModCmd_GetAppPicVersion     = 0x25
BleModCmd_GetRadioPicVersion   = 0x26
BleModCmd_SetCriticalBluetooth = 0x27
BleModCmd_SetWiegandLedMode    = 0x28
BleModCmd_GetWiegandLedMode    = 0x29
BleModCmd_DebugOutput          = 0x2A
BleModCmd_BootloaderStart      = 0x2B

ATTN_CHAR = 0x7E
CMD_HEADER_SIZE = 4
debugWriteHex = True

debugPrefix     = str(chr(0x01))
infoPrefix      = str(chr(0x02))
alertPrefix     = str(chr(0x03))
importantPrefix = str(chr(0x04))
wroteNewLine = True

def WriteCharacter(c):
#
	sys.stdout.write(c)
#
def WriteString(prefix, line):
#
	global wroteNewLine
	for c in line:
	#
		if (wroteNewLine):
		#
			for p in prefix:
			#
				WriteCharacter(p)
			#
			wroteNewLine = False
		#
		
		WriteCharacter(c)
		
		if (c == '\n'): wroteNewLine = True
	#
#

def DEBUG_Write(line):
#
	WriteString(debugPrefix, line)
	sys.stdout.flush()
#
def DEBUG_WriteLine(line):
#
	WriteString(debugPrefix, line + "\n")
	sys.stdout.flush()
#

def INFO_Write(line):
#
	WriteString(infoPrefix, line)
	sys.stdout.flush()
#
def INFO_WriteLine(line):
#
	WriteString(infoPrefix, line + "\n")
	sys.stdout.flush()
#

def ALERT_Write(line):
#
	WriteString(alertPrefix, line)
	sys.stdout.flush()
#
def ALERT_WriteLine(line):
#
	WriteString(alertPrefix, line + "\n")
	sys.stdout.flush()
#

def IMPORTANT_Write(line):
#
	WriteString(importantPrefix, line)
	sys.stdout.flush()
#
def IMPORTANT_WriteLine(line):
#
	WriteString(importantPrefix, line + "\n")
	sys.stdout.flush()
#

print("bleUartCommands.py started!")
sys.stdout.flush()

counter = 0
justWroteNewLine = True
dataBuffer = []
while(True):
#
	newCharacters = sys.stdin.read(1)
	# print("Char \'%s\'" % (newCharacter))
	
	for newCharacter in newCharacters:
	#
		newCharacter = ord(newCharacter)
		
		if (len(dataBuffer) == 0):
		#
			if (newCharacter == ATTN_CHAR):
			#
				dataBuffer.append(newCharacter)
			#
			else:
			#
				ALERT_WriteLine("Dropped 0x%02X" % (newCharacter))
			#
		#
		else:
		#
			dataBuffer.append(newCharacter)
			
			if (len(dataBuffer) >= CMD_HEADER_SIZE):
			#
				attn = dataBuffer[0]
				cmd = dataBuffer[1]
				length = dataBuffer[2] + (dataBuffer[3] >> 8)
				payload = dataBuffer[4:]
				
				if (len(dataBuffer) == CMD_HEADER_SIZE + length):
				#
					if (debugWriteHex):
					#
						DEBUG_Write("CMD %02X %u byte(s): { " % (cmd, length))
						for bIndex in range(len(dataBuffer)):
						#
							b = dataBuffer[bIndex]
							if (bIndex == 0): DEBUG_Write("ATTN ")
							elif (bIndex == 1): DEBUG_Write("[%02X] " % b)
							elif (bIndex == 2): DEBUG_Write("[%02X"   % b)
							elif (bIndex == 3): DEBUG_Write("%02X] "  % b)
							else: DEBUG_Write("%02X " % b)
						#
						DEBUG_WriteLine("}")
					#
					
					# if (cmd == BleModCmd_GetBridgeInfo):
					# #
					# 	INFO_WriteLine("GetBridgeInfo")
					# #
					# elif (cmd == BleModCmd_GetStatus):
					# #
					# 	INFO_WriteLine("GetStatus")
					# #
					# elif (cmd == BleModCmd_DebugOutput):
					# #
					# 	INFO_Write("b-")
					# 	for p in payload[1:]:
					# 	#
					# 		INFO_Write("%c" % p)
					# 	#
					# 	INFO_WriteLine("")
					# #
					# else:
					# #
					# 	ALERT_WriteLine("Unknown %u byte CMD %02X!" % (length, cmd))
					# #
					
					dataBuffer = []
				#
			#
		#
	#
#