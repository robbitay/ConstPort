/*
File:   osx_files.cpp
Author: Taylor Robbins
Date:   10\07\2017
Description: 
	** Holds the functions responsible for reading and writing to and from files on the operating system 
*/

// +==============================+
// |      OSX_FreeFileMemory      |
// +==============================+
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
ReadEntireFile_DEFINITION(OSX_ReadEntireFile)
{
	FileInfo_t result = {};
	
	FILE* fileHandle = fopen(filename, "rb");
	
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
	
	printf("ReadEntireFile(\"%s\") result: %u bytes %p\n", filename, result.size, result.content);
	return result;
}

// +==============================+
// |     OSX_WriteEntireFile      |
// +==============================+
WriteEntireFile_DEFINITION(OSX_WriteEntireFile)
{
	//TODO: Write the file
	
	return false;
}

// +==============================+
// |         OSX_OpenFile         |
// +==============================+
OpenFile_DEFINITION(OSX_OpenFile)
{
	ClearPointer(openFileOut);
	
	//TODO: Open the file
	
	return false;
}

// +==============================+
// |        OSX_AppendFile        |
// +==============================+
AppendFile_DEFINITION(OSX_AppendFile)
{
	//TODO: Append the file
	
	return false;
}

// +==============================+
// |        OSX_CloseFile         |
// +==============================+
CloseFile_DEFINITION(OSX_CloseFile)
{
	//TODO: Close the file handle
	
	ClearPointer(filePntr);
}

// +==============================+
// |        OSX_LaunchFile        |
// +==============================+
LaunchFile_DEFINITION(OSX_LaunchFile)
{
	//TODO: Launch the file
	
	return false;
}
