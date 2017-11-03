/*
File:   osx_files.cpp
Author: Taylor Robbins
Date:   10\07\2017
Description: 
	** Holds the functions responsible for reading and writing to and from files on the operating system 
*/

#define FreeFileMemory_DEFINITION(functionName)  void functionName(FileInfo_t* fileInfo)

#define ReadEntireFile_DEFINITION(functionName)  FileInfo_t functionName(const char* filename)

#define WriteEntireFile_DEFINITION(functionName) bool32 functionName(const char* filename, void* memory, uint32 memorySize)

#define OpenFile_DEFINITION(functionName) bool32 functionName(const char* fileName, OpenFile_t* openFileOut)

#define AppendFile_DEFINITION(functionName) bool32 functionName(OpenFile_t* filePntr, const void* newData, u32 newDataSize)

#define CloseFile_DEFINITION(functionName) void functionName(OpenFile_t* filePntr)

#define LaunchFile_DEFINITION(functionName) bool32 functionName(const char* filename)


// +==============================+
// |      OSX_FreeFileMemory      |
// +==============================+
//  void OSX_FreeFileMemory(FileInfo_t* fileInfo)
FreeFileMemory_DEFINITION(OSX_FreeFileMemory)
{
	if (fileInfo == nullptr) { return; }
	if (fileInfo->content == nullptr) { return; }
	
	free(fileInfo->content);
	fileInfo->content = nullptr;
	fileInfo->size = 0;
}

// +==============================+
// |      OSX_ReadEntireFile      |
// +==============================+
//  FileInfo_t OSX_ReadEntireFile(const char* filename)
ReadEntireFile_DEFINITION(OSX_ReadEntireFile)
{
	FileInfo_t result = {};
	FILE* fileHandle;
	
	if (filename[0] != '/')
	{
		char absolutePath[256] = {};
		snprintf(absolutePath, sizeof(absolutePath), "%s/%s", WorkingDirectory, filename);
		printf("Attempting to open \"%s\"\n", absolutePath);
		fileHandle = fopen(absolutePath, "rb");
	}
	else
	{
		printf("Attempting to open \"%s\"\n", filename);
		fileHandle = fopen(filename, "rb");
		
	}
	
	
	if (fileHandle != nullptr)
	{
		fseek(fileHandle, 0, SEEK_END);
		result.size = ftell(fileHandle);
		result.content = malloc(result.size+1);
		
		fseek(fileHandle, 0, SEEK_SET);
		fread(result.content, 1, result.size, fileHandle);
		
		((u8*)result.content)[result.size] = '\0';
		
		fclose(fileHandle);
	}
	else
	{
		printf("Failed to open file, errno = %s\n", GetErrnoName(errno));
	}
	
	// printf("ReadEntireFile(\"%s\") result: %u bytes %p\n", filename, result.size, result.content);
	return result;
}

// +==============================+
// |     OSX_WriteEntireFile      |
// +==============================+
// bool32 OSX_WriteEntireFile(const char* filename, void* memory, uint32 memorySize)
WriteEntireFile_DEFINITION(OSX_WriteEntireFile)
{
	FILE* fileHandle;
	
	if (filename[0] != '/')
	{
		char absolutePath[256] = {};
		snprintf(absolutePath, sizeof(absolutePath), "%s/%s", WorkingDirectory, filename);
		printf("Attempting to save to \"%s\"\n", absolutePath);
		fileHandle = fopen(absolutePath, "wb");
	}
	else
	{
		printf("Attempting to save to \"%s\"\n", filename);
		fileHandle = fopen(filename, "wb");
	}
	
	if (fileHandle == nullptr)
	{
		OSX_WriteLine("Failed to open file");
		return false;
	}
	
	size_t writeResult = fwrite(memory, 1, (size_t)memorySize, fileHandle);
	
	fclose(fileHandle);
	
	if (writeResult != memorySize)
	{
		return false;
	}
	else
	{
		return true;
	}
}

// +==============================+
// |         OSX_OpenFile         |
// +==============================+
// bool32 OSX_OpenFile(const char* fileName, OpenFile_t* openFileOut)
OpenFile_DEFINITION(OSX_OpenFile)
{
	ClearPointer(openFileOut);
	
	//TODO: Open the file
	
	return false;
}

// +==============================+
// |        OSX_AppendFile        |
// +==============================+
// bool32 OSX_AppendFile(OpenFile_t* filePntr, const void* newData, u32 newDataSize)
AppendFile_DEFINITION(OSX_AppendFile)
{
	//TODO: Append the file
	
	return false;
}

// +==============================+
// |        OSX_CloseFile         |
// +==============================+
// void OSX_CloseFile(OpenFile_t* filePntr)
CloseFile_DEFINITION(OSX_CloseFile)
{
	//TODO: Close the file handle
	
	ClearPointer(filePntr);
}

// +==============================+
// |        OSX_LaunchFile        |
// +==============================+
// bool32 OSX_LaunchFile(const char* filename)
LaunchFile_DEFINITION(OSX_LaunchFile)
{
	char printBuffer[256];
	if (filename[0] != '/')
	{
		snprintf(printBuffer, sizeof(printBuffer), "open \"%s/%s\"", WorkingDirectory, filename);
	}
	else
	{
		snprintf(printBuffer, sizeof(printBuffer), "open \"%s\"", filename);
	}
	
	pid_t launchPid = popen2(printBuffer, nullptr, nullptr);
	
	if (launchPid > 0)
	{
		return true;
	}
	else
	{
		OSX_PrintLine("popen2 failed, errno = %s", GetErrnoName(errno));
		return false;
	}
}
