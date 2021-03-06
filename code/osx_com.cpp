/*
File:   osx_com.cpp
Author: Taylor Robbins
Date:   10\07\2017
Description: 
	** Holds the functions that handle interfacing with OSX enumerated COM ports 
*/

#include <dirent.h>

// +==============================+
// |      OSX_GetComPortList      |
// +==============================+
GetComPortList_DEFINITION(OSX_GetComPortList)
{
	BoundedStrList_t result = {};
	BoundedStrListCreate(&result, MAX_COM_PORT_NUM, MAX_COM_PORT_NAME_LENGTH, memArena);
	
	DIR* devDir = opendir("/dev/");
	if (devDir != nullptr)
	{
		dirent* deviceName = readdir(devDir);
		while (deviceName != nullptr)
		{
			const char* nameStr = deviceName->d_name;
			u32 nameStrLength = (u32)strlen(nameStr);
			if (strncmp(&nameStr[0], "tty", 3) == 0)
			{
				if (nameStrLength >= 4 && nameStr[3] == '.')
				{
					OSX_PrintLine("Found device \"%s\"", nameStr);
					BoundedStrListAdd(&result, nameStr);
				}
				else if (nameStrLength == 7 && nameStr[3] == 's' &&
					IsCharClassNumeric(nameStr[4]) && IsCharClassNumeric(nameStr[7]) && IsCharClassNumeric(nameStr[6]))
				{
					OSX_PrintLine("Found device \"%s\"", nameStr);
					BoundedStrListAdd(&result, nameStr);
				}
			}
			else
			{
				// OSX_PrintLine("Ignoring device \"%s\"", nameStr);
			}
			deviceName = readdir(devDir);
		}
	}
	
	#if 0
	#if DEBUG
	BoundedStrListAdd(&result, "tty.usbserial-A506KDE9");
	BoundedStrListAdd(&result, "ttys000");
	#endif
	#endif
	
	return result;
}

// +==============================+
// |       OSX_OpenComPort        |
// +==============================+
OpenComPort_DEFINITION(OSX_OpenComPort)
{
	ComPort_t result = {};
	result.handle = -1;
	result.settings = settings;
	
	char filePathBuffer[32];
	int printfResult = snprintf(filePathBuffer, ArrayCount(filePathBuffer), "/dev/%s", comPortName);
	
	printf("Attempting to open \"%s\"\n", filePathBuffer);
	int fileHandle = open(filePathBuffer, O_RDWR | O_NDELAY | O_NOCTTY);
	
	if (fileHandle != -1)
	{
		fcntl(fileHandle, F_SETFL, FNDELAY);
		
		termios termSettings = {};
		
		if (tcgetattr(fileHandle, &termSettings) != 0) { printf("tcgetattr call failed: %s!\n", GetErrnoName(errno)); }
		
		switch (settings.baudRate)
		{
			case BaudRate_110:    cfsetospeed(&termSettings, B110);    cfsetispeed(&termSettings, B110);    break;
			case BaudRate_300:    cfsetospeed(&termSettings, B300);    cfsetispeed(&termSettings, B300);    break;
			case BaudRate_600:    cfsetospeed(&termSettings, B600);    cfsetispeed(&termSettings, B600);    break;
			case BaudRate_1200:   cfsetospeed(&termSettings, B1200);   cfsetispeed(&termSettings, B1200);   break;
			case BaudRate_2400:   cfsetospeed(&termSettings, B2400);   cfsetispeed(&termSettings, B2400);   break;
			case BaudRate_4800:   cfsetospeed(&termSettings, B4800);   cfsetispeed(&termSettings, B4800);   break;
			case BaudRate_9600:   cfsetospeed(&termSettings, B9600);   cfsetispeed(&termSettings, B9600);   break;
			case BaudRate_14400:  cfsetospeed(&termSettings, B14400);  cfsetispeed(&termSettings, B14400);  break;
			case BaudRate_19200:  cfsetospeed(&termSettings, B19200);  cfsetispeed(&termSettings, B19200);  break;
			case BaudRate_38400:  cfsetospeed(&termSettings, B38400);  cfsetispeed(&termSettings, B38400);  break;
			case BaudRate_57600:  cfsetospeed(&termSettings, B57600);  cfsetispeed(&termSettings, B57600);  break;
			case BaudRate_115200: cfsetospeed(&termSettings, B115200); cfsetispeed(&termSettings, B115200); break;
			default: printf("Unknown Baud Rate selected!\n"); return result;
		}
		
		/* Setting other Port Stuff */
		FlagUnset(termSettings.c_cflag, PARENB); //Disable parity
		FlagUnset(termSettings.c_cflag, CSTOPB); //1 stop bit
		FlagUnset(termSettings.c_cflag, CRTSCTS); //Disable Flow Control
		FlagUnset(termSettings.c_cflag, CSIZE);
		termSettings.c_cflag |=  CS8; //Set byte size to 8 bits
		FlagSet(termSettings.c_cflag, CREAD); //Enable Reading
		FlagSet(termSettings.c_cflag, CLOCAL); //Ignore control lines
		
		termSettings.c_cc[VMIN]   =  0; // read doesn't block
		termSettings.c_cc[VTIME]  =  0; // 0.5 seconds read timeout
		
		cfmakeraw(&termSettings);
		
		tcflush(fileHandle, TCIFLUSH); //Flush the port
		if (tcsetattr (fileHandle, TCSANOW, &termSettings) != 0)
		{
			printf("tcsetattr call failed: %s\n", GetErrnoName(errno));
			close(fileHandle);
			return result;
		}
		
		u32 comPortNameLength = (u32)strlen(comPortName);
		result.name = PushArray(memArena, char, comPortNameLength+1);
		strncpy(result.name, comPortName, comPortNameLength);
		result.name[comPortNameLength] = '\0';
		result.isOpen = true;
		result.handle = fileHandle;
		printf("Port opened successfully!\n");
	}
	
	return result;
}

// +==============================+
// |       OSX_CloseComPort       |
// +==============================+
CloseComPort_DEFINITION(OSX_CloseComPort)
{
	if (comPortPntr == nullptr) { return; }
	if (comPortPntr->isOpen == false) { return; }
	if (comPortPntr->handle == -1) { return; }
	
	close(comPortPntr->handle);
	
	if (comPortPntr->name != nullptr)
	{
		ArenaPop(memArena, comPortPntr->name);
	}
	
	ClearPointer(comPortPntr);
	comPortPntr->handle = -1;
}

// +==============================+
// |       OSX_ReadComPort        |
// +==============================+
ReadComPort_DEFINITION(OSX_ReadComPort)
{
	Assert(comPortPntr != nullptr);
	Assert(comPortPntr->isOpen);
	Assert(comPortPntr->handle != -1);
	
	// printf("Reading %u bytes from file %d\n", 1, comPortPntr->handle);
	i64 readResult = read(comPortPntr->handle, outputBuffer, outputBufferLength);
	
	if (readResult == -1)
	{
		switch (errno)
		{
			case EAGAIN: return 0;
			default: printf("read call failed: %s\n", GetErrnoName(errno)); return -1;
		};
	}
	
	return readResult;
}

// +==============================+
// |       OSX_WriteComPort       |
// +==============================+
WriteComPort_DEFINITION(OSX_WriteComPort)
{
	Assert(comPortPntr != nullptr);
	Assert(comPortPntr->isOpen);
	Assert(comPortPntr->handle != -1);
	
	i64 writeResult = write(comPortPntr->handle, newChars, numChars);
	
	if (writeResult == -1)
	{
		printf("write call failed %s\n", GetErrnoName(errno));
	}
}
