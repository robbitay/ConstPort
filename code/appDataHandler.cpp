/*
File:   appDataHandler.cpp
Author: Taylor Robbins
Date:   02\28\2018
Description: 
	** Holds functions that process and handle data coming from various
	** sources and route them to be saved or sent other places 
*/

void SaveSelectionToFile(TextLocation_t start, TextLocation_t end, bool openFile = false);

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
u32 SanatizeString(const char* charPntr, u32 numChars, char* outputBuffer = nullptr)
{
	u32 result = 0;
	
	char* outPntr = outputBuffer;
	for (u32 cIndex = 0; cIndex < numChars; cIndex++)
	{
		if (charPntr[cIndex] < ' ') 
		{
			//Ignore these characters
		}
		else
		{
			result++;
			if (outputBuffer != nullptr)
			{
				*outPntr = charPntr[cIndex];
				outPntr++;
			}
		}
	}
	
	return result;
}

char* SanatizeStringAdvanced(const char* strPntr, u32 numChars, MemoryArena_t* arenaPntr,
	bool keepNewLines, bool keepBackspaces, bool convertInvalidToHex)
{
	char* result = nullptr;
	u32 resultLength = 0;
	
	for (u8 round = 0; round < 2; round++)
	{
		Assert(round == 0 || result != nullptr);
		
		//Run this code twice, once to measure and once to actually copy into buffer
		u32 fromIndex = 0;
		u32 toIndex = 0;
		while (fromIndex < numChars)
		{
			char c = strPntr[fromIndex];
			char lastChar = '\0';
			if (fromIndex > 0) { lastChar = strPntr[fromIndex-1]; }
			char nextChar = strPntr[fromIndex+1];
			
			bool charHandled = false;
			if (c == '\n')
			{
				if (keepNewLines)
				{
					if (result != nullptr) { result[toIndex] = '\n'; }
					toIndex++;
					charHandled = true;
				}
			}
			else if (c == '\r')
			{
				if (nextChar == '\n' || lastChar == '\n')
				{
					//Drop the \r character
					charHandled = true;
				}
				else if (keepNewLines)
				{
					if (result != nullptr) { result[toIndex] = '\n'; }
					toIndex++;
					charHandled = true;
				}
			}
			else if (c == '\b')
			{
				if (keepBackspaces)
				{
					if (result != nullptr) { result[toIndex] = '\b'; }
					toIndex++;
					charHandled = true;
				}
			}
			else if (IsCharClassPrintable(c) || c == '\t')
			{
				if (result != nullptr) { result[toIndex] = c; }
				toIndex++;
				charHandled = true;
			}
			
			if (charHandled == false && convertInvalidToHex)
			{
				if (result != nullptr)
				{
					result[toIndex+0] = '[';
					result[toIndex+1] = UpperHexChar(c);
					result[toIndex+2] = LowerHexChar(c);
					result[toIndex+3] = ']';
				}
				toIndex += 4;
			}
			
			fromIndex++;
		}
		
		resultLength = toIndex;
		if (resultLength == 0) { break; }
		if (round == 0)
		{
			result = (char*)ArenaPush_(arenaPntr, resultLength+1);
		}
		result[resultLength] = '\0';
	}
	
	return result;
}

void ReplaceLineWithCapture(Line_t* linePntr, const char* regexStr)
{
	TempPushMark();
	
	char* tempString = GetRegexCaptureString(regexStr, linePntr->chars, linePntr->numChars, TempArena);
	DEBUG_PrintLine("Captured replacement: \"%s\"", tempString);
	LineListReplaceLine(&app->lineList, tempString, (u32)strlen(tempString));
	
	TempPopMark();
}

void ReplaceLineWithCaptureFormatString(Line_t* linePntr, const char* regexStr, const char* formatStr)
{
	TempPushMark();
	
	char* tempString = GetRegexCaptureFormatString(regexStr, linePntr->chars, linePntr->numChars, formatStr, TempArena);
	// DEBUG_PrintLine("Captured formatted replacement: \"%s\"", tempString);
	LineListReplaceLine(&app->lineList, tempString, (u32)strlen(tempString));
	
	TempPopMark();
}

