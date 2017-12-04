/*
File:   appCombobox.cpp
Author: Taylor Robbins
Date:   12\03\2017
Description: 
	** Handles the interaction and rendering of comboboxes 
#included from app.cpp
*/

void InitializeCombobox(Combobox_t* cb, rec drawRec, MemoryArena_t* memArena, const BoundedStrList_t* options)
{
	Assert(cb != nullptr);
	Assert(memArena != nullptr || options  == nullptr);
	
	ClearPointer(cb);
	
	cb->drawRec = drawRec;
	cb->arrowsRec = drawRec;
	cb->arrowsRec.width = cb->arrowsRec.height;
	cb->arrowsRec.x += drawRec.width - cb->arrowsRec.width;
	cb->font = &app->uiFont;
	
	cb->selectedIndex = -1;
	if (options != nullptr && options->count > 0)
	{
		cb->options = *options;
		BoundedStrListSolidify(&cb->options, nullptr, memArena, false);
		cb->allocArena = memArena;
		cb->selectedIndex = 0;
	}
}

void ComboboxDropOptions(Combobox_t* cb)
{
	Assert(cb != nullptr);
	Assert(cb->allocArena != nullptr);
	
	ArenaPop(cb->allocArena, cb->options.memoryBase);
	ClearStruct(cb->options);
	cb->allocArena = nullptr;
}

void ComboboxSetOptions(Combobox_t* cb, MemoryArena_t* memArena, const BoundedStrList_t* options)
{
	Assert(cb != nullptr);
	Assert(memArena != nullptr || options  == nullptr);
	
	if (cb->options.memoryBase != nullptr)
	{
		ComboboxDropOptions(cb);
	}
	
	cb->selectedIndex = -1;
	if (options != nullptr && options->count > 0)
	{
		cb->options = *options;
		BoundedStrListSolidify(&cb->options, nullptr, memArena, false);
		cb->allocArena = memArena;
	}
}

void ComboboxRelocate(Combobox_t* cb, rec drawRec)
{
	Assert(cb != nullptr);
	
	cb->drawRec = drawRec;
	cb->arrowsRec = drawRec;
	cb->arrowsRec.width = cb->arrowsRec.height;
	cb->arrowsRec.x += drawRec.width - cb->arrowsRec.width;
}

void ComboboxSetSelectedIndex(Combobox_t* cb, i32 selectedIndex)
{
	Assert(cb != nullptr);
	cb->selectedIndex = selectedIndex;
}

void ComboboxSetFont(Combobox_t* cb, const Font_t* font)
{
	Assert(cb != nullptr);
	cb->font = font;
}

r32 ComboboxMeasureMinWidth(Combobox_t* cb)
{
	Assert(cb != nullptr);
	Assert(cb->font != nullptr);
	
	r32 minTextWidth = 0;
	for (u32 cIndex = 0; cIndex < cb->options.count; cIndex++)
	{
		r32 newWidth = MeasureString(cb->font, cb->options[cIndex]).width;
		if (newWidth > minTextWidth)
		{
			minTextWidth = newWidth;
		}
	}
	
	return minTextWidth + cb->arrowsRec.width;
}

void ComboboxUpdate(Combobox_t* cb, bool activeElement)
{
	Assert(cb != nullptr);
	Assert(cb->selectedIndex < 0 || (u32)cb->selectedIndex < cb->options.count);
	
	if (activeElement)
	{
		rec upButtonRec = cb->arrowsRec;
		upButtonRec.height /= 2;
		rec downButtonRec = upButtonRec;
		downButtonRec.y += upButtonRec.height;
		
		bool upButtonPressed = false;
		bool downButtonPressed = false;
		if (input->mouseInsideWindow && IsInsideRec(upButtonRec, RenderMousePos))
		{
			if (ButtonReleased(MouseButton_Left) && IsInsideRec(upButtonRec, RenderMouseStartPos))
			{
				upButtonPressed = true;
			}
		}
		if (input->mouseInsideWindow && IsInsideRec(downButtonRec, RenderMousePos))
		{
			if (ButtonReleased(MouseButton_Left) && IsInsideRec(downButtonRec, RenderMouseStartPos))
			{
				downButtonPressed = true;
			}
		}
		
		if (upButtonPressed)
		{
			if (cb->selectedIndex < 0)
			{
				if (cb->options.count > 0)
				{
					cb->selectedIndex = cb->options.count - 1;
				}
			}
			else if (cb->selectedIndex > 0)
			{
				cb->selectedIndex--;
			}
		}
		else if (downButtonPressed)
		{
			if (cb->selectedIndex < 0)
			{
				if (cb->options.count > 0)
				{
					cb->selectedIndex = 0;
				}
			}
			else if ((u32)cb->selectedIndex < cb->options.count-1)
			{
				cb->selectedIndex++;
			}
		}
	}
}

