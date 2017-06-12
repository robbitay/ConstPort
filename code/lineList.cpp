/*
File:   lineList.cpp
Author: Taylor Robbins
Date:   06\09\2017
Description: 
	** Handles chunking string data into lines that
	** have useful functions associated with them 
*/

Line_t* AddLineToList(LineList_t* lineList, const char* string, u32 stringLength)
{
	Assert(lineList != nullptr);
	Assert(string != nullptr);
	
	Line_t* newLine = LinkedPushStruct(&lineList->list, lineList->arenaPntr, Line_t);
	char* newStringBuffer = PushArray(lineList->arenaPntr, char, stringLength+1);
	
	memcpy(newStringBuffer, string, stringLength);
	newStringBuffer[stringLength] = '\0';
	
	newLine->numChars = stringLength;
	newLine->chars = newStringBuffer;
	
	lineList->numLines++;
	
	return newLine;
}

inline Line_t* AddLineToList(LineList_t* lineList, const char* nullTermString)
{
	Assert(lineList != nullptr);
	Assert(nullTermString != nullptr);
	
	return AddLineToList(lineList, nullTermString, (u32)strlen(nullTermString));
}

inline Line_t* GetLineAt(LineList_t* lineList, i32 index)
{
	Assert(lineList != nullptr);
	
	return (Line_t*)LinkedListGetItem(&lineList->list, index);
}

inline Line_t* GetLastLine(LineList_t* lineList)
{
	Assert(lineList != nullptr);
	
	return (Line_t*)LinkedListGetLastItem(&lineList->list);
}

inline void LineAppend(LineList_t* lineList, Line_t* line, char newCharacter)
{
	Assert(lineList != nullptr);
	Assert(line != nullptr);
	
	char* newLocation = PushArray(lineList->arenaPntr, char, line->numChars+1+1);
	memcpy(newLocation, line->chars, line->numChars+1);
	ArenaPop(lineList->arenaPntr, line->chars);
	line->chars = newLocation;
	line->chars[line->numChars] = newCharacter;
	line->numChars++;
	line->chars[line->numChars] = '\0';
}

void CreateLineList(LineList_t* lineList, MemoryArena_t* arenaPntr, const char* contents)
{
	Assert(lineList != nullptr);
	Assert(arenaPntr != nullptr);
	Assert(contents != nullptr);
	ClearPointer(lineList);
	
	lineList->arenaPntr = arenaPntr;
	CreateLinkedList(&lineList->list);
	
	u32 lastIndex = 0;
	u32 cIndex = 0;
	for (cIndex = 0; contents[cIndex] != '\0'; cIndex++)
	{
		if (contents[cIndex] == '\n')
		{
			if (cIndex > lastIndex)
			{
				AddLineToList(lineList, &contents[lastIndex], cIndex - lastIndex);
			}
			
			lastIndex = cIndex+1;
		}
	}
	
	if (cIndex > lastIndex)
	{
		AddLineToList(lineList, &contents[lastIndex], cIndex - lastIndex);
	}
	
	if (lineList->numLines == 0)
	{
		AddLineToList(lineList, "");
	}
}

void DestroyLineList(LineList_t* lineList)
{
	Assert(lineList != nullptr);
	
	for (i32 lineIndex = 0; lineIndex < lineList->numLines; lineIndex++)
	{
		Line_t* linePntr = (Line_t*)LinkedListGetItem(&lineList->list, lineIndex);
		ArenaPop(lineList->arenaPntr, linePntr->chars);
	}
	
	DeleteLinkedList(&lineList->list, lineList->arenaPntr);
	lineList->numLines = 0;
	lineList->arenaPntr = 0;
}
