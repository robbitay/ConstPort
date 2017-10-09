/*
File:   win32_helpers.cpp
Author: Taylor Robbins
Date:   06\04\2017
Description: 
	** Contains any and all other functions that are helpful for various tasks

#included from win32_main.cpp
*/

FreeFileMemory_DEFINITION(Win32_FreeFileMemory)
{
	if(fileInfo->content != nullptr)
	{
		VirtualFree(fileInfo->content, 0, MEM_RELEASE);
		fileInfo->content = nullptr;
	}
}

ReadEntireFile_DEFINITION(Win32_ReadEntireFile)
{
	FileInfo_t result = {};
	
	HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		if(GetFileSizeEx(fileHandle, &fileSize))
		{
			//TODO: Define and use SafeTruncateUInt64 
			uint32 fileSize32 = (uint32)(fileSize.QuadPart);
			result.content = VirtualAlloc(0, fileSize32+1, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			if(result.content)
			{
				DWORD bytesRead;
				if(ReadFile(fileHandle, result.content, fileSize32, &bytesRead, 0) &&
				   (fileSize32 == bytesRead))
				{
					// NOTE: File read successfully
					result.size = fileSize32;
					((u8*)result.content)[fileSize32] = '\0';
				}
				else
				{
					Win32_FreeFileMemory(&result);
					result.content = 0;
				}
			}
			else
			{
				// TODO: Logging
			}
		}
		else
		{
			// TODO: Logging
		}

		CloseHandle(fileHandle);
	}
	else
	{
		// TODO(casey): Logging
	}

	return result;
}

WriteEntireFile_DEFINITION(Win32_WriteEntireFile)
{
	bool32 result = false;
	
	HANDLE fileHandle = CreateFileA(
		filename,              //Name of the file
		GENERIC_WRITE,         //Open for writing
		0,                     //Do not share
		NULL,                  //Default security
		CREATE_ALWAYS,         //Always overwrite
		FILE_ATTRIBUTE_NORMAL, //Default file attributes
		0                      //No Template File
	);
	if(fileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;
		if (WriteFile(fileHandle, memory, memorySize, &bytesWritten, 0))
		{
			// NOTE: File read successfully
			result = (bytesWritten == memorySize);
		}
		else
		{
			// TODO: Logging
		}

		CloseHandle(fileHandle);
	}
	else
	{
		// TODO: Logging
	}

	return result;
}

OpenFile_DEFINITION(Win32_OpenFile)
{
	Assert(openFileOut != nullptr);
	Assert(fileName != nullptr);
	
	HANDLE fileHandle = CreateFileA(
		fileName,              //Name of the file
		GENERIC_WRITE,         //Open for reading and writing
		FILE_SHARE_READ,       //Do not share
		NULL,                  //Default security
		OPEN_ALWAYS,           //Open existing or create new
		FILE_ATTRIBUTE_NORMAL, //Default file attributes
		NULL                   //No Template File
	);
	
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		Win32_PrintLine("Couldn't open/create file: \"%s\"", fileName);
		return false;
	}
	
	SetFilePointer(fileHandle, 0, NULL, FILE_END);
	
	openFileOut->isOpen = true;
	openFileOut->handle = fileHandle;
	return true;
}

AppendFile_DEFINITION(Win32_AppendFile)
{
	Assert(filePntr != nullptr);
	Assert(filePntr->isOpen);
	Assert(filePntr->handle != INVALID_HANDLE_VALUE);
	
	if (newDataSize == 0) return true;
	Assert(newData != nullptr);
	
	DWORD bytesWritten = 0;
	if (WriteFile(filePntr->handle, newData, newDataSize, &bytesWritten, 0))
	{
		// NOTE: File read successfully
		if (bytesWritten == newDataSize)
		{
			return true;
		}
		else
		{
			Win32_PrintLine("Not all bytes appended to file. %u/%u written", bytesWritten, newDataSize);
			return false;
		}
	}
	else
	{
		Win32_PrintLine("WriteFile failed. %u/%u written", bytesWritten, newDataSize);
		return false;
	}
}

CloseFile_DEFINITION(Win32_CloseFile)
{
	if (filePntr == nullptr) return;
	if (filePntr->handle == INVALID_HANDLE_VALUE) return;
	if (filePntr->isOpen == false) return;
	
	CloseHandle(filePntr->handle);
	filePntr->handle = INVALID_HANDLE_VALUE;
	filePntr->isOpen = false;
}

LaunchFile_DEFINITION(Win32_LaunchFile)
{
	u64 executeResult = (u64)ShellExecute(
		NULL,   //No parent window
		"open", //The action verb
		filename, //The target file
		NULL, //No parameters
		NULL, //Use default working directory
		SW_SHOWNORMAL //Show command is normal
	);
	
	if (executeResult > 32)
	{
		return true;
	}
	else
	{
		switch (executeResult)
		{
			case ERROR_FILE_NOT_FOUND:
			{
				Win32_WriteLine("ShellExecute returned ERROR_FILE_NOT_FOUND");
			} break;
			
			default:
			{
				Win32_PrintLine("ShellExecute failed with result 0x%02X", executeResult);
			} break;
		};
		
		return false;
	}
}

uint32 GetRunningDirectory(char* buffer, uint32 maxBufferSize)
{
	DWORD moduleFilenameLength = GetModuleFileNameA(0, buffer, maxBufferSize);
	char* lastSlash = buffer;
	for(char* scan = buffer; *scan != '\0'; scan++)
	{
		if (*scan == '\\')
		{
			lastSlash = scan + 1;
		}
	}
	memset(lastSlash, '\0', buffer + maxBufferSize - lastSlash);
	
	return (uint32)(lastSlash - buffer);
}

inline FILETIME GetFileWriteTime(const char* filename)
{
	FILETIME lastWriteTime = {};
	
	WIN32_FIND_DATA findData;
	HANDLE fileHandle = FindFirstFileA(filename, &findData);
	
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		lastWriteTime = findData.ftLastWriteTime;
		FindClose(fileHandle);
	}
	
	return lastWriteTime;
}

inline bool CatStrings(const char* sourceA, const char* sourceB, 
	char* destination, uint32 destSize)
{
	uint32 numChars = 0;
	
	for (const char* scan = sourceA; *scan != '\0'; scan++,numChars++)
	{
		if (numChars >= destSize-1)
		{
			return false;
		}
		
		destination[numChars] = *scan;
	}
	
	for (const char* scan = sourceB; *scan != '\0'; scan++,numChars++)
	{
		if (numChars >= destSize-1)
		{
			return false;
		}
		
		destination[numChars] = *scan;
	}
	
	Assert(numChars < destSize);
	destination[numChars] = '\0';
	
	return true;
}

void UpdateWindowTitle(GLFWwindow* window, const char* baseName, Version_t* platformVersion, Version_t* appVersion)
{
	char windowTitle[128] = {};
	
	#if DEBUG
		snprintf(windowTitle, ArrayCount(windowTitle),
			"%s (Platform %u.%u:%03u App %u.%u:%03u)", baseName,
			platformVersion->major, platformVersion->minor, platformVersion->build,
			appVersion->major, appVersion->minor, appVersion->build);
	#else
		snprintf(windowTitle, ArrayCount(windowTitle),
			"%s (v%u.%u)", baseName,
			appVersion->major, appVersion->minor);
	#endif
	
	glfwSetWindowTitle(window, windowTitle);
}