void HandleDownsize(bool byLines, u32 target)
{
	if (!byLines && app->lineList.charDataSize <= target) { return; }//No need to downsize
	if (byLines && (u32)app->lineList.numLines <= target) { return; }//No need to downsize
	
	DEBUG_PrintLine("Resizing %u bytes down to %u bytes", app->lineList.charDataSize, target);
	u32 numLinesRemoved = 0;
	r32 heightRemoved = 0;
	if (byLines)
	{
		u32 numLinesToRemove = app->lineList.numLines - target;
		heightRemoved = LineListRemoveTopLines(&app->lineList, numLinesToRemove, &numLinesRemoved);
	}
	else
	{
		heightRemoved = LineListDownsize(&app->lineList, target, &numLinesRemoved);
	}
	app->uiElements.scrollOffset.y = MaxR32(0, app->uiElements.scrollOffset.y - heightRemoved);
	app->uiElements.scrollOffsetGoto.y = MaxR32(0, app->uiElements.scrollOffsetGoto.y - heightRemoved);
	app->selectionStart.lineIndex -= numLinesRemoved;
	if (app->selectionStart.lineIndex < 0)
	{
		app->selectionStart.lineIndex = 0;
		app->selectionStart.charIndex = 0;
	}
	app->selectionEnd.lineIndex -= numLinesRemoved;
	if (app->selectionEnd.lineIndex < 0)
	{
		app->selectionEnd.lineIndex = 0;
		app->selectionEnd.charIndex = 0;
	}
	
	DEBUG_PrintLine("Dropped %u line(s) due to space requirements", numLinesRemoved);
}

// +--------------------------------------------------------------+
// |               Regex Trigger Handling Functions               |
// +--------------------------------------------------------------+

struct TriggerResults_t
{
	bool addLineToBuffer;
	bool createNewLine;
	bool clearScreen;
};

