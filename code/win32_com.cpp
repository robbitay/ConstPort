/*
File:   win32_com.cpp
Author: Taylor Robbins
Date:   06\10\2017
Description: 
	** Holds functions that are useful for managing serial COM ports in windows

#included from win32_main.cpp
*/

GetComPortList_DEFINITION(Win32_GetComPortList)
{
	u32 result = 0;
	
	for (u8 cIndex = 0; cIndex < arrayOutSize && cIndex < NumComPorts; cIndex++)
	{
		ComPortIndex_t comIndex = (ComPortIndex_t)cIndex;
		bool* boolPntr = &arrayOut[cIndex]; 
		
		// Win32_PrintLine("Trying to open %s...", GetComPortName(comIndex));
		
		HANDLE comHandle = CreateFileA(GetComPortName(comIndex), 
			GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		
		if (comHandle != INVALID_HANDLE_VALUE)
		{
			// Win32_PrintLine("%s Exists!", GetComPortName(comIndex));
			*boolPntr = true;
			result++;
			CloseHandle(comHandle);
		}
		else
		{
			*boolPntr = false;
		}
	}
	
	return result;
}

OpenComPort_DEFINITION(Win32_OpenComPort)
{
	ComPort_t result = {};
	result.index = portIndex;
	result.handle = INVALID_HANDLE_VALUE;
	
	HANDLE comHandle = CreateFileA(GetComPortName(portIndex), 
		GENERIC_READ | GENERIC_WRITE, 
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	
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
	
	return result;
}

CloseComPort_DEFINITION(Win32_CloseComPort)
{
	if (comPortPntr == nullptr) return;
	
	if (comPortPntr->isOpen && comPortPntr->handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(comPortPntr->handle);
		comPortPntr->handle = INVALID_HANDLE_VALUE;
		comPortPntr->isOpen = false;
	}
}

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
		Win32_PrintLine("Wrote %d bytes", numBytesWritten);
		return;
	}
	else
	{
		Win32_WriteLine("COM port write failed");
	}
}

