/*
File:   appTextBox.cpp
Author: Taylor Robbins
Date:   10\29\2017
Description: 
	** Handles the interaction and rendering with Text Boxes 
*/

u32 FindWordBound(const char* str, u32 strLength, u32 startIndex, bool forward)
{
	Assert(str != nullptr);
	
	u32 result = startIndex;
	if (forward && result < strLength) { result++; }
	if (!forward && result > 0) { result--; }
	
	while (result > 0 && result < strLength)
	{
		char rightChar = str[result];
		char leftChar  = str[result-1];
		
		char prevChar = leftChar;
		char nextChar = rightChar;
		if (!forward) { prevChar = rightChar; nextChar = leftChar; }
		
		if (IsCharClassWord(prevChar) && !IsCharClassWord(nextChar))
		{
			break;
		}
		
		if (forward) { result++; }
		else { result--; }
	}
	
	return result;
}

u32 InsertStringInPlace(char* str, u32 strLength, u32 maxLength, u32 insertIndex, const char* insertStr, u32 insertStrLength)
{
	Assert(str != nullptr);
	Assert(insertStr != nullptr);
	Assert(insertIndex <= strLength);
	
	u32 fromIndex = strLength;
	u32 toIndex = strLength + insertStrLength;
	while (true)
	{
		Assert(toIndex < maxLength);
		
		str[toIndex] = str[fromIndex]; //NOTE: This will also write the \0
		
		if (fromIndex == insertIndex)
		{
			for (u32 cIndex = insertStrLength; cIndex > 0; cIndex--)
			{
				Assert(toIndex > 0);
				str[toIndex-1] = insertStr[cIndex-1];
				toIndex--;
			}
		}
		
		if (toIndex == 0) { break; }
		toIndex--;
		fromIndex--;
	}
	
	Assert(toIndex == 0);
	Assert(fromIndex == 0);
	return strLength + insertStrLength;
}

u32 RemoveStringRegionInPlace(char* str, u32 strLength, u32 regionStartIndex, u32 regionEndIndex)
{
	Assert(str != nullptr);
	Assert(regionStartIndex <= strLength);
	Assert(regionEndIndex <= strLength);
	if (regionStartIndex == regionEndIndex) { return strLength; } //nothing to remove
	
	u32 regionMin = (u32)MinI32(regionStartIndex, regionEndIndex);
	u32 regionMax = (u32)MaxI32(regionStartIndex, regionEndIndex);
	
	u32 fromIndex = 0;
	u32 toIndex = 0;
	while (fromIndex <= strLength)
	{
		if (fromIndex >= regionMin && fromIndex < regionMax)
		{
			//Don't insert the character
		}
		else
		{
			str[toIndex] = str[fromIndex]; //NOTE: This will also write the \0
			toIndex++;
		}
		
		fromIndex++;
	}
	
	return toIndex-1;
}

TextBox_t NewTextBox(rec drawRec, const char* chars, u32 numChars, u32 maxNumChars, MemoryArena_t* arenaPntr, const Font_t* font)
{
	Assert(arenaPntr != nullptr);
	Assert(maxNumChars > 0);
	
	TextBox_t result = {};
	result.drawRec = drawRec;
	result.font = font;
	
	result.leftPadding = 5;
	
	result.colors.background1 = DynamicColor_iha(GC->colors.textInputBox1, GC->colors.textInputBox2, GC->colors.textInputBox2);
	result.colors.background2 = DynamicColor_iha(GC->colors.textInputBox2, GC->colors.textInputBox1, GC->colors.textInputBox1);
	result.colors.text        = DynamicColor(GC->colors.textInputText);
	result.colors.cursor1     = DynamicColor(GC->colors.textInputText);
	result.colors.cursor2     = DynamicColor(ColorTransparent(GC->colors.textInputText, 0.5f));
	result.colors.border      = DynamicColor_ia(NewColor(Color_Black), GC->colors.textInputText);
	result.colors.selection1  = DynamicColor_ia(ColorTransparent(NewColor(0xFFB4B4B4), 0.5f), ColorTransparent(NewColor(Color_Blue), 0.5f));
	result.colors.selection2  = DynamicColor_ia(ColorTransparent(NewColor(0xFFB4B4B4), 0.5f), ColorTransparent(NewColor(Color_Blue), 0.3f));
	
	result.chars = PushArray(arenaPntr, char, maxNumChars);
	result.maxNumChars = maxNumChars;
	
	if (chars != nullptr && numChars > 0)
	{
		Assert(numChars < maxNumChars);
		
		memcpy(result.chars, chars, numChars);
		result.chars[numChars] = '\0';
		result.numChars = numChars;
		
		result.cursorBegin = numChars;
		result.cursorEnd = numChars;
		
		//TODO: Set the drawOffset so we can see the cursor position
	}
	else
	{
		result.numChars = 0;
		result.cursorBegin = 0;
		result.cursorEnd = 0;
	}
	
	return result;
}