void ApplyTriggerEffects(TriggerResults_t* results, Line_t* newLine, RegexTrigger_t* trigger)
{
	for (u32 eIndex = 0; eIndex < trigger->numEffects; eIndex++)
	{
		const char* effectStr = trigger->effects[eIndex];
		
		if (strcmp(effectStr, "mark") == 0)
		{
			newLine->flags |= LineFlag_MarkBelow;
		}
		else if (strcmp(effectStr, "thick_mark") == 0)
		{
			newLine->flags |= LineFlag_MarkBelow | LineFlag_ThickMark;
		}
		else if (strcmp(effectStr, "mark_above") == 0)
		{
			if (newLine->previous != nullptr)
			{
				FlagSet(newLine->previous->flags, LineFlag_MarkBelow);
			}
		}
		else if (strcmp(effectStr, "thick_mark_above") == 0)
		{
			if (newLine->previous != nullptr)
			{
				FlagSet(newLine->previous->flags, LineFlag_MarkBelow | LineFlag_ThickMark);
			}
		}
		else if (strcmp(effectStr, "clear_screen") == 0)
		{
			if (results->clearScreen == false) { results->clearScreen = true; }
		}
		else if (strcmp(effectStr, "new_line") == 0)
		{
			if (results->createNewLine == false) { results->createNewLine = true; }
		}
		else if (strcmp(effectStr, "clear_line") == 0)
		{
			if (results->addLineToBuffer == true) { results->addLineToBuffer = false; }
		}
		else if (strcmp(effectStr, "count") == 0)
		{
			app->genericCounter++;
		}
		else if (strstr(effectStr, "=") != nullptr)
		{
			char* nameStr = nullptr;
			char* valueStr = nullptr;
			
			bool foundEquals = false;
			u32 valueStartIndex = 0;
			for (u32 cIndex = 0; effectStr[cIndex] != '\0'; cIndex++)
			{
				char c = effectStr[cIndex];
				if (!foundEquals)
				{
					if (IsCharClassWhitespace(c) || c == '=')
					{
						if (nameStr == nullptr)
						{
							nameStr = TempString(cIndex+1);
							memcpy(nameStr, &effectStr[0], cIndex);
							nameStr[cIndex] = '\0';
						}
					}
					if (c == '=') { foundEquals = true; }
				}
				else
				{
					if (IsCharClassWhitespace(c) == false)
					{
						valueStartIndex = cIndex;
						break;
					}
				}
			}
			Assert(foundEquals);
			
			if (valueStartIndex != 0)
			{
				u32 valueStrLength = (u32)strlen(effectStr) - valueStartIndex;
				valueStr = TempString(valueStrLength+1);
				memcpy(valueStr, &effectStr[valueStartIndex], valueStrLength);
				valueStr[valueStrLength] = '\0';
			}
			
			if (nameStr == nullptr || nameStr[0] == '\0')
			{
				DEBUG_PrintLine("No name found in effect: \"%s\"", effectStr);
			}
			else if (valueStr == nullptr || valueStr[0] == '\0')
			{
				DEBUG_PrintLine("No value found in effect: \"%s\"", effectStr);
			}
			else
			{
				if (strcmp(nameStr, "background_color") == 0 ||
					strcmp(nameStr, "foreground_color") == 0)
				{
					Color_t colorValue = {};
					if (TryParseColor(valueStr, (u32)strlen(valueStr), &colorValue) == ConfigError_None)
					{
						if (strcmp(nameStr, "background_color") == 0)
						{
							// DEBUG_PrintLine("Background = %s", valueStr);
							newLine->backgroundColor = colorValue;
						}
						else if (strcmp(nameStr, "foreground_color") == 0)
						{
							// DEBUG_PrintLine("Foreground = %s", valueStr);
							newLine->matchColor = colorValue;
						}
					}
					else
					{
						DEBUG_PrintLine("Could not parse color in effect: %s = \"%s\"", nameStr, valueStr);
					}
				}
				else if (strcmp(nameStr, "status") == 0)
				{
					StatusInfo(valueStr);
				}
				else if (strcmp(nameStr, "status_debug") == 0)
				{
					StatusDebug(valueStr);
				}
				else if (strcmp(nameStr, "status_info") == 0)
				{
					StatusInfo(valueStr);
				}
				else if (strcmp(nameStr, "status_success") == 0)
				{
					StatusSuccess(valueStr);
				}
				else if (strcmp(nameStr, "status_error") == 0)
				{
					StatusError(valueStr);
				}
				else if (strcmp(nameStr, "popup") == 0)
				{
					PopupInfo(valueStr);
				}
				else if (strcmp(nameStr, "popup_debug") == 0)
				{
					PopupDebug(valueStr);
				}
				else if (strcmp(nameStr, "popup_info") == 0)
				{
					PopupInfo(valueStr);
				}
				else if (strcmp(nameStr, "popup_success") == 0)
				{
					PopupSuccess(valueStr);
				}
				else if (strcmp(nameStr, "popup_error") == 0)
				{
					PopupError(valueStr);
				}
				else if (strcmp(nameStr, "save") == 0)
				{
					i32 numLinesOffset = 0;
					if (TryParseI32(valueStr, (u32)strlen(valueStr), &numLinesOffset) && numLinesOffset >= 0)
					{
						i32 lineIndex = app->lineList.numLines - numLinesOffset;
						if (lineIndex < 0) { lineIndex = 0; }
						Assert(lineIndex < app->lineList.numLines);
						
						SaveSelectionToFile(NewTextLocation(lineIndex, 0), NewTextLocation(app->lineList.numLines-1, app->lineList.lastLine->numChars), DEBUG);
					}
					else
					{
						PopupError("Couldn't parse effect value as integer: \"%s\" in save effect", valueStr);
					}
				}
				else
				{
					DEBUG_PrintLine("Unknown effect in regex trigger: \"%s\"", nameStr);
				}
			}
		}
		else
		{
			DEBUG_PrintLine("Unknown effect in regex trigger: \"%s\"", effectStr);
		}
	}
}

