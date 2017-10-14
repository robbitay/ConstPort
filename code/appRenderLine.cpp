/*
File:   appRenderLine.cpp
Author: Taylor Robbins
Date:   07\08\2017
Description: 
	** Handles rendering a line of text on the screen at some position 

#included from app.cpp
*/

//Returns the height of the line
r32 RenderLine(Line_t* linePntr, v2 position, bool sizeOnly = false)
{
	UiElements_t* ui = &app->uiElements;
	r32 result = 0;
	
	Color_t color = linePntr->matchColor;
	Color_t backgroundColor = linePntr->backgroundColor;
	
	if (!sizeOnly)
	{
		//TODO: Draw this non-relative to the camera?
		rec backgroundRec = NewRectangle(
			0,
			position.y - app->mainFont.maxExtendUp - GC->lineSpacing/2,
			10000,
			app->mainFont.lineHeight + GC->lineSpacing
		);
		app->renderState.DrawRectangle(backgroundRec, backgroundColor);
		app->renderState.DrawString(linePntr->chars, position, color, 1.0f);
	}
	result += app->mainFont.lineHeight;
	
	if (GC->elapsedBannerEnabled)
	{
		Line_t* nextLine = (Line_t*)linePntr->header.nextItem;
		if (nextLine != nullptr)
		{
			u64 nextLineTimestamp = nextLine->timestamp;
			if (nextLineTimestamp == 0)
				nextLineTimestamp = GetTimestamp(platform->localTime);
			
			if (nextLineTimestamp > linePntr->timestamp)
			{
				u64 difference = nextLineTimestamp - linePntr->timestamp;
				if (difference > GC->elapsedBannerTime)
				{
					if (linePntr->animProgress < 1.0f)
					{
						 //TODO: Make this time based and finer tuned
						linePntr->animProgress += BANNER_EXPAND_SPEED;
						if (linePntr->animProgress >= 1.0f)
							linePntr->animProgress = 1.0f;
					}
					
					if (linePntr->animProgress < 0.5f)
					{
						r32 halfAnimProgress = linePntr->animProgress / 0.5f;
						r32 bannerWidth = ui->viewRec.width * EaseCubicOut(halfAnimProgress);
						//No line extension during this stage
					}
					else
					{
						r32 halfAnimProgress = (linePntr->animProgress-0.5f) / 0.5f;
						r32 bannerHeight = MaxReal32(MIN_BANNER_HEIGHT, GC->elapsedBannerHeight * EaseCubicOut(halfAnimProgress));
						result += bannerHeight;
					}
				}
				else
				{
					linePntr->animProgress = 0;
				}
			}
		}
	}
	
	if (IsFlagSet(linePntr->flags, LineFlag_MarkBelow) && IsFlagSet(linePntr->flags, LineFlag_ThickMark))
	{
		result += GC->thickMarkHeight;
	}
	
	linePntr->lineHeight = result;
	return result;
}

void RenderLineGutter(const Line_t* linePntr, v2 position, i32 lineIndex, r32 lineHeight)
{
	UiElements_t* ui = &app->uiElements;
	RenderState_t* rs = &app->renderState;
	
	// +==============================+
	// |       Draw Line Number       |
	// +==============================+
	if (GC->showLineNumbers)
	{
		rs->PrintString(NewVec2(position.x, position.y), GC->colors.lineNumbers, 1.0f, "%u", lineIndex+1);
	}
	
	// +==============================+
	// |     Draw Elapsed Banner      |
	// +==============================+
	r32 bannerHeight = 0;
	if (GC->elapsedBannerEnabled)
	{
		Line_t* nextLine = (Line_t*)linePntr->header.nextItem;
		if (nextLine != nullptr)
		{
			u64 nextLineTimestamp = nextLine->timestamp;
			if (nextLineTimestamp == 0)
				nextLineTimestamp = GetTimestamp(platform->localTime);
			
			if (nextLineTimestamp > linePntr->timestamp)
			{
				u64 difference = nextLineTimestamp - linePntr->timestamp;
				if (linePntr->animProgress > 0)
				{
					if (linePntr->animProgress < 0.5f)
					{
						r32 halfAnimProgress = linePntr->animProgress / 0.5f;
						r32 bannerWidth = ui->viewRec.width * EaseCubicOut(halfAnimProgress);
						rec bannerRec = NewRectangle(
							ui->viewRec.x + ui->viewRec.width/2 - bannerWidth/2,
							position.y + app->mainFont.maxExtendDown,
							bannerWidth,
							2
						);
						bannerHeight = bannerRec.height;
						rs->DrawRectangle(bannerRec, GC->colors.banner1);
					}
					else
					{
						r32 halfAnimProgress = (linePntr->animProgress-0.5f) / 0.5f;
						bannerHeight = MaxReal32(MIN_BANNER_HEIGHT, GC->elapsedBannerHeight * EaseCubicOut(halfAnimProgress));
						rec bannerRec = NewRectangle(
							ui->viewRec.x,
							position.y + app->mainFont.maxExtendDown + GC->lineSpacing/2, 
							ui->viewRec.width,
							bannerHeight
						);
						rs->DrawGradient(bannerRec, GC->colors.banner1, GC->colors.banner2, Direction2D_Down);
						
						if (linePntr->animProgress > 0.8f)
						{
							char* timespanStr = TempPrint("%s Passed", GetElapsedString(difference));
							v2 stringSize = MeasureString(&app->uiFont, timespanStr);
							v2 stringDrawPos = NewVec2(
								bannerRec.x + bannerRec.width/2 - stringSize.x/2,
								bannerRec.y + bannerRec.height/2 - stringSize.y/2 + app->uiFont.maxExtendUp
							);
							r32 stringOpacity = (linePntr->animProgress-0.8f) / 0.2f;
							Color_t stringColor = GC->colors.bannerText;
							stringColor.a = (u8)(stringOpacity*255);
							
							rs->BindFont(&app->uiFont);
							rs->DrawString(timespanStr, stringDrawPos, stringColor);
							rs->BindFont(&app->mainFont);
						}
					}
				}
			}
		}
	}
	
	if (IsFlagSet(linePntr->flags, LineFlag_MarkBelow) ||
		(ButtonDown(MouseButton_Left) && IsInsideRectangle(input->mouseStartPos[MouseButton_Left] / GUI_SCALE, ui->gutterRec) && ui->markIndex != -1 && ui->markIndex == lineIndex))
	{
		if (bannerHeight > 0) bannerHeight += 2;
		rec markRec = NewRectangle(
			ui->gutterRec.x, 
			position.y + app->mainFont.maxExtendDown + bannerHeight, 
			ui->gutterRec.width + ui->viewRec.width,
			(r32)GC->markHeight
		);
		if ((IsFlagSet(linePntr->flags, LineFlag_MarkBelow) && IsFlagSet(linePntr->flags, LineFlag_ThickMark)) ||
			(ui->markIndex == lineIndex && ButtonDown(Button_Shift)))
		{
			markRec.y += 1;
			markRec.height = (r32)GC->thickMarkHeight;
		}
		
		bool drawButtonAbove = false;
		bool drawButtonBelow = false;
		#if 0
		//TODO: Only do this check if this line is within the view bounds
		if (IsInsideRectangle(RenderMousePos, ui->viewRec) &&
			ui->hoverLocation.lineNum >= 0)
		{
			if (ui->hoverLocation.lineNum <= lineIndex)
			{
				drawButtonAbove = true;
				
				const Line_t* tempLinePntr = linePntr;
				for (i32 lIndex = lineIndex; lIndex >= ui->hoverLocation.lineNum && tempLinePntr != nullptr; lIndex--)
				{
					if (lIndex != lineIndex && IsFlagSet(tempLinePntr->flags, LineFlag_MarkBelow))
					{
						//NOTE: Another mark is above us before the hovering position.
						drawButtonAbove = false;
						break;
					}
					tempLinePntr = (const Line_t*)tempLinePntr->header.lastItem;
				}
			}
			else
			{
				drawButtonBelow = true;
				
				const Line_t* tempLinePntr = linePntr;
				for (i32 lIndex = lineIndex; lIndex < ui->hoverLocation.lineNum && tempLinePntr != nullptr; lIndex++)
				{
					if (lIndex != lineIndex && IsFlagSet(tempLinePntr->flags, LineFlag_MarkBelow))
					{
						//NOTE: Another mark is above us before the hovering position.
						drawButtonBelow = false;
						break;
					}
					tempLinePntr = (const Line_t*)tempLinePntr->header.nextItem;
				}
			}
		}
		#endif
		
		Color_t markColor1 = GC->colors.lineMark1;
		Color_t markColor2 = GC->colors.lineMark2;
		if (ui->markIndex == lineIndex &&
			// IsInsideRectangle(input->mouseStartPos[MouseButton_Left] / GUI_SCALE, ui->gutterRec) &&
			IsInsideRectangle(RenderMousePos, ui->gutterRec))
		{
			markColor1 = GC->colors.lineMarkHover;
			markColor2 = GC->colors.lineMarkHover;
		}
		
		if (drawButtonAbove)
		{
			markColor1 = {Color_Red};
			markColor2 = {Color_Blue};
		}
		if (drawButtonBelow)
		{
			markColor1 = {Color_Orange};
			markColor2 = {Color_Yellow};
		}
		
		rs->DrawGradient(markRec, markColor1, markColor2, Direction2D_Right);
	}
}

v2 MeasureLines(LineList_t* lineList, const Font_t* font)
{
	v2 result = Vec2_Zero;
	UiElements_t* ui = &app->uiElements;
	RenderState_t* rs = &app->renderState;
	Line_t* linePntr = (Line_t*)lineList->list.firstItem;
	u32 numCharsMax = 0;
	i32 lineIndex = 0;
	i32 firstLineIndex = 0;
	v2 relMousePos = ui->scrollOffset + RenderMousePos - ui->viewRec.topLeft - NewVec2((r32)GC->lineSpacing, 0);
	ui->hoverLocation.lineNum = -1;
	ui->firstRenderLine = 0;
	ui->firstRenderLineOffset = 0;
	ui->markIndex = -1;
	
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
		r32 lineHeight = RenderLine(linePntr, Vec2_Zero, true);
		
		result.y += lineHeight;
		result.y += GC->lineSpacing;
		
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
			ui->hoverMouseLineOffset = relMousePos - NewVec2(0, beforeHeight);
			ui->hoverLocation.charIndex = GetStringIndexForLocation(font, linePntr->chars, ui->hoverMouseLineOffset);
			ui->markIndex = lineIndex;
			if (ui->markIndex > 0 && ui->hoverMouseLineOffset.y < (lineHeight+GC->lineSpacing) / 2.0f)
			{
				ui->markIndex--;
			}
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
