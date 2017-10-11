/*
File:   appLineList.cpp
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
	
	// DEBUG_PrintLine("New Line Added at %08X. Last: %08X  Next: %08X", newLine, newLine->header.lastItem, newLine->header.nextItem);
	// Line_t* lastLine = (Line_t*)newLine->header.lastItem;
	// if (lastLine != nullptr)
	// {
	// 	DEBUG_PrintLine("Last line at %08X. Last: %08X  Next: %08X", lastLine, lastLine->header.lastItem, lastLine->header.nextItem);
	// }
	
	memcpy(newStringBuffer, string, stringLength);
	newStringBuffer[stringLength] = '\0';
	
	newLine->numChars = stringLength;
	newLine->chars = newStringBuffer;
	newLine->animProgress = 0.0f;
	newLine->lineHeight = 0;
	newLine->timestamp = 0;
	newLine->flags = 0;
	newLine->matchColor = GC->colors.foreground;
	newLine->backgroundColor = GC->colors.background;
	
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
	//First Character to come in on this line, record the timestamp
	if (line->numChars == 0)
	{
		line->timestamp = GetTimestamp(platform->localTime);
	}
	line->numChars++;
	line->chars[line->numChars] = '\0';
}

inline void LineReset(LineList_t* lineList, Line_t* line)
{
	Assert(lineList != nullptr);
	Assert(line != nullptr);
	
	ArenaPop(lineList->arenaPntr, line->chars);
	line->numChars = 0;
	char* newLocation = PushArray(lineList->arenaPntr, char, line->numChars+1);
	line->chars = newLocation;
	line->chars[line->numChars] = '\0';
	
	line->animProgress = 0.0f;
	line->lineHeight = 0;
	line->timestamp = 0;
	line->flags = 0;
	line->matchColor = GC->colors.foreground;
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

inline v2 MeasureLine(const Font_t* font, const Line_t* line)
{
	return MeasureString(font, line->chars);
}

inline TextLocation_t TextLocationMin(TextLocation_t location1, TextLocation_t location2)
{
	if (location1.lineNum == location2.lineNum)
	{
		if (location1.charIndex >= location2.charIndex)
		{
			return location2;
		}
		else
		{
			return location1;
		}
	}
	else if (location1.lineNum > location2.lineNum)
	{
		return location2;
	}
	else
	{
		return location1;
	}
}
inline TextLocation_t TextLocationMax(TextLocation_t location1, TextLocation_t location2)
{
	if (location1.lineNum == location2.lineNum)
	{
		if (location1.charIndex >= location2.charIndex)
		{
			return location1;
		}
		else
		{
			return location2;
		}
	}
	else if (location1.lineNum > location2.lineNum)
	{
		return location1;
	}
	else
	{
		return location2;
	}
}