/*
File:   win32_clipboard.cpp
Author: Taylor Robbins
Date:   06\18\2017
Description: 
	** Holds functions for interacting with the clipboard 
*/


// +==============================+
// |    Win32_CopyToClipboard     |
// +==============================+
//void CopyToClipboard(const void* dataPntr, u32 dataSize)
CopyToClipboard_DEFINITION(Win32_CopyToClipboard)
{
	HWND windowHandle = GetActiveWindow();
	
	HGLOBAL globalCopy = GlobalAlloc(GMEM_MOVEABLE, dataSize); 
	if (globalCopy == nullptr)
	{
		Win32_WriteLine("Couldn't allocate space for clipboard data!");
		return;
	}
	
	u8* lockPntr = (u8*)GlobalLock(globalCopy);
	memcpy(lockPntr, dataPntr, dataSize);
	// ((u8*)globalCopy)[dataSize] = '\0';
	GlobalUnlock(globalCopy);
	
	if (OpenClipboard(windowHandle) == false)
	{
		Win32_WriteLine("Couldn't open clipboard!");
		return;
	}
	if (EmptyClipboard() == false)
	{
		Win32_WriteLine("Could not empty current clipboard contents!");
		CloseClipboard(); return;
	}
	
	SetClipboardData(CF_TEXT, globalCopy);
	
	CloseClipboard();
}

// +==============================+
// |   Win32_CopyFromClipboard    |
// +==============================+
//void* CopyFromClipboard(MemoryArena_t* arenaPntr, u32* dataLengthOut)
CopyFromClipboard_DEFINITION(Win32_CopyFromClipboard)
{
	void* result = nullptr;
	if (dataLengthOut != nullptr) { *dataLengthOut = 0; }
	
	HWND windowHandle = GetActiveWindow();
	
	if (OpenClipboard(windowHandle))
	{
		HANDLE dataHandle = GetClipboardData(CF_TEXT);
		
		if (dataHandle != nullptr)
		{
			char* lockPntr = (char*)GlobalLock(dataHandle);
			u32 dataLength = (u32)strlen(lockPntr);
			result = ArenaPush_(arenaPntr, dataLength+1);
			memcpy(result, lockPntr, dataLength);
			((u8*)result)[dataLength] = '\0';
			GlobalUnlock(dataHandle);
			
			if (dataLengthOut != nullptr) { *dataLengthOut = dataLength; }
		}
		else
		{
			Win32_WriteLine("Clipboard did not contain CF_TEXT data");
		}
		
		CloseClipboard();
	}
	else
	{
		Win32_WriteLine("Couldn't open clipboard");
	}
	
	return result;
}
