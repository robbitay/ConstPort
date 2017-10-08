/*
File:   osx_com.cpp
Author: Taylor Robbins
Date:   10\07\2017
Description: 
	** Holds the functions that handle interfacing with OSX enumerated COM ports 
*/

// +==============================+
// |      OSX_GetComPortList      |
// +==============================+
GetComPortList_DEFINITION(OSX_GetComPortList)
{
	u32 result = 0;
	
	for (u32 bIndex = 0; bIndex < arrayOutSize; bIndex++)
	{
		bool comExists = false;
		
		if (bIndex == 0 || bIndex == 1)
		{
			comExists = true;
			result++;
		}
		
		arrayOut[bIndex] = comExists;
	}
	
	return result;
}

// +==============================+
// |       OSX_OpenComPort        |
// +==============================+
OpenComPort_DEFINITION(OSX_OpenComPort)
{
	ComPort_t result = {};
	result.handle = -1;
	result.index = portIndex;
	
	const char* portFileName = GetComPortFileName(portIndex);
	// char portFilePath[256] = {};
	// snprintf(portFilePath, sizeof(portFilePath), "/dev/%s", portFileName);
	// printf("ComPortPath: \"%s\"\n", portFilePath);
	
	printf("Attempting to open \"%s\"\n", portFileName);
	int fileHandle = open(portFileName, O_RDWR | O_NDELAY | O_NOCTTY);
	printf("open result: %d\n", fileHandle);
	
	if (fileHandle != -1)
	{
		fcntl(fileHandle, F_SETFL, FNDELAY);
		
		/*
		struct termios {
			tcflag_t	c_iflag;	//input flags
			tcflag_t	c_oflag;	//output flags
			tcflag_t	c_cflag;	//control flags
			tcflag_t	c_lflag;	//local flags
			cc_t		c_cc[NCCS];	//control chars
			speed_t		c_ispeed;	//input speed
			speed_t		c_ospeed;	//output speed
		};
		*/
		#if 1
		termios termSettings = {};
		termios oldTermSettings = {};
		
		printf("Getting port settings...\n");
		if (tcgetattr(fileHandle, &oldTermSettings) != 0)
		{
			printf("tcgetattr returned an error: %d!\n", errno);
		}
		termSettings = oldTermSettings;
		
		printf("termSettings.c_iflag  = %u\n", (u32)termSettings.c_iflag);
		printf("termSettings.c_oflag  = %u\n", (u32)termSettings.c_oflag);
		printf("termSettings.c_cflag  = %u\n", (u32)termSettings.c_cflag);
		printf("termSettings.c_lflag  = %u\n", (u32)termSettings.c_lflag);
		for (u32 cIndex = 0; cIndex < NCCS; cIndex++)
		{
			printf("termSettings.c_cc[%u]  = %02X\n", cIndex, termSettings.c_cc[cIndex]);
		}
		printf("termSettings.c_ispeed = %u\n", (u32)termSettings.c_ispeed);
		printf("termSettings.c_ospeed = %u\n", (u32)termSettings.c_ospeed);
		
		cfsetospeed(&termSettings, B115200);
		cfsetispeed(&termSettings, B115200);
		
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
		
		printf("Flushing the port\n");
		tcflush(fileHandle, TCIFLUSH); //Flush the port
		printf("Setting the port settings\n");
		if (tcsetattr (fileHandle, TCSANOW, &termSettings) != 0)
		{
			printf("tcsetattr returned an error: %d\n", errno);
		}
		#endif
		
		result.isOpen = true;
		result.settings = settings;
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
	i64 readResult = read(comPortPntr->handle, outputBuffer, 1);
	
	if (readResult == -1)
	{
		switch (errno)
		{
			case EAGAIN: return 0;
			default: printf("read returned %lld\n", readResult); return readResult;
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
		printf("write call failed %u\n", errno);
	}
}
