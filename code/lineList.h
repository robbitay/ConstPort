/*
File:   lineList.h
Author: Taylor Robbins
Date:   06\09\2017
*/

#ifndef _LINE_LIST_H
#define _LINE_LIST_H

struct Line_t
{
	LinkHeader_t header;
	
	u32 numChars;
	char* chars;
	
	Color_t color;
};

struct LineList_t
{
	i32 numLines;
	LinkedList_t list;
	MemoryArena_t* arenaPntr;
};

union TextLocation_t
{
	v2i vec;
	struct
	{
		i32 lineNum;
		i32 charIndex;
	};
};

inline TextLocation_t NewTextLocation(i32 lineNum, i32 charIndex)
{
	return { lineNum, charIndex };
}

#endif // _LINE_LIST_H