void TextBoxRellocate(TextBox_t* tb, rec drawRec)
{
	Assert(tb != nullptr);
	
	tb->drawRec = drawRec;
	
	//TODO: Set the drawOffset so we can still see the cursor
}

void TextBoxClear(TextBox_t* tb)
{
	Assert(tb != nullptr);
	
	tb->numChars = 0;
	tb->chars[0] = '\0';
	tb->cursorEnd = 0;
	tb->cursorBegin = tb->cursorEnd;
}

void TextBoxSet(TextBox_t* tb, const char* newChars, u32 numChars)
{
	Assert(tb != nullptr);
	Assert(newChars != nullptr);
	Assert(numChars < tb->maxNumChars);
	
	memcpy(tb->chars, newChars, numChars);
	tb->chars[numChars] = '\0';
	tb->numChars = numChars;
	tb->cursorEnd = numChars;
	tb->cursorBegin = tb->cursorEnd;
}

void TextBoxUpdate(TextBox_t* tb, bool selected)
{
	Assert(tb != nullptr);
	
	u32 cursorMin = (u32)MinI32(tb->cursorBegin, tb->cursorEnd);
	u32 cursorMax = (u32)MaxI32(tb->cursorBegin, tb->cursorEnd);
	
	if (selected)
	{
		// +==============================+
		// |          Left Arrow          |
		// +==============================+
		if (ButtonPressed(Button_Left))
		{
			if (ButtonDown(Button_Shift))
			{
				if (tb->cursorEnd > 0)
				{
					if (ButtonDown(Button_Control))
					{
						tb->cursorEnd = FindWordBound(tb->chars, tb->numChars, tb->cursorEnd, false);
					}
					else
					{
						tb->cursorEnd--;
					}
				}
			}
			else
			{
				if (tb->cursorBegin != tb->cursorEnd)
				{
					tb->cursorBegin = cursorMin;
					tb->cursorEnd = cursorMin;
				}
				else if (tb->cursorEnd > 0)
				{
					if (ButtonDown(Button_Control))
					{
						tb->cursorEnd = FindWordBound(tb->chars, tb->numChars, tb->cursorEnd, false);
					}
					else
					{
						tb->cursorEnd--;
					}
					tb->cursorBegin = tb->cursorEnd;
				}
			}
		}
		
		// +==============================+
		// |         Right Arrow          |
		// +==============================+
		if (ButtonPressed(Button_Right))
		{
			if (ButtonDown(Button_Shift))
			{
				if (tb->cursorEnd < tb->numChars)
				{
					if (ButtonDown(Button_Control))
					{
						tb->cursorEnd = FindWordBound(tb->chars, tb->numChars, tb->cursorEnd, true);
					}
					else
					{
						tb->cursorEnd++;
					}
				}
			}
			else
			{
				if (tb->cursorBegin != tb->cursorEnd)
				{
					tb->cursorBegin = cursorMax;
					tb->cursorEnd = cursorMax;
				}
				else if (tb->cursorEnd < tb->numChars)
				{
					if (ButtonDown(Button_Control))
					{
						tb->cursorEnd = FindWordBound(tb->chars, tb->numChars, tb->cursorEnd, true);
					}
					else
					{
						tb->cursorEnd++;
					}
					tb->cursorBegin = tb->cursorEnd;
				}
			}
		}
		
		// +==============================+
		// |       Character Input        |
		// +==============================+
		if (input->textInputLength > 0)
		{
			for (u32 cIndex = 0; cIndex < input->textInputLength; cIndex++)
			{
				char newChar = input->textInput[cIndex];
				
				// +==============================+
				// |          Backspace           |
				// +==============================+
				if (newChar == '\b')
				{
					if (tb->cursorBegin == tb->cursorEnd)
					{
						if (tb->cursorEnd > 0)
						{
							u32 wordEndIndex = tb->cursorEnd-1;
							if (ButtonDown(Button_Control))
							{
								wordEndIndex = FindWordBound(tb->chars, tb->numChars, tb->cursorEnd, false);
							}
							
							tb->numChars = RemoveStringRegionInPlace(tb->chars, tb->numChars, tb->cursorEnd, wordEndIndex);
							tb->cursorEnd = wordEndIndex;
							tb->cursorBegin = tb->cursorEnd;
						}
					}
					else
					{
						tb->numChars = RemoveStringRegionInPlace(tb->chars, tb->numChars, tb->cursorBegin, tb->cursorEnd);
						tb->cursorEnd = (u32)MinI32(tb->cursorBegin, tb->cursorEnd);
						tb->cursorBegin = tb->cursorEnd;
					}
				}
				else if (!(newChar == '\n' && IsButtonHandled(Button_Enter))) //all other regular characters
				{
					if (tb->cursorBegin != tb->cursorEnd)
					{
						//Remove the selected text before writing the characters
						
						tb->numChars = RemoveStringRegionInPlace(tb->chars, tb->numChars, tb->cursorBegin, tb->cursorEnd);
						tb->cursorEnd = (u32)MinI32(tb->cursorBegin, tb->cursorEnd);
						tb->cursorBegin = tb->cursorEnd;
					}
					
					if (tb->numChars+1 < tb->maxNumChars)
					{
						tb->numChars = InsertStringInPlace(tb->chars, tb->numChars, tb->maxNumChars, tb->cursorEnd, &newChar, 1);
						tb->cursorEnd++;
						tb->cursorBegin++;
					}
					else
					{
						StatusError("Input text box is full");
					}
				}
			}
		}
		
		// +==============================+
		// |            Delete            |
		// +==============================+
		if (ButtonPressed(Button_Delete))
		{
			if (tb->cursorBegin == tb->cursorEnd)
			{
				if (tb->cursorEnd < tb->numChars)
				{
					u32 wordEndIndex = tb->cursorEnd+1;
					if (ButtonDown(Button_Control))
					{
						wordEndIndex = FindWordBound(tb->chars, tb->numChars, tb->cursorEnd, true);
					}
					
					tb->numChars = RemoveStringRegionInPlace(tb->chars, tb->numChars, tb->cursorEnd, wordEndIndex);
				}
			}
			else
			{
				tb->numChars = RemoveStringRegionInPlace(tb->chars, tb->numChars, tb->cursorBegin, tb->cursorEnd);
				tb->cursorEnd = (u32)MinI32(tb->cursorBegin, tb->cursorEnd);
				tb->cursorBegin = tb->cursorEnd;
			}
		}
		
		// +==============================+
		// |         Home and End         |
		// +==============================+
		if (ButtonPressed(Button_Home))
		{
			if (ButtonDown(Button_Shift))
			{
				tb->cursorEnd = 0;
			}
			else
			{
				tb->cursorEnd = 0;
				tb->cursorBegin = tb->cursorEnd;
			}
		}
		if (ButtonPressed(Button_End))
		{
			if (ButtonDown(Button_Shift))
			{
				tb->cursorEnd = tb->numChars;
			}
			else
			{
				tb->cursorEnd = tb->numChars;
				tb->cursorBegin = tb->cursorEnd;
			}
		}
		
		// +==============================+
		// |       Mouse Selection        |
		// +==============================+
		if (input->mouseInsideWindow && IsInsideRec(tb->drawRec, RenderMouseStartPos))
		{
			v2 relativePos = NewVec2(RenderMousePos.x - (tb->drawRec.x + tb->leftPadding) + tb->drawOffset, 0);
			i32 mouseIndex = GetStringIndexForLocation(tb->font, tb->chars, tb->numChars, relativePos);
			if (mouseIndex < 0) { mouseIndex = 0; }
			if ((u32)mouseIndex > tb->numChars) { mouseIndex = (i32)tb->numChars; }
			
			if (ButtonPressed(MouseButton_Left))
			{
				tb->cursorBegin = (u32)mouseIndex;
			}
			if (ButtonDown(MouseButton_Left))
			{
				tb->cursorEnd = (u32)mouseIndex;
			}
		}
		
		// +==============================+
		// |     Paste From Clipboard     |
		// +==============================+
		if (ButtonDown(Button_Control) && ButtonPressed(Button_V))
		{
			TempPushMark();
			
			u32 clipboardDataSize = 0;
			char* clipbardData = (char*)platform->CopyFromClipboard(TempArena, &clipboardDataSize);
			
			if (clipbardData != nullptr)
			{
				char* sanatized = SanatizeStringAdvanced(clipbardData, clipboardDataSize, TempArena, true, false, true);
				u32 sanatizedLength = (u32)strlen(sanatized);
				
				for (u32 cIndex = 0; cIndex < sanatizedLength; cIndex++)
				{
					char newChar = sanatized[cIndex];
					if (tb->cursorBegin != tb->cursorEnd)
					{
						//Remove the selected text before writing the characters
						
						tb->numChars = RemoveStringRegionInPlace(tb->chars, tb->numChars, tb->cursorBegin, tb->cursorEnd);
						tb->cursorEnd = (u32)MinI32(tb->cursorBegin, tb->cursorEnd);
						tb->cursorBegin = tb->cursorEnd;
					}
					
					if (tb->numChars+1 < tb->maxNumChars)
					{
						tb->numChars = InsertStringInPlace(tb->chars, tb->numChars, tb->maxNumChars, tb->cursorEnd, &newChar, 1);
						tb->cursorEnd++;
						tb->cursorBegin++;
					}
					else
					{
						StatusError("Input text box is full");
					}
				}
			}
			
			TempPopMark();
		}
		
		// +==============================+
		// |      Copy to Clipboard       |
		// +==============================+
		if (ButtonDown(Button_Control) && ButtonPressed(Button_C))
		{
			if (tb->cursorBegin != tb->cursorEnd)
			{
				u32 selectionLength = cursorMax-cursorMin;
				char* tempString = TempString(selectionLength+1);
				memcpy(tempString, &tb->chars[cursorMin], selectionLength);
				tempString[selectionLength] = '\0';
				platform->CopyToClipboard(tempString, selectionLength+1);
				StatusSuccess("Copied %u bytes to clipboard", selectionLength);
			}
		}
	}
	
	// +==============================+
	// |      Update Text Offset      |
	// +==============================+
	{
		r32 viewRecWidth = tb->drawRec.width-2;
		r32 fullStrWidth = MeasureString(tb->font, tb->chars, tb->numChars).width;
		r32 maxOffset = fullStrWidth + viewRecWidth/4 - viewRecWidth;
		r32 cursorOffset = MeasureString(tb->font, tb->chars, tb->cursorEnd).width;
		
		if (fullStrWidth < viewRecWidth)
		{
			tb->drawOffsetGoto = 0;
		}
		
		if (cursorOffset <= tb->drawOffsetGoto)
		{
			tb->drawOffsetGoto -= viewRecWidth/4;
			if (tb->drawOffsetGoto < 0) { tb->drawOffsetGoto = 0; }
		}
		else if (cursorOffset+20 >= tb->drawOffsetGoto+viewRecWidth)
		{
			tb->drawOffsetGoto += viewRecWidth/4;
			if (tb->drawOffsetGoto > maxOffset) { tb->drawOffsetGoto = maxOffset; }
		}
		
		if (AbsR32(tb->drawOffset - tb->drawOffsetGoto) > 1.0f)
		{
			tb->drawOffset += (tb->drawOffsetGoto - tb->drawOffset) / GC->viewSpeedDivider;
		}
		else
		{
			tb->drawOffset = tb->drawOffsetGoto;
		}
	}
}