void ApplyRegexTriggers(TriggerResults_t* results, bool perCharacter, Line_t* linePntr)
{
	for (u32 rIndex = 0; rIndex < GC->numTriggers; rIndex++)
	{
		RegexTrigger_t* trigger = &GC->triggers[rIndex];
		if (trigger->runPerCharacter == perCharacter)
		{
			bool appliedToThisComPort = (trigger->numComPorts == 0 || app->comPort.isOpen == false);
			if (appliedToThisComPort == false)
			{
				for (u32 comListIndex = 0; comListIndex < trigger->numComPorts; comListIndex++)
				{
					const char* supportedName = trigger->comPorts[comListIndex];
					if (strcmp(supportedName, app->comPort.name) == 0 ||
						strcmp(supportedName, GetPortUserName(app->comPort.name)) == 0)
					{
						appliedToThisComPort = true;
						break;
					}
				}
			}
			
			if (appliedToThisComPort)
			{
				const char* regexStr = nullptr;
				if (trigger->expression != nullptr)
				{
					regexStr = trigger->expression;
				}
				else if (trigger->expressionName != nullptr)
				{
					regexStr = GetRegularExpression(&app->regexList, trigger->expressionName);
				}
				
				if (regexStr != nullptr)
				{
					bool expressionMatched = TestRegularExpression(regexStr, linePntr->chars, linePntr->numChars);
					if (expressionMatched)
					{
						if (trigger->showOnlyCaptured)
						{
							ReplaceLineWithCapture(linePntr, regexStr);
						}
						if (trigger->replaceStr != nullptr)
						{
							ReplaceLineWithCaptureFormatString(linePntr, regexStr, trigger->replaceStr);
						}
						ApplyTriggerEffects(results, linePntr, trigger);
					}
				}
			}
		}
	}
}


// +--------------------------------------------------------------+
// |                       Public Functions                       |
// +--------------------------------------------------------------+

//NOTE: This function serves as a measuring function AS WELL AS
//		a buffer filling function if not passed nullptr for bufferOutput
u32 GetSelection(TextLocation_t location1, TextLocation_t location2, bool insertTimestamps, char* bufferOutput = nullptr)
{
	TextLocation_t minLocation = TextLocationMin(location1, location2);
	TextLocation_t maxLocation = TextLocationMax(location1, location2);
	
	if (minLocation.lineIndex == maxLocation.lineIndex &&
		minLocation.charIndex == maxLocation.charIndex)
	{
		//No selection made
		return 0;
	}
	
	u8 newLineSize = (platform->platformType == Platform_Windows) ? 2 : 1;
	char newLine[2] = {};
	if (platform->platformType == Platform_Windows)
	{
		newLine[0] = '\r';
		newLine[1] = '\n';
	}
	else
	{
		newLine[0] = '\n';
	}
	u32 bufferLength = 0;
	char* outputPntr = bufferOutput;
	
	if (minLocation.lineIndex == maxLocation.lineIndex)
	{
		Line_t* linePntr = LineListGetItemAt(&app->lineList, minLocation.lineIndex);
		
		if (insertTimestamps)
		{
			TempPushMark();
			u64 lineTimestamp = linePntr->timestamp;
			char* timeString = FormattedTimeStr(lineTimestamp);
			u32 timeStringLength = (u32)strlen(timeString);
			if (bufferOutput != nullptr)
			{
				*outputPntr = '['; outputPntr++;
				memcpy(outputPntr, timeString, timeStringLength);
				outputPntr += timeStringLength;
				*outputPntr = ']'; outputPntr++;
				*outputPntr = ' '; outputPntr++;
			}
			bufferLength += timeStringLength + 3;
			TempPopMark();
		}
		
		bufferLength += maxLocation.charIndex - minLocation.charIndex;
		if (bufferOutput != nullptr)
		{
			memcpy(outputPntr, &linePntr->chars[minLocation.charIndex], bufferLength);
			outputPntr += bufferLength;
		}
	}
	else
	{
		{ //First Line
			Line_t* minLinePntr = LineListGetItemAt(&app->lineList, minLocation.lineIndex);
			
			if (insertTimestamps)
			{
				TempPushMark();
				u64 lineTimestamp = minLinePntr->timestamp;
				char* timeString = FormattedTimeStr(lineTimestamp);
				u32 timeStringLength = (u32)strlen(timeString);
				if (bufferOutput != nullptr)
				{
					*outputPntr = '['; outputPntr++;
					memcpy(outputPntr, timeString, timeStringLength);
					outputPntr += timeStringLength;
					*outputPntr = ']'; outputPntr++;
					*outputPntr = ' '; outputPntr++;
				}
				bufferLength += timeStringLength + 3;
				TempPopMark();
			}
			
			bufferLength += SanatizeString(&minLinePntr->chars[minLocation.charIndex], minLinePntr->numChars - minLocation.charIndex);
			bufferLength += newLineSize;
			if (bufferOutput != nullptr)
			{
				outputPntr += SanatizeString(&minLinePntr->chars[minLocation.charIndex], minLinePntr->numChars - minLocation.charIndex, outputPntr);
				memcpy(outputPntr, newLine, newLineSize);
				outputPntr += newLineSize;
			}
		}
		
		//In Between Lines
		for (i32 lineIndex = minLocation.lineIndex+1; lineIndex < maxLocation.lineIndex && lineIndex < app->lineList.numLines; lineIndex++)
		{
			Line_t* linePntr = LineListGetItemAt(&app->lineList, lineIndex);
			
			if (insertTimestamps)
			{
				TempPushMark();
				u64 lineTimestamp = linePntr->timestamp;
				char* timeString = FormattedTimeStr(lineTimestamp);
				u32 timeStringLength = (u32)strlen(timeString);
				if (bufferOutput != nullptr)
				{
					*outputPntr = '['; outputPntr++;
					memcpy(outputPntr, timeString, timeStringLength);
					outputPntr += timeStringLength;
					*outputPntr = ']'; outputPntr++;
					*outputPntr = ' '; outputPntr++;
				}
				bufferLength += timeStringLength + 3;
				TempPopMark();
			}
			
			bufferLength += SanatizeString(linePntr->chars, linePntr->numChars);
			bufferLength += newLineSize;
			if (bufferOutput != nullptr)
			{
				outputPntr += SanatizeString(linePntr->chars, linePntr->numChars, outputPntr);
				memcpy(outputPntr, newLine, newLineSize);
				outputPntr += newLineSize;
			}
		}
		
		{ //Last Line
			Line_t* maxLinePntr = LineListGetItemAt(&app->lineList, maxLocation.lineIndex);
			
			if (insertTimestamps)
			{
				TempPushMark();
				u64 lineTimestamp = maxLinePntr->timestamp;
				char* timeString = FormattedTimeStr(lineTimestamp);
				u32 timeStringLength = (u32)strlen(timeString);
				if (bufferOutput != nullptr)
				{
					*outputPntr = '['; outputPntr++;
					memcpy(outputPntr, timeString, timeStringLength);
					outputPntr += timeStringLength;
					*outputPntr = ']'; outputPntr++;
					*outputPntr = ' '; outputPntr++;
				}
				bufferLength += timeStringLength + 3;
				TempPopMark();
			}
			
			bufferLength += SanatizeString(maxLinePntr->chars, maxLocation.charIndex);
			if (bufferOutput != nullptr)
			{
				outputPntr += SanatizeString(maxLinePntr->chars, maxLocation.charIndex, outputPntr);
			}
		}
	}
	
	bufferLength += 1; //For null terminator
	if (bufferOutput != nullptr)
	{
		*outputPntr = '\0';
	}
	
	return bufferLength;
}

