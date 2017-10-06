/*
File:   win32_helpers.h
Author: Taylor Robbins
Date:   10\05\2017
*/

#ifndef _WIN_32_HELPERS_H
#define _WIN_32_HELPERS_H

struct FileInfo_t
{
	uint32 size;
	void* content;
};

struct OpenFile_t
{
	bool isOpen;
	HANDLE handle;
};

#endif //  _WIN_32_HELPERS_H
