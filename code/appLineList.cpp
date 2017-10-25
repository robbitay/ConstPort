/*
File:   appLineList.cpp
Author: Taylor Robbins
Date:   06\09\2017
Description: 
	** Handles chunking string data into lines that
	** have useful functions associated with them 
*/

void InitializeLine(Line_t* linePntr)
{
	ClearPointer(linePntr);
	linePntr->matchColor = GC->colors.textDefault;
	linePntr->backgroundColor = GC->colors.textBackground;
}

void InitializeLineList(LineList_t* lineList, char* charStorageBase, u32 charStorageSize)
{
	Assert(lineList != nullptr);
	Assert(charStorageBase != nullptr);
	Assert(charStorageSize > 0);
	
	ClearPointer(lineList);
	
	lineList->charDataBase = charStorageBase;
	lineList->charDataSize = 0;
	lineList->charDataMaxSize = charStorageSize;
	
	Line_t* firstLine = PushStruct(&app->mainHeap, Line_t);
	InitializeLine(firstLine);
	firstLine->chars = lineList->charDataBase + lineList->charDataSize;
	firstLine->chars[0] = '\0';
	
	lineList->firstLine = firstLine;
	lineList->lastLine = firstLine;
	lineList->numLines = 1;
	lineList->firstLineNum = 1;
}

void DestroyLineList(LineList_t* lineList)
{
	Assert(lineList != nullptr);
	if (lineList->numLines == 0) { Assert(lineList->firstLine == nullptr); return; } //already deallocated
	
	Line_t* linePntr = lineList->firstLine;
	for (i32 lIndex = 0; lIndex < lineList->numLines; lIndex++)
	{
		Assert(linePntr != nullptr);
		
		Line_t* nextLine = linePntr->next;
		ArenaPop(&app->mainHeap, linePntr);
		linePntr = nextLine;
	}
	
	ClearPointer(lineList);
}

Line_t* LineListGetItemAt(LineList_t* lineList, i32 lineIndex)
{
	Assert(lineList != nullptr);
	Assert(lineList->numLines > lineIndex);
	Assert(lineIndex >= 0);
	
	if (lineIndex < 0 || lineIndex >= lineList->numLines) { return nullptr; }
	
	Line_t* linePntr = lineList->firstLine;
	for (i32 lIndex = 0; lIndex < lineIndex; lIndex++)
	{
		Assert(linePntr != nullptr);
		
		linePntr = linePntr->next;
	}
	
	return linePntr;
}

void LineListAppendData(LineList_t* lineList, const char* newChars, u32 numChars)
{
	Assert(lineList != nullptr);
	Assert(lineList->lastLine != nullptr);
	Assert(lineList->charDataBase != nullptr);
	Assert(lineList->charDataSize + numChars < lineList->charDataMaxSize);
	if (newChars == nullptr || numChars == 0) { Assert(false); return; }
	
	Line_t* linePntr = lineList->lastLine;
	Assert(linePntr->chars + linePntr->numChars == lineList->charDataBase + lineList->charDataSize);
	
	memcpy(linePntr->chars + linePntr->numChars, newChars, numChars);
	linePntr->numChars += numChars;
	linePntr->chars[linePntr->numChars] = '\0';
	lineList->charDataSize += numChars;
}

bool LineListPopCharacter(LineList_t* lineList)
{
	Assert(lineList != nullptr);
	Assert(lineList->lastLine != nullptr);
	
	if (lineList->lastLine->numChars > 0)
	{
		lineList->lastLine->numChars--;
		lineList->lastLine->chars[lineList->lastLine->numChars] = '\0';
		lineList->charDataSize--;
		
		return true;
	}
	else
	{
		return false;
	}
}

Line_t* LineListPushLine(LineList_t* lineList)
{
	Assert(lineList != nullptr);
	Assert(lineList->charDataBase != nullptr);
	Assert(lineList->charDataSize + 1 < lineList->charDataMaxSize);
	
	lineList->charDataBase[lineList->charDataSize] = '\0';
	lineList->charDataSize++;
	
	Line_t* newLine = PushStruct(&app->mainHeap, Line_t);
	InitializeLine(newLine);
	newLine->chars = lineList->charDataBase + lineList->charDataSize;
	newLine->chars[0] = '\0';
	
	newLine->previous = lineList->lastLine;
	lineList->lastLine->next = newLine;
	lineList->lastLine = newLine;
	
	lineList->numLines++;
	
	return newLine;
}

void LineListClearLine(LineList_t* lineList)
{
	Assert(lineList != nullptr);
	Assert(lineList->lastLine != nullptr);
	
	Line_t* lastLine = lineList->lastLine;
	Assert(lastLine->chars + lastLine->numChars == lineList->charDataBase + lineList->charDataSize);
	Assert(lineList->charDataSize >= lastLine->numChars);
	
	if (lastLine->numChars == 0) { return; } //Already empty
	
	lineList->charDataSize -= lastLine->numChars;
	lastLine->numChars = 0;
	lastLine->chars[0] = '\0';
}

void LineListReplaceLine(LineList_t* lineList, const char* newChars, u32 numChars)
{
	Assert(lineList != nullptr);
	
	LineListClearLine(lineList);
	LineListAppendData(lineList, newChars, numChars);
}

//Returns the aggregate height of the lines that were removed
r32 LineListDownsize(LineList_t* lineList, u32 targetDataSize, u32* numLinesRemovedOut = nullptr)
{
	Assert(lineList != nullptr);
	Assert(lineList->firstLine != nullptr);
	
	if (numLinesRemovedOut != nullptr) { *numLinesRemovedOut = 0; }
	if (lineList->charDataSize <= targetDataSize) { return 0; }
	
	//Count and remove the lines until we're under targetDataSize
	u32 numLinesRemoved = 0;
	u32 numBytesRemoved = 0;
	r32 removedHeight = 0.0f;
	Line_t* linePntr = lineList->firstLine;
	u32 lIndex = 0;
	while (linePntr != nullptr && lineList->charDataSize - numBytesRemoved > targetDataSize)
	{
		Line_t* nextLine = linePntr->next;
		
		removedHeight += linePntr->lineHeight + GC->lineSpacing; ;

		numBytesRemoved += linePntr->numChars;
		if (nextLine != nullptr) { numBytesRemoved += 1; } //Add one for null-terminator
		Assert(numBytesRemoved <= lineList->charDataSize);

		ArenaPop(&app->mainHeap, linePntr);
		numLinesRemoved++;
		Assert((i32)numLinesRemoved <= lineList->numLines);
		
		linePntr = nextLine;
	}
	
	//Rectify the linkage (firstLine and it's previous pntr)
	if (linePntr == nullptr)
	{
		//We removed all the lines from the list in order to satisfy the criteria
		//we need to add one back on
		linePntr = PushStruct(&app->mainHeap, Line_t);
		InitializeLine(linePntr);
	}
	Assert(linePntr != nullptr);
	lineList->firstLine = linePntr;
	linePntr->previous = nullptr;
	lineList->numLines -= numLinesRemoved;
	lineList->firstLineNum += numLinesRemoved;
	
	//Shift all the charData down into the empty space
	char* charPntr = lineList->charDataBase;
	char* oldDataPntr = lineList->charDataBase + numBytesRemoved;
	for (u32 cIndex = 0; cIndex < lineList->charDataSize - numBytesRemoved; cIndex++)
	{
		Assert(charPntr >= lineList->charDataBase && charPntr < lineList->charDataBase + lineList->charDataMaxSize);
		Assert(oldDataPntr >= lineList->charDataBase && oldDataPntr < lineList->charDataBase + lineList->charDataMaxSize);
		
		*charPntr = *oldDataPntr;
		charPntr++;
		oldDataPntr++;
	}
	lineList->charDataSize -= numBytesRemoved;
	lineList->charDataBase[lineList->charDataSize] = '\0'; //make sure null-terminator is at the end
	
	//Move all the char pointers in the Line_t structures so they point to the shifted data correctly
	linePntr = lineList->firstLine;
	while (linePntr != nullptr)
	{
		linePntr->chars -= numBytesRemoved;
		
		Assert(linePntr->chars >= lineList->charDataBase && linePntr->chars <= lineList->charDataBase + lineList->charDataSize);
		
		linePntr = linePntr->next;
	}
	
	Assert(lineList->charDataSize <= targetDataSize);
	
	if (numLinesRemovedOut != nullptr) { *numLinesRemovedOut = numLinesRemoved; }
	return removedHeight;
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

