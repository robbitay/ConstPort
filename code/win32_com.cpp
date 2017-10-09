/*
File:   win32_com.cpp
Author: Taylor Robbins
Date:   06\10\2017
Description: 
	** Holds functions that are useful for managing serial COM ports in windows

#included from win32_main.cpp
*/

const char* GetComPortFileName(const char* portName)
{
	Assert(portName != nullptr);
	u32 portNameLength = (u32)strlen(portName);
	
	if (portNameLength == 4)
	{
		if (portName[0] == 'C' && portName[1] == 'O' && portName[2] == 'M' &&
			portName[3] >= '0' && portName[3] <= '9')
		{
			return portName;
		}
		else
		{
			return "Unknown";
		}
	}
	else if (portNameLength == 5)
	{
		if (portName[0] == 'C' && portName[1] == 'O' && portName[2] == 'M' &&
			portName[3] >= '1' && portName[3] <= '9' && portName[4] >= '0' && portName[4] <= '9')
		{
			     if (portName[3] == '1' && portName[4] == '0') { return "\\\\.\\COM10"; }
			else if (portName[3] == '1' && portName[4] == '1') { return "\\\\.\\COM11"; }
			else if (portName[3] == '1' && portName[4] == '2') { return "\\\\.\\COM12"; }
			else if (portName[3] == '1' && portName[4] == '3') { return "\\\\.\\COM13"; }
			else if (portName[3] == '1' && portName[4] == '4') { return "\\\\.\\COM14"; }
			else if (portName[3] == '1' && portName[4] == '5') { return "\\\\.\\COM15"; }
			else if (portName[3] == '1' && portName[4] == '6') { return "\\\\.\\COM16"; }
			else if (portName[3] == '1' && portName[4] == '7') { return "\\\\.\\COM17"; }
			else if (portName[3] == '1' && portName[4] == '8') { return "\\\\.\\COM18"; }
			else if (portName[3] == '1' && portName[4] == '9') { return "\\\\.\\COM19"; }
			else if (portName[3] == '2' && portName[4] == '0') { return "\\\\.\\COM20"; }
			else if (portName[3] == '2' && portName[4] == '1') { return "\\\\.\\COM21"; }
			else if (portName[3] == '2' && portName[4] == '2') { return "\\\\.\\COM22"; }
			else if (portName[3] == '2' && portName[4] == '3') { return "\\\\.\\COM23"; }
			else if (portName[3] == '2' && portName[4] == '4') { return "\\\\.\\COM24"; }
			else
			{
				return "Unknown";
			}
		}
		else
		{
			return "Unknown";
		}
	}
	else
	{
		return "Unknown";
	}
}

