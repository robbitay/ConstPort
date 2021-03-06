/*
File:   appRenderLine.cpp
Author: Taylor Robbins
Date:   07\08\2017
Description: 
	** Handles rendering a line of text on the screen at some position 

#included from app.cpp
*/

v2 RenderLine(Line_t* linePntr, v2 position, r32 viewWidth, bool sizeOnly = false)
{
	UiElements_t* ui = &app->uiElements;
	v2 result = Vec2_Zero;
	v2 relMousePos = RenderMousePos - ui->viewRec.topLeft + ui->scrollOffset;
	linePntr->lineWrapWidth = viewWidth;
	
	Color_t color = linePntr->matchColor;
	Color_t backgroundColor = linePntr->backgroundColor;
	v2 lineStringSize = Vec2_Zero;
	if (GC->lineWrapEnabled)
	{
		lineStringSize = MeasureFormattedString(&app->mainFont, linePntr->chars, viewWidth, GC->lineWrapPreserveWords);
	}
	else
	{
		lineStringSize = MeasureString(&app->mainFont, linePntr->chars);
	}
	if (lineStringSize.y < app->mainFont.lineHeight)
	{
		lineStringSize.y = app->mainFont.lineHeight;
	}
	
	// +====================================+
	// | Draw the Line Background and Text  |
	// +====================================+
	if (!sizeOnly)
	{
		//NOTE: We need to shift the rectangle to be 1 down and 1 shorter so that
		//		the IsInsideRec is never true for more than one line
		rec backgroundRec = NewRec(
			0,
			position.y - app->mainFont.maxExtendUp,
			10000,
			lineStringSize.y + (r32)GC->lineSpacing
		);
		RsDrawRectangle(backgroundRec, backgroundColor);
		
		backgroundRec.y += 1; backgroundRec.height -= 1;
		if (GC->highlightHoverLine && IsInsideRec(backgroundRec, relMousePos) && input->mouseInsideWindow && !ui->mouseInMenu)
		{
			backgroundColor = GC->colors.hoverBackground;
		}
		backgroundRec.y -= 1; backgroundRec.height += 1;
		RsDrawRectangle(backgroundRec, backgroundColor);
		
		if (GC->lineWrapEnabled)
		{
			RsDrawFormattedString(linePntr->chars, linePntr->numChars, position, viewWidth, color, Alignment_Left, GC->lineWrapPreserveWords);
		}
		else
		{
			RsDrawString(linePntr->chars, position, color, 1.0f);
		}
	}
	result = lineStringSize;
	
	// +==============================+
	// |  Measure the Elapsed Banner  |
	// +==============================+
	if (GC->elapsedBannerEnabled)
	{
		Line_t* nextLine = linePntr->next;
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
						r32 bannerHeight = MaxR32(MIN_BANNER_HEIGHT, GC->elapsedBannerHeight * EaseCubicOut(halfAnimProgress));
						result.y += bannerHeight;
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
		result.y += GC->thickMarkHeight;
	}
	
	linePntr->size = result;
	return result;
}

void RenderLineGutter(const Line_t* linePntr, i32 lineIndex, v2 position)
{
	UiElements_t* ui = &app->uiElements;
	i32 lineNumber = lineIndex + app->lineList.firstLineNum;
	
	v2 lineStringSize = Vec2_Zero;
	if (GC->lineWrapEnabled)
	{
		lineStringSize = MeasureFormattedString(&app->mainFont, linePntr->chars, linePntr->lineWrapWidth, GC->lineWrapPreserveWords);
	}
	else
	{
		lineStringSize = MeasureString(&app->mainFont, linePntr->chars);
	}
	if (lineStringSize.y < app->mainFont.lineHeight)
	{
		lineStringSize.y = app->mainFont.lineHeight;
	}
	
	// +==============================+
	// |       Draw Line Number       |
	// +==============================+
	if (GC->showLineNumbers)
	{
		RsPrintString(NewVec2(position.x, position.y), GC->colors.lineNumbers, 1.0f, "%u", lineNumber);
	}
	
	// +==============================+
	// |     Draw Elapsed Banner      |
	// +==============================+
	r32 bannerHeight = 0;
	if (GC->elapsedBannerEnabled)
	{
		Line_t* nextLine = linePntr->next;
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
						rec bannerRec = NewRec(
							ui->viewRec.x + ui->viewRec.width/2 - bannerWidth/2,
							position.y - app->mainFont.maxExtendUp + lineStringSize.y,
							bannerWidth,
							2
						);
						bannerHeight = bannerRec.height;
						RsDrawRectangle(bannerRec, GC->colors.banner1);
					}
					else
					{
						r32 halfAnimProgress = (linePntr->animProgress-0.5f) / 0.5f;
						bannerHeight = MaxR32(MIN_BANNER_HEIGHT, GC->elapsedBannerHeight * EaseCubicOut(halfAnimProgress));
						rec bannerRec = NewRec(
							ui->viewRec.x,
							position.y - app->mainFont.maxExtendUp + lineStringSize.y + GC->lineSpacing/2, 
							ui->viewRec.width,
							bannerHeight
						);
						RsDrawGradient(bannerRec, GC->colors.banner1, GC->colors.banner2, Dir2_Down);
						
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
							
							RsBindFont(&app->uiFont);
							RsDrawString(timespanStr, stringDrawPos, stringColor);
							RsBindFont(&app->mainFont);
						}
					}
				}
			}
		}
	}
	
	// +==============================+
	// |        Draw Line Mark        |
	// +==============================+
	if (IsFlagSet(linePntr->flags, LineFlag_MarkBelow) ||
		(ButtonDown(MouseButton_Left) && IsInsideRec(ui->gutterRec, RenderMouseStartPos) && ui->markIndex != -1 && ui->markIndex == lineIndex))
	{
		if (bannerHeight > 0) bannerHeight += 2;
		rec markRec = NewRec(
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
		if (IsInsideRec(RenderMousePos, ui->viewRec) &&
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
			// IsInsideRec(RenderMouseStartPos, ui->gutterRec) &&
			IsInsideRec(ui->gutterRec, RenderMousePos))
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
		
		RsDrawGradient(markRec, markColor1, markColor2, Dir2_Right);
	}
}

v2 MeasureLines(LineList_t* lineList, r32 viewWidth)
{
	v2 result = Vec2_Zero;
	Line_t* linePntr = lineList->firstLine;
	u32 numCharsMax = 0;
	i32 lineIndex = 0;
	
	while (linePntr != nullptr)
	{
		v2 lineSize = RenderLine(linePntr, Vec2_Zero, viewWidth, true);
		
		r32 beforeHeight = result.y;
		result.y += lineSize.y + GC->lineSpacing;
		
		if (lineSize.x > result.x)
		{
			result.x = lineSize.x;
		}
		
		linePntr = linePntr->next;
		lineIndex++;
	}
	
	return result;
}