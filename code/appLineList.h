/*
File:   appLineList.h
Author: Taylor Robbins
Date:   07\22\2017
*/

#ifndef _APP_LINE_LIST_H
#define _APP_LINE_LIST_H

enum
{
	LineFlag_MarkBelow        = 0x01,
	LineFlag_ThickMark        = 0x02,
	LineFlag_HasElapsedBanner = 0x04,
	// LineFlag_Something        = 0x08,
	// LineFlag_Something        = 0x10,
	
};

struct Line_t
{
	Line_t* next;
	Line_t* previous;
	
	u32 numChars;
	char* chars;
	
	r32 animProgress;
	v2  size;
	r32 lineWrapWidth;
	u64 timestamp;
	//Use the LineFlag enumeration
	flags8 flags;
	Color_t matchColor;
	Color_t backgroundColor;
};

struct LineList_t
{
	i32 firstLineNum;
	i32 numLines;
	Line_t* firstLine;
	Line_t* lastLine;
	
	char* charDataBase;
	u32 charDataSize;
	u32 charDataMaxSize;
};

union TextLocation_t
{
	v2i vec;
	struct
	{
		i32 lineIndex;
		i32 charIndex;
	};
};

inline TextLocation_t NewTextLocation(i32 lineIndex, i32 charIndex)
{
	return { lineIndex, charIndex };
}

#endif // _APP_LINE_LIST_H