// +==============================+
// |     Win32_GetComPortList     |
// +==============================+
// BoundedStrList_t Win32_GetComPortList(MemoryArena_t* memArena)
GetComPortList_DEFINITION(Win32_GetComPortList)
{
	Assert(memArena != nullptr);
	
	BoundedStrList_t result = {};
	BoundedStrListCreate(&result, NumComPorts, MAX_COM_PORT_NAME_LENGTH, memArena);
	
	for (u8 cIndex = 0; cIndex < NumComPorts; cIndex++)
	{
		ComPortIndex_t comIndex = (ComPortIndex_t)cIndex;
		const char* portName = GetComPortReadableName(comIndex);
		
		// Win32_PrintLine("Trying to open %s...", portName);
		
		HANDLE comHandle = CreateFileA(GetComPortFileName(portName), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		
		if (comHandle != INVALID_HANDLE_VALUE)
		{
			// Win32_PrintLine("%s Exists!", portName);
			BoundedStrListAdd(&result, portName);
			CloseHandle(comHandle);
		}
		else
		{
			// Win32_PrintLine("Couldn't open %s", GetComPortReadableName(comIndex));
		}
	}
	
	#if 1
	BoundedStrListAdd(&result, "Test1");
	BoundedStrListAdd(&result, "Test2");
	BoundedStrListAdd(&result, "Test3");
	#endif
	
	return result;
}

// +==============================+
// |      Win32_OpenComPort       |
// +==============================+
// ComPort_t Win32_OpenComPort(MemoryArena_t* memArena, const char* comPortName, ComSettings_t settings)
OpenComPort_DEFINITION(Win32_OpenComPort)
{
	ComPort_t result = {};
	result.settings = settings;
	result.handle = INVALID_HANDLE_VALUE;
	
	HANDLE comHandle = CreateFileA(GetComPortFileName(comPortName), 
		GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, 0, NULL);
	
	if (comHandle == INVALID_HANDLE_VALUE)
	{
		Win32_WriteLine("Could not open port");
		return result;
	}
	
	DCB dcb = {};
	
	if (!GetCommState(comHandle, &dcb))
	{
		Win32_WriteLine("Couldn't get port state.");
		CloseHandle(comHandle);
		return result;
	}
	
	// dcb.DCBlength = sizeof(DCB);
	switch (settings.baudRate)
	{
		case BaudRate_110:    dcb.BaudRate = CBR_110; break;
		case BaudRate_300:    dcb.BaudRate = CBR_300; break;
		case BaudRate_600:    dcb.BaudRate = CBR_600; break;
		case BaudRate_1200:   dcb.BaudRate = CBR_1200; break;
		case BaudRate_2400:   dcb.BaudRate = CBR_2400; break;
		case BaudRate_4800:   dcb.BaudRate = CBR_4800; break;
		case BaudRate_9600:   dcb.BaudRate = CBR_9600; break;
		case BaudRate_14400:  dcb.BaudRate = CBR_14400; break;
		case BaudRate_19200:  dcb.BaudRate = CBR_19200; break;
		case BaudRate_38400:  dcb.BaudRate = CBR_38400; break;
		case BaudRate_57600:  dcb.BaudRate = CBR_57600; break;
		case BaudRate_115200: dcb.BaudRate = CBR_115200; break;
		case BaudRate_128000: dcb.BaudRate = CBR_128000; break;
		case BaudRate_256000: dcb.BaudRate = CBR_256000; break;
		default:              dcb.BaudRate = CBR_9600; break;
	};
	switch (settings.parity)
	{
		case Parity_None:  dcb.Parity = NOPARITY; break;
		case Parity_Even:  dcb.Parity = EVENPARITY; break;
		case Parity_Odd:   dcb.Parity = ODDPARITY; break;
		case Parity_Mark:  dcb.Parity = MARKPARITY; break;
		case Parity_Space: dcb.Parity = SPACEPARITY; break;
		default:           dcb.Parity = NOPARITY; break;
	};
	switch (settings.stopBits)
	{
		case StopBits_1:   dcb.StopBits = ONESTOPBIT; break;
		case StopBits_1_5: dcb.StopBits = ONE5STOPBITS; break;
		case StopBits_2:   dcb.StopBits = TWOSTOPBITS; break;
		default:           dcb.StopBits = ONESTOPBIT; break;
	};
	// dcb.fBinary           = TRUE;
	// dcb.fParity           = parityEnabled ? TRUE : FALSE;
	// dcb.fOutxCtsFlow      = useFlowControl ? TRUE: FALSE;
	// dcb.fOutxDsrFlow      = FALSE; //TODO: DSR is kinda like CTS? What do we do?
	// dcb.fDtrControl       = DTR_CONTROL_DISABLE;
	// dcb.fDsrSensitivity   = FALSE;
	// dcb.fTXContinueOnXoff = FALSE; //Has to do with the buffer getting full
	// dcb.fOutX             = FALSE; //TODO: Flow control using characters?
	// dcb.fInX              = FALSE; // "      "      "       "
	// dcb.fErrorChar        = TRUE;
	// dcb.fNull             = FALSE //Set to TRUE to discard NULL bytes
	// dcb.fRtsControl       = useFlowControl ? RTS_CONTROL_TOGGLE : RTS_CONTROL_DISABLE;
	// dcb.fAbortOnError     = FALSE; //TODO: Should we use this and make sure we handle errors calling ClearCommError()?
	// dcb.XonLim            = useFlowControl ? 16 : 0; //TODO: Make this an input?
	// dcb.XoffLim           = useFlowControl ? 4 : 0; //TODO: Make this an input?
	dcb.ByteSize          = settings.numBits;
	// dcb.XonChar           = '~'; //TODO: Do we need to set this?
	// dcb.XoffChar          = '~'; //TODO: Do we need to set this?
	// dcb.ErrorChar         = '?';
	// dcb.EofChar           = '~'; //TODO: Do we need to set this?
	// dcb.EvtChar           = '~'; //TODO: Do we need to set this?
	
	if (!SetCommState(comHandle, &dcb))
	{
		Win32_WriteLine("Could not set port settings.");
		CloseHandle(comHandle);
		return result;
	}
	
	COMMTIMEOUTS comTimeouts = {};
	comTimeouts.ReadIntervalTimeout = MAXDWORD;//ms
	
	if (!SetCommTimeouts(comHandle, &comTimeouts))
	{
		Win32_WriteLine("Could not set the COM timeouts.");
		CloseHandle(comHandle);
		return result;
	}
	
	result.handle = comHandle;
	result.settings = settings;
	result.isOpen = true;
	u32 comPortNameLength = (u32)strlen(comPortName);
	result.name = PushArray(memArena, char, comPortNameLength+1);
	strncpy(result.name, comPortName, comPortNameLength);
	result.name[comPortNameLength] = '\0';
	
	return result;
}

// +==============================+
// |      Win32_CloseComPort      |
// +==============================+
CloseComPort_DEFINITION(Win32_CloseComPort)
{
	if (comPortPntr == nullptr) return;
	
	if (comPortPntr->isOpen && comPortPntr->handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(comPortPntr->handle);
		comPortPntr->handle = INVALID_HANDLE_VALUE;
		comPortPntr->isOpen = false;
	}
	
	if (comPortPntr->name != nullptr)
	{
		ArenaPop(memArena, comPortPntr->name);
		comPortPntr->name = nullptr;
	}
}

// +==============================+
// |      Win32_ReadComPort       |
// +==============================+
ReadComPort_DEFINITION(Win32_ReadComPort)
{
	Assert(comPortPntr != nullptr);
	Assert(comPortPntr->isOpen);
	Assert(comPortPntr->handle != INVALID_HANDLE_VALUE);
	Assert(outputBuffer != nullptr);
	Assert(outputBufferLength != 0);
	
	DWORD numBytesTransferred = 0;
	
	bool32 readResult = ReadFile(comPortPntr->handle, outputBuffer, outputBufferLength, &numBytesTransferred, 0);
	if (readResult)
	{
		// Win32_PrintLine("Read Result: %d", readResult);
		return numBytesTransferred;
	}
	else
	{
		return 0;
	}
}

// +==============================+
// |      Win32_WriteComPort      |
// +==============================+
WriteComPort_DEFINITION(Win32_WriteComPort)
{
	Assert(comPortPntr != nullptr);
	Assert(comPortPntr->isOpen);
	Assert(comPortPntr->handle != INVALID_HANDLE_VALUE);
	Assert(newChars != nullptr);
	
	DWORD numBytesWritten;
	
	bool32 writeResult = WriteFile(comPortPntr->handle, newChars, numChars, &numBytesWritten, NULL);
	
	if (writeResult)
	{
		// Win32_PrintLine("Wrote %d bytes", numBytesWritten);
		return;
	}
	else
	{
		Win32_WriteLine("COM port write failed");
	}
}

