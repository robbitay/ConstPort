/*
File:   osx_helpers.h
Author: Taylor Robbins
Date:   10\05\2017
*/

#ifndef _OSX_HELPERS_H
#define _OSX_HELPERS_H

struct FileInfo_t
{
	uint32 size;
	void* content;
};

struct OpenFile_t
{
	bool isOpen;
	//TODO: Add a handle?
};


#endif //  _OSX_HELPERS_H