void ComboboxRender(Combobox_t* cb, bool activeElement)
{
	Assert(cb != nullptr);
	
	rec oldViewport = app->renderState.viewport;
	const Font_t* oldFont = app->renderState.boundFont;
	RsBindFont(cb->font);
	
	RsSetViewport(cb->drawRec);
	RsDrawButton(cb->drawRec, NewColor(Color_White), NewColor(Color_Black), 1);
	
	RsSetViewport(RecInflate(cb->drawRec, -1));
	if (cb->selectedIndex >= 0)
	{
		const char* selectedStr = cb->options[cb->selectedIndex];
		v2 strPos = cb->drawRec.topLeft;
		strPos.x += (cb->drawRec.width - cb->arrowsRec.width) / 2;
		strPos.y += cb->drawRec.height/2 - cb->font->lineHeight/2 + cb->font->maxExtendUp;
		strPos = NewVec2((r32)RoundR32(strPos.x), (r32)RoundR32(strPos.y));
		RsDrawString(selectedStr, strPos, NewColor(Color_Black), 1.0f, Alignment_Center);
	}
	RsSetViewport(cb->drawRec);
	
	// RsDrawButton(RecInflate(cb->arrowsRec, 1), NewColor(Color_White), NewColor(Color_Black), 1);
	
	rec upButtonRec = cb->arrowsRec;
	upButtonRec.height /= 2;
	rec downButtonRec = upButtonRec;
	downButtonRec.y += upButtonRec.height;
	rec upArrowRec = RecInflate(upButtonRec, -1);
	upArrowRec = RecSquarify(upArrowRec);
	upArrowRec = RecInflateY(upArrowRec, -1);
	rec downArrowRec = RecInflate(downButtonRec, -1);
	downArrowRec = RecSquarify(downArrowRec);
	downArrowRec = RecInflateY(downArrowRec, -1);
	
	{
		Color_t backColor = NewColor(Color_White);
		Color_t iconColor = NewColor(Color_Black);
		Color_t borderColor = NewColor(Color_Black);
		if (cb->selectedIndex >= 0 && cb->selectedIndex == 0)
		{
			backColor = GC->colors.windowBackground1;
		}
		else
		{
			ButtonColorChoice(backColor, iconColor, borderColor, upButtonRec, false, false);
		}
		RsDrawButton(upButtonRec, backColor, borderColor, 1);
		RsBindTexture(&app->arrowSprite);
		RsDrawTexturedRec(RecInvertY(upArrowRec), iconColor);
	}
	{
		Color_t backColor = NewColor(Color_White);
		Color_t iconColor = NewColor(Color_Black);
		Color_t borderColor = NewColor(Color_Black);
		if (cb->selectedIndex >= 0 && cb->selectedIndex >= (i32)cb->options.count-1)
		{
			backColor = GC->colors.windowBackground1;
		}
		else
		{
			rec altRec = downButtonRec;
			altRec.y += 1;
			altRec.height -= 1;
			ButtonColorChoice(backColor, iconColor, borderColor, altRec, false, false);
		}
		RsDrawButton(downButtonRec, backColor, borderColor, 1);
		RsBindTexture(&app->arrowSprite);
		RsDrawTexturedRec(downArrowRec, iconColor);
	}
	
	RsSetViewport(oldViewport);
	RsBindFont(oldFont);
}