void SaveSelectionToFile(TextLocation_t start, TextLocation_t end, bool openFile)
{
	char* fileName = TempPrint("ConstPortSave_%02u-%02u-%u_%u-%02u-%02u.txt",
		platform->localTime.year, platform->localTime.month, platform->localTime.day,
		platform->localTime.hour, platform->localTime.minute, platform->localTime.second
	);
	
	u32 selectionSize = GetSelection(start, end, GC->saveTimesToFile, nullptr);
	if (selectionSize > 0)
	{
		TempPushMark();
		
		char* fileBuffer = TempString(selectionSize);
		GetSelection(start, end, GC->saveTimesToFile, fileBuffer);
		
		//NOTE: GetSelection adds a \0 on the end so need to remove it
		DEBUG_PrintLine("Saving %u bytes to %s", selectionSize-1, fileName);
		platform->WriteEntireFile(fileName, fileBuffer, selectionSize-1);
		DEBUG_WriteLine("Done!");
		
		TempPopMark();
		
		if (openFile)
		{
			if (platform->LaunchFile(fileName))
			{
				DEBUG_WriteLine("Opened output file for viewing");
			}
			else
			{
				DEBUG_WriteLine("Could not open output file");
			}
		}
		
		StatusSuccess("Saved to %s", fileName);
	}
}

// +--------------------------------------------------------------+
// |                  Push and Process Functions                  |
// +--------------------------------------------------------------+
//Process functions take input, sanatize them, and then call the
//appropriate Push function based off configuration options and application state

void PushLineListData(const char* strPntr, u32 strLength, bool sanatize)
{
	if (strLength == 0) { return; }
	
	Line_t* lastLine = app->lineList.lastLine;
	
	for (u32 cIndex = 0; cIndex < strLength; cIndex++)
	{
		char newChar = strPntr[cIndex];
		if (newChar == '\n' || newChar == '\r')
		{
			Line_t* finishedLine = lastLine;
			
			if (finishedLine->timestamp == 0)
			{
				finishedLine->timestamp = GetTimestamp(platform->localTime);
			}
			
			if (app->writeToFile)
			{
				//Write the line to the outputFile
				
				char timestampBuffer[256]; ClearArray(timestampBuffer);
				RealTime_t lineTime = RealTimeAt(finishedLine->timestamp);
				u32 timestampLength = snprintf(timestampBuffer, sizeof(timestampBuffer)-1,
					"[%s %02u:%02u:%02u%s (%s %s, %04u)] ",
					GetDayOfWeekStr(GetDayOfWeek(lineTime)),
					Convert24HourTo12Hour(lineTime.hour), lineTime.minute, lineTime.second,
					IsPostMeridian(lineTime.hour) ? "pm" : "am",
					GetMonthStr((Month_t)lineTime.month), GetDayOfMonthString(lineTime.day), lineTime.year);
				
				platform->AppendFile(&app->outputFile, timestampBuffer, timestampLength);
				
				for (u32 cIndex2 = 0; cIndex2 < finishedLine->numChars; cIndex2++)
				{
					if (finishedLine->chars[cIndex2] == 0x01)
					{
						finishedLine->chars[cIndex2] = ' ';
					}
					if (finishedLine->chars[cIndex2] == 0x02)
					{
						finishedLine->chars[cIndex2] = ' ';
					}
					if (finishedLine->chars[cIndex2] == 0x03)
					{
						finishedLine->chars[cIndex2] = ' ';
					}
					if (finishedLine->chars[cIndex2] == 0x04)
					{
						finishedLine->chars[cIndex2] = ' ';
					}
					if (finishedLine->chars[cIndex2] == 0x05)
					{
						finishedLine->chars[cIndex2] = ' ';
					}
				}
				platform->AppendFile(&app->outputFile, finishedLine->chars, finishedLine->numChars);
				platform->AppendFile(&app->outputFile, "\r\n", 2);
				
				LineListClearLine(&app->lineList);
			}
			else //Throw the line on the end of the list
			{
				TriggerResults_t results = {};
				results.addLineToBuffer = true;
				ApplyRegexTriggers(&results, true, finishedLine);
				ApplyRegexTriggers(&results, false, finishedLine);
				
				if (results.clearScreen)
				{
					ClearConsole();
					lastLine = app->lineList.lastLine;
				}
				else
				{
					if (results.addLineToBuffer)
					{
						lastLine = LineListPushLine(&app->lineList);
					}
					else
					{
						LineListClearLine(&app->lineList);
					}
				}
			}
		}
		else if (newChar == '\b')
		{
			bool charRemoved = LineListPopCharacter(&app->lineList);
		}
		else
		{
			//TODO: Change invalid characters to something readable?
			LineListAppendData(&app->lineList, &newChar, 1);
			
			TriggerResults_t results = {};
			results.addLineToBuffer = true;
			ApplyRegexTriggers(&results, true, lastLine);
			if (results.createNewLine) { ApplyRegexTriggers(&results, false, lastLine); }
			
			if (results.clearScreen)
			{
				ClearConsole();
				lastLine = app->lineList.lastLine;
			}
			else if (results.createNewLine)
			{
				if (results.addLineToBuffer)
				{
					lastLine = LineListPushLine(&app->lineList);
				}
				else
				{
					LineListClearLine(&app->lineList);
				}
			}
		}
		
		// +==============================+
		// |        Drop Char Data        |
		// +==============================+
		u32 inputArenaMax        = (u32)(app->inputArenaSize/100.f * 98);
		u32 targetInputArenaSize = (u32)(app->inputArenaSize/100.f * 90);
		if (app->lineList.charDataSize >= inputArenaMax)
		{
			HandleDownsize(false, targetInputArenaSize);
		}
		if (app->lineList.numLines > MAX_SAVED_LINES)
		{
			HandleDownsize(true, MAX_SAVED_LINES);
		}
	}
}

