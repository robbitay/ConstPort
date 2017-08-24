/*
File:   appRegularExpressions.cpp
Author: Taylor Robbins
Date:   08\13\2017
Description: 
	** Handles loading and parsing the regular expressions file 

#included from app.cpp
*/

#include <boost/regex.hpp>

//NOTE: I guess since exceptions are disabled in Boost.Regex
//		it looks for a user defined throw_exception function...
namespace boost
{
	void throw_exception(std::exception const & exc)
	{
		DEBUG_WriteLine("WARNING: Exception thrown!");
	}
}

// +----------------------------------------------------------------+
// |                        Public Functions                        |
// +----------------------------------------------------------------+
void InitializeRegexList(RegexList_t* regexList, MemoryArena_t* memArena)
{
	Assert(regexList != nullptr);
	Assert(memArena != nullptr);
	ClearPointer(regexList);
	
	regexList->memArena = memArena;
	
	CreateLinkedList(&regexList->list);
}

Regex_t* AddRegex(RegexList_t* regexList,
	const char* namePntr, u32 nameLength,
	const char* expressionPntr, u32 expressionLength)
{
	Assert(regexList != nullptr);
	Assert(regexList->memArena != nullptr);
	Assert(namePntr != nullptr);
	Assert(expressionPntr != nullptr);
	
	Regex_t* newRegex = LinkedPushStruct(&regexList->list, regexList->memArena, Regex_t);
	ClearPointer(newRegex);
	
	char* nameSpacePntr = (char*)ArenaPush_(regexList->memArena, sizeof(char) * (nameLength+1));
	memcpy(nameSpacePntr, namePntr, nameLength);
	nameSpacePntr[nameLength] = '\0';
	
	char* expressionSpacePntr = (char*)ArenaPush_(regexList->memArena, sizeof(char) * (expressionLength+1));
	memcpy(expressionSpacePntr, expressionPntr, expressionLength);
	expressionSpacePntr[expressionLength] = '\0';
	
	newRegex->name = nameSpacePntr;
	newRegex->expression = expressionSpacePntr;
	
	return newRegex;
}

bool SplitFileLine(const char* linePntr, u32 lineLength, const char** namePntrOut, u32* nameLengthOut, const char** expressionPntrOut, u32* expressionLengthOut)
{
	bool foundFirstChar = false;
	bool foundColon = false;
	bool foundFirstExpChar = false;
	u32 nameStart = 0;
	u32 expressionStart = 0;
	
	for (u32 cIndex = 0; cIndex < lineLength; cIndex++)
	{
		char c = linePntr[cIndex];
		
		if (foundFirstChar == false)
		{
			if (c != ' ' && c != '\t')
			{
				Assert(c != ':');
				
				foundFirstChar = true;
				*namePntrOut = &linePntr[cIndex];
				nameStart = cIndex;
			}
		}
		else if (foundColon == false)
		{
			if (c == ':')
			{
				foundColon = true;
				*nameLengthOut = cIndex-nameStart;
			}
		}
		else
		{
			if (c != ' ' && c != '\t')
			{
				foundFirstExpChar = true;
				*expressionPntrOut = &linePntr[cIndex];
				expressionStart = cIndex;
				break;
			}
		}
	}
	
	if (foundFirstChar == false)
	{
		DEBUG_WriteLine("Empty line, could not split");
		return false;
	}
	else if (foundColon == false)
	{
		DEBUG_PrintLine("No colon on line: \"%.*s\"", lineLength, linePntr);
		return false;
	}
	else if (foundFirstExpChar == false)
	{
		DEBUG_PrintLine("No expression on line: \"%.*s\"", lineLength, linePntr);
		return false;
	}
	
	*expressionLengthOut = lineLength - expressionStart;
	return true;
}

void LoadRegexFile(RegexList_t* regexList, const char* filename)
{
	const PlatformInfo_t* PlatformInfo = Gl_PlatformInfo;
	
	FileInfo_t expressionFile = PlatformInfo->ReadEntireFilePntr(filename);
	
	const char* fileContents = (const char*)expressionFile.content;
	u32 lineStart = 0;
	bool lineEmpty = true;
	bool isComment = false;
	u32 lineNum = 0;
	for (u32 cIndex = 0; cIndex <= expressionFile.size; cIndex++)
	{
		char c = '\0';
		if (cIndex < expressionFile.size) c = fileContents[cIndex];
		char last = '\0';
		if (cIndex > 0) last = fileContents[cIndex-1];
		char next = '\0';
		if (cIndex < expressionFile.size-1) next = fileContents[cIndex+1];
		
		if (cIndex == expressionFile.size || c == '\r' || c == '\n')
		{
			if (!(c == '\n' && last == '\r'))
			{
				if (lineEmpty)
				{
					DEBUG_PrintLine("Line %u is empty", lineNum);
				}
				else if (isComment)
				{
					DEBUG_PrintLine("Line %u is comment", lineNum);
				}
				else
				{
					const char* linePntr = &fileContents[lineStart];
					u32 lineLength = cIndex-lineStart;
					
					const char* namePntr = nullptr; u32 nameLength = 0;
					const char* expressionPntr = nullptr; u32 expressionLength = 0;
					
					DEBUG_PrintLine("Splitting line %u...", lineNum);
					if (SplitFileLine(linePntr, lineLength,
						&namePntr, &nameLength, &expressionPntr, &expressionLength))
					{
						DEBUG_PrintLine("\"%.*s\" = %.*s", nameLength, namePntr, expressionLength, expressionPntr);
					}
					else
					{
						StatusError("Could not parse line %u in %s", lineNum, filename);
					}
					
					// DEBUG_PrintLine("Line %u: \"%.*s\"", lineNum, lineLength, linePntr);
				}
			
				lineEmpty = true;
				isComment = false;
				lineNum++;
			}
			
			lineStart = cIndex+1;
		}
		else if (c == '#')
		{
			if (lineEmpty)
			{
				isComment = true;
			}
		}
		else if (c != ' ' && c != '\t')
		{
			lineEmpty = false;
		}
	}
	
	PlatformInfo->FreeFileMemoryPntr(&expressionFile);
}

bool TestRegularExpression(const char* expressionStr, const char* target, u32 targetLength)
{
	bool result = false;
	boost::regex expression(expressionStr);
	boost::cmatch matches;
	
	DEBUG_PrintLine("Trying to use \"%s\" to match target \"%.*s\"", expressionStr, targetLength, target);
	
	if (boost::regex_search(target, target + targetLength, matches, expression))
	{
		DEBUG_PrintLine("Found %d captured regions:", matches.size());
		for (u32 mIndex = 0; mIndex < matches.size(); mIndex++)
		{
			DEBUG_PrintLine("[%u] = \"%s\"", mIndex, matches[mIndex].str().c_str());
		}
	}
	else
	{
		DEBUG_WriteLine("Regular Expression Failed");
	}
	
	return result;
}