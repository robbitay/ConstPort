/*
File:   osx_clipboard.cpp
Author: Taylor Robbins
Date:   10\07\2017
Description: 
	** Holds the functions that handle interfacing with the OSX clipboard 

#included from osx_main.cpp
*/

// +==============================+
// |     OSX_CopyToClipboard      |
// +==============================+
// void OSX_CopyToClipboard(const void* dataPntr, u32 dataSize)
CopyToClipboard_DEFINITION(OSX_CopyToClipboard)
{
	char* tempSpace = (char*)malloc(dataSize+1);
	memcpy(tempSpace, dataPntr, dataSize);
	tempSpace[dataSize] = '\0';
	
	glfwSetClipboardString(PlatformInfo.window, tempSpace);
	
	free(tempSpace);
}

// +==============================+
// |    OSX_CopyFromClipboard     |
// +==============================+
// void* OSX_CopyFromClipboard(MemoryArena_t* arenaPntr, u32* dataLengthOut)
CopyFromClipboard_DEFINITION(OSX_CopyFromClipboard)
{
	*dataLengthOut = 0;
	
	const char* contents = glfwGetClipboardString(PlatformInfo.window);
	if (contents == nullptr) { return nullptr; }
	
	*dataLengthOut = (u32)strlen(contents);

	return (void*)contents;
}