void PushComData(const char* strPntr, u32 strLength, bool sanatize)
{
	if (strLength == 0) { return; }
	if (app->comPort.isOpen == false) { return; }
	
	if (sanatize)
	{
		//TODO: Change line endings into whatever is defined in GlobalConfig
	}
	
	platform->WriteComPort(&app->comPort, strPntr, strLength);
	
	app->txShiftRegister |= 0x80;
}

void PushPythonData(const char* strPntr, u32 strLength, bool sanatize)
{
	if (strLength == 0) { return; }
	if (app->programInstance.isOpen == false) { return; }
	
	if (sanatize)
	{
		//TODO: Change line endings into whatever is defined in GlobalConfig
	}
	
	u32 writeResult = platform->WriteProgramInput(&app->programInstance, strPntr, strLength);
}

//NOTE: This should process clipboard data, keyboard input, and textbox data
void ProcessInputData(const char* inputData, u32 inputDataLength, bool sanatize)
{
	if (inputDataLength == 0) { return; }
	
	if (sanatize)
	{
		//TODO: Sanatize the line endings to '\n'
	}
	
	if (app->comPort.isOpen && !(app->programInstance.isOpen && GC->sendInputToPython && !GC->alsoSendInputToCom))
	{
		PushComData(inputData, inputDataLength, sanatize);
	}
	if (app->programInstance.isOpen && GC->sendInputToPython)
	{
		PushPythonData(inputData, inputDataLength, sanatize);
	}
	if (GC->autoEchoInput)
	{
		PushLineListData(inputData, inputDataLength, sanatize);
	}
}

void ProcessComData(const char* comData, u32 comDataLength, bool sanatize)
{
	if (comDataLength == 0) { return; }
	
	if (sanatize)
	{
		//TODO: Sanatize the line endings to '\n'
	}
	
	bool sendDataToPython = (app->programInstance.isOpen && GC->sendComDataToPython);
	if (sendDataToPython)
	{
		PushPythonData(comData, comDataLength, sanatize);
	}
	if (!sendDataToPython || GC->alsoShowComData)
	{
		PushLineListData(comData, comDataLength, sanatize);
	}
	
	app->rxShiftRegister |= 0x80;
}

void ProcessPythonData(const char* pythonData, u32 pythonDataLength, bool sanatize)
{
	if (pythonDataLength == 0) { return; }
	
	if (sanatize)
	{
		//TODO: Sanatize the line endings to '\n'
	}
	
	if (GC->showPythonOutput)
	{
		PushLineListData(pythonData, pythonDataLength, sanatize);
	}
}
