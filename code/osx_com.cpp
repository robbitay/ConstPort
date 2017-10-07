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
		//TODO: Check to see if the com port exists
		
		arrayOut[bIndex] = false;
	}
	
	return result;
}

// +==============================+
// |       OSX_OpenComPort        |
// +==============================+
OpenComPort_DEFINITION(OSX_OpenComPort)
{
	ComPort_t result = {};
	
	//TODO: Attempt to open the COM port
	
	return result;
}

// +==============================+
// |       OSX_CloseComPort       |
// +==============================+
CloseComPort_DEFINITION(OSX_CloseComPort)
{
	//TODO: Close the COM port
	
	ClearPointer(comPortPntr);
}

// +==============================+
// |       OSX_ReadComPort        |
// +==============================+
ReadComPort_DEFINITION(OSX_ReadComPort)
{
	//TODO: Read data from the COM port
	
	return 0;
}

// +==============================+
// |       OSX_WriteComPort       |
// +==============================+
WriteComPort_DEFINITION(OSX_WriteComPort)
{
	//TODO: Write to the COM port
}