void TextBoxRender(TextBox_t* tb, bool selected)
{
	Assert(tb != nullptr);
	
	rec originalViewport = renderState->viewport;
	RsSetViewport(tb->drawRec);
	
	r32 colorLerp = (SinR32((platform->programTime/1000.0f)*6.0f) + 1.0f) / 2.0f;
	ColorState_t colorState = selected ? ColorState_Active : ColorState_Idle;
	if (IsInsideRec(tb->drawRec, RenderMousePos) && input->mouseInsideWindow)
	{
		colorState = selected ? ColorState_ActiveHover : ColorState_Hover;
		if (ButtonDown(MouseButton_Left) && IsInsideRec(tb->drawRec, RenderMouseStartPos))
		{
			colorState = ColorState_Pressed;
		}
	}
	
	u32 cursorMin = (u32)MinI32(tb->cursorBegin, tb->cursorEnd);
	u32 cursorMax = (u32)MaxI32(tb->cursorBegin, tb->cursorEnd);
	
	v2 textSize = MeasureString(tb->font, tb->chars, tb->numChars);
	v2 textPos = tb->drawRec.topLeft + NewVec2(tb->leftPadding, tb->drawRec.height/2.0f - textSize.y/2.0f + tb->font->maxExtendUp);
	textPos.x -= tb->drawOffset;
	textPos.x = (r32)RoundR32(textPos.x);
	textPos.y = (r32)RoundR32(textPos.y);
	
	r32 cursorOffset   = (r32)RoundR32(MeasureString(tb->font, tb->chars, tb->cursorEnd).x);
	r32 selectionStart = MeasureString(tb->font, tb->chars, cursorMin).x;
	r32 selectionWidth = MeasureString(tb->font, &tb->chars[cursorMin], cursorMax-cursorMin).x;
	
	// +==============================+
	// |     Draw the Background      |
	// +==============================+
	RsDrawGradient(tb->drawRec, tb->colors.background1[colorState], tb->colors.background2[colorState], Dir2_Down);
	RsDrawButton(tb->drawRec, {Color_TransparentBlack}, tb->colors.border[colorState], 1);
	
	// +==============================+
	// |      Draw the Selection      |
	// +==============================+
	{
		Rectangle_t selectionRec = NewRec(
			textPos.x + selectionStart,
			textPos.y - tb->font->maxExtendUp,
			selectionWidth, tb->font->lineHeight
		);
		Color_t selectionColor = ColorLerp(tb->colors.selection1[colorState], tb->colors.selection2[colorState], colorLerp);
		RsDrawRectangle(selectionRec, selectionColor);
	}
	
	// +==============================+
	// |        Draw the Text         |
	// +==============================+
	RsBindFont(tb->font);
	RsDrawString(tb->chars, tb->numChars, textPos, tb->colors.text[colorState]);
	RsBindFont(&app->mainFont);
	
	// +==============================+
	// |       Draw the Cursor        |
	// +==============================+
	if (selected)
	{
		Rectangle_t cursorRec = NewRec(
			textPos.x + cursorOffset,
			textPos.y - tb->font->maxExtendUp,
			1, tb->font->lineHeight
		);
		
		Color_t cursorColor = ColorLerp(tb->colors.cursor1[colorState], tb->colors.cursor2[colorState], colorLerp);
		RsDrawRectangle(cursorRec, cursorColor);
	}
	
	RsSetViewport(originalViewport);
}


