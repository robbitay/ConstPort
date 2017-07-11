/*
File:   appRenderLine.cpp
Author: Taylor Robbins
Date:   07\08\2017
Description: 
	** Handles rendering a line of text on the screen at some position 

#included from app.cpp
*/

//Returns the height of the line
r32 RenderLine(const AppInput_t* AppInput, const Line_t* linePntr, v2 position, bool sizeOnly = false)
{
	AppData_t* appData = GL_AppData;
	r32 result = 0;
	
	Color_t color = Color_Foreground;
	if (linePntr->numChars > 0)
	{
		if (linePntr->chars[0] == 0x01)
		{
			color = Color_Highlight1;
		}
		else if (linePntr->chars[0] == 0x02)
		{
			color = Color_Highlight2;
		}
		else if (linePntr->chars[0] == 0x03)
		{
			color = Color_Highlight3;
		}
		else if (linePntr->chars[0] == 0x04)
		{
			color = Color_Highlight4;
		}
		else if (linePntr->chars[0] == 0x05)
		{
			color = Color_Highlight5;
		}
	}
	
	if (!sizeOnly)
	{
		appData->renderState.DrawString(linePntr->chars, position, color, 1.0f);
	}
	result += appData->testFont.lineHeight;
	
	if (IsFlagSet(linePntr->flags, LineFlag_MarkBelow) && IsFlagSet(linePntr->flags, LineFlag_ThickMark))
	{
		result += 5;
	}
	
	return result;
}

void RenderLineNumber(const AppInput_t* AppInput, const Line_t* linePntr, v2 position, i32 lineIndex, r32 lineHeight)
{
	AppData_t* appData = GL_AppData;
	UiElements_t* ui = &appData->uiElements;
	RenderState_t* rs = &appData->renderState;
	
	rs->PrintString(NewVec2(position.x, position.y), {Color_White}, 1.0f, "%u", lineIndex+1);
				
	if (IsFlagSet(linePntr->flags, LineFlag_MarkBelow) ||
		(ButtonDown(MouseButton_Left) && ui->markIndex != -1 && ui->markIndex == lineIndex))
	{
		rec markRec = NewRectangle(
			ui->gutterRec.x, 
			position.y + appData->testFont.maxExtendDown, 
			ui->gutterRec.width + ui->viewRec.width,
			MARK_SIZE
		);
		if (IsFlagSet(linePntr->flags, LineFlag_ThickMark) ||
			(ui->markIndex == lineIndex && ButtonDown(Button_Shift)))
		{
			markRec.y += 1;
			markRec.height = THICK_MARK_SIZE;
		}
		rs->DrawRectangle(markRec, (ui->markIndex == lineIndex) ? Color_Highlight2 : Color_MarkColor);
	}
}

v2 MeasureLines(const AppInput_t* AppInput, LineList_t* lineList, const Font_t* font)
{
	AppData_t* appData = GL_AppData;
	v2 result = Vec2_Zero;
	UiElements_t* ui = &appData->uiElements;
	RenderState_t* rs = &appData->renderState;
	Line_t* linePntr = (Line_t*)lineList->list.firstItem;
	u32 numCharsMax = 0;
	i32 lineIndex = 0;
	i32 firstLineIndex = 0;
	v2 relMousePos = ui->scrollOffset + ui->mousePos - ui->viewRec.topLeft - NewVec2(LINE_SPACING, 0);
	ui->hoverLocation.lineNum = -1;
	ui->firstRenderLine = 0;
	ui->firstRenderLineOffset = 0;
	
	while (linePntr != nullptr)
	{
		if (linePntr->numChars > numCharsMax)
		{
			v2 lineSize = MeasureString(font, linePntr->chars);
			if (result.x < lineSize.x)
			{
				result.x = lineSize.x;
			}
			numCharsMax = linePntr->numChars;
		}
		
		r32 beforeHeight = result.y;
		r32 lineHeight = RenderLine(AppInput, linePntr, Vec2_Zero, true);
		
		result.y += lineHeight;
		result.y += LINE_SPACING;
		
		//Check if we've passed the top of the view
		if (beforeHeight <= ui->scrollOffset.y && result.y > ui->scrollOffset.y)
		{
			ui->firstRenderLine = lineIndex;
			ui->firstRenderLineOffset = ui->scrollOffset.y - beforeHeight;
		}
		//Check if we passed the mouse hover location
		if (beforeHeight <= relMousePos.y && result.y > relMousePos.y)
		{
			ui->hoverLocation.lineNum = lineIndex;
			v2 lineRelMousePos = relMousePos - NewVec2(0, beforeHeight);
			ui->hoverLocation.charIndex = GetStringIndexForLocation(&appData->testFont, linePntr->chars, lineRelMousePos);
		}
		
		linePntr = (Line_t*)linePntr->header.nextItem;
		lineIndex++;
	}
	
	// if (result.y <= ui->scrollOffset.y)
	// {
	// 	ui->firstRenderLine = max(0, lineIndex-1);
	// 	ui->firstRenderLineOffset = ui->scrollOffset.y - result.y;
	// }
	if (ui->hoverLocation.lineNum == -1)
	{
		ui->hoverLocation.lineNum = max(0, lineList->numLines-1);
		ui->hoverLocation.charIndex = ((Line_t*)lineList->list.lastItem)->numChars;
	}
	
	return result;
}
