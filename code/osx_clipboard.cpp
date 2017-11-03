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
CopyToClipboard_DEFINITION(OSX_CopyToClipboard)
{
	//TODO: Copy to the clipboard
}

// +==============================+
// |    OSX_CopyFromClipboard     |
// +==============================+
CopyFromClipboard_DEFINITION(OSX_CopyFromClipboard)
{
	*dataLengthOut = 0;
	
	//TODO: Copy contents from the clipboard
	
	return nullptr;
}
