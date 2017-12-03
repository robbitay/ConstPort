/*
File:   appCheckbox.cpp
Author: Taylor Robbins
Date:   12\03\2017
Description: 
	** Handles interaction and rendering of checkboxes 
*/

void InitializeCheckbox(Checkbox_t* cb, rec drawRec, MemoryArena_t* memArena, const char* label, Color_t activeColor)
{
	Assert(cb != nullptr);
	ClearPointer(cb);
	
	cb->enabled = true;
	cb->mouseHasLeft = true;
	cb->drawRec = drawRec;
	cb->activeColor = activeColor;
	cb->labelAlignment = Alignment_Right;
	
	if (label != nullptr)
	{
		Assert(memArena != nullptr);
		cb->label = ArenaString(memArena, NtStr(label));
		cb->allocArena = memArena;
	}
}

void CheckboxSet(Checkbox_t* cb, bool checked)
{
	Assert(cb != nullptr);
	
	if (cb->checked != checked)
	{
		cb->checked = checked;
		cb->changeTime = platform->programTime;
		cb->mouseHasLeft = false;
	}
}

void CheckboxToggle(Checkbox_t* cb)
{
	Assert(cb != nullptr);
	
	CheckboxSet(cb, !cb->checked);
}

void CheckboxSetLabel(Checkbox_t* cb, MemoryArena_t* memArena, const char* newLabel)
{
	Assert(cb != nullptr);
	
	if (cb->label != nullptr)
	{
		ArenaPop(cb->allocArena, cb->label);
		cb->label = nullptr;
		cb->allocArena = nullptr;
	}
	
	if (newLabel != nullptr)
	{
		Assert(memArena != nullptr);
		cb->label = ArenaString(memArena, NtStr(newLabel));
		cb->allocArena = memArena;
	}
}

void CheckboxUpdate(Checkbox_t* cb)
{
	Assert(cb != nullptr);
	
	bool mouseInside = IsInsideRec(cb->drawRec, RenderMousePos);
	bool mouseStartedInside = IsInsideRec(cb->drawRec, RenderMouseStartPos);
	
	if (!mouseInside) { cb->mouseHasLeft = true; }
	
	if (cb->enabled)
	{
		if (mouseInside && mouseStartedInside && ButtonReleased(MouseButton_Left))
		{
			CheckboxToggle(cb);
		}
	}
	
}

void CheckboxRender(Checkbox_t* cb, Font_t* labelFont)
{
	Assert(cb != nullptr);
	
	bool mouseInside = IsInsideRec(cb->drawRec, RenderMousePos);
	bool mouseStartedInside = IsInsideRec(cb->drawRec, RenderMouseStartPos);
	
	Assert(platform->programTime >= cb->changeTime);
	u64 timeSinceClick = platform->programTime - cb->changeTime;
	r32 animAmount = ClampR32((r32)timeSinceClick / GC->checkboxAnimTime, 0.0f, 1.0f);
	if (!cb->checked)
	{
		animAmount = Ease(EasingStyle_CubicOut, animAmount);
		animAmount = 1.0f - animAmount;
	}
	else
	{
		animAmount = Ease(EasingStyle_BounceOut, animAmount);
	}
	
	Color_t outlineColor = GC->colors.debugMessage;
	Color_t unselectedColor = GC->colors.debugMessage;
	Color_t selectedColor = cb->activeColor;
	Color_t currentColor = unselectedColor;
	Color_t textColor = unselectedColor;
	if (cb->enabled)
	{
		if (mouseInside)
		{
			outlineColor = cb->activeColor;
			// if (ButtonDown(MouseButton_Left) && IsInsideRec(RenderMouseStartPos, cb->drawRec))
			// {
			// 	outlineColor = GC->colors.buttonPress;
			// }
		}
		currentColor = ColorLerp(unselectedColor, selectedColor, animAmount);
		
		textColor = currentColor;
		if (mouseInside && (cb->mouseHasLeft || cb->checked))
		{
			textColor = cb->activeColor;
		}
		if (cb->mouseHasLeft == false && cb->checked == false)
		{
			outlineColor = currentColor;
		}
		if (animAmount != 1.0f && cb->checked == true)
		{
			outlineColor = cb->activeColor;
		}
	}
	
	if (animAmount > 0)
	{
		RsSetCircleRadius(animAmount*1.1f, 0.0f);
		RsDrawRectangle(cb->drawRec, currentColor);
		RsSetCircleRadius(0.0f, 0.0f);
	}
	if (cb->enabled)
	{
		RsDrawButton(cb->drawRec, {Color_TransparentBlack}, outlineColor, 1);
	}
	else
	{
		RsDrawRectangle(cb->drawRec, outlineColor);
	}
	
	const Font_t* actualFont = renderState->boundFont;
	const Font_t* originalFont = renderState->boundFont;
	if (labelFont != nullptr)
	{
		actualFont = labelFont;
		RsBindFont(labelFont);
	}
	
	if (cb->labelAlignment == Alignment_Right)
	{
		v2 labelPos = NewVec2(
			cb->drawRec.x + cb->drawRec.width + 5,
			cb->drawRec.y + cb->drawRec.height/2 + actualFont->lineHeight/2 - actualFont->maxExtendDown
		);
		RsDrawString(cb->label, labelPos, textColor, 1.0f, Alignment_Left);
	}
	else if (cb->labelAlignment == Alignment_Left)
	{
		v2 labelPos = NewVec2(
			cb->drawRec.x - 5,
			cb->drawRec.y + cb->drawRec.height/2 - actualFont->lineHeight/2 - actualFont->maxExtendDown
		);
		RsDrawString(cb->label, labelPos, textColor, 1.0f, Alignment_Right);
	}
	else if (cb->labelAlignment == Alignment_Center)
	{
		v2 labelPos = NewVec2(
			cb->drawRec.x + cb->drawRec.width/2,
			cb->drawRec.y + cb->drawRec.height + 5 + actualFont->maxExtendUp
		);
		RsDrawString(cb->label, labelPos, textColor, 1.0f, Alignment_Center);
		
	}
	
	RsBindFont(originalFont);
}
