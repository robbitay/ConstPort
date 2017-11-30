/*
File:   appComMenu.cpp
Author: Taylor Robbins
Date:   11\28\2017
Description: 
	** Holds functions that handle the COM Menu that is presented to
	** the user when they need to choose a COM port to open 

#included from app.cpp
*/

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
const char* GetPortUserName(const char* portName)
{
	for (u32 nIndex = 0; nIndex < GC->comNameKeys.count; nIndex++)
	{
		Assert(nIndex < GC->comNameValues.count);
		
		if (strcmp(GC->comNameKeys[nIndex], portName) == 0)
		{
			return GC->comNameValues[nIndex];
		}
	}
	
	return portName;
}

bool IsComAvailable(const char* comName)
{
	for (u32 cIndex = 0; cIndex < app->availablePorts.count; cIndex++)
	{
		if (strcmp(app->availablePorts[cIndex], comName) == 0)
		{
			return true;
		}
	}
	
	return false;
}

// +--------------------------------------------------------------+
// |                       Public Functions                       |
// +--------------------------------------------------------------+
void ComMenuInitialize(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	ClearPointer(comMenu);
	
	
}

void ComMenuShow(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	if (comMenu->open == false)
	{
		DEBUG_WriteLine("Showing COM Menu");
		
		comMenu->comListSelectedIndex = app->availablePorts.count;
		if (app->comPort.isOpen)
		{
			for (u32 cIndex = 0; cIndex < app->availablePorts.count; cIndex++)
			{
				if (strcmp(app->availablePorts[cIndex], app->comPort.name) == 0)
				{
					comMenu->comListSelectedIndex = cIndex;
				}
			}
		}
	}
	comMenu->open = true;
}

void ComMenuHide(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	if (comMenu->open == true)
	{
		DEBUG_WriteLine("Hiding COM Menu");
		//TODO: Any opening initialization?
	}
	comMenu->open = false;
}

void ComMenuToggle(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	if (comMenu->open) { ComMenuHide(comMenu); }
	else               { ComMenuShow(comMenu); }
}

void ComMenuUpdate(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	
	// +==============================+
	// |     Update Anim Percent      |
	// +==============================+
	if (comMenu->open)
	{
		if (comMenu->animPercent < 1.0f)
		{
			comMenu->animPercent += COM_MENU_OPEN_SPEED;
			if (comMenu->animPercent >= 1.0f) { comMenu->animPercent = 1.0f; }
		}
	}
	else
	{
		if (comMenu->animPercent > 0.0f)
		{
			comMenu->animPercent -= COM_MENU_OPEN_SPEED;
			if (comMenu->animPercent <= 0.0f) { comMenu->animPercent = 0.0f; }
		}
	}
	
	// +==============================+
	// |    Update Menu Rectangle     |
	// +==============================+
	comMenu->drawRec = NewRec(Vec2_Zero, RenderScreenSize);
	comMenu->drawRec = RecInflateY(comMenu->drawRec, -(50));
	if (comMenu->animPercent < 0.5f)
	{
		comMenu->drawRec.y += comMenu->drawRec.height/2;
		comMenu->drawRec.height = 1.0f;
		r32 openPercent = Ease(EasingStyle_QuadraticOut, comMenu->animPercent * 2.0f);
		comMenu->drawRec = RecInflateX(comMenu->drawRec, -(comMenu->drawRec.width/2) * (1.0f - openPercent));
	}
	else if (comMenu->animPercent < 1.0f)
	{
		r32 openPercent = Ease(EasingStyle_QuadraticOut, (comMenu->animPercent - 0.5f) * 2.0f);
		comMenu->drawRec = RecInflateY(comMenu->drawRec, -(comMenu->drawRec.height/2) * (1.0f - openPercent));
	}
	
	// +==============================+
	// |   Update Other Rectangles    |
	// +==============================+
	comMenu->comListRec = comMenu->drawRec;
	comMenu->comListRec.width = (r32)RoundR32(2*comMenu->comListRec.width/5);
	comMenu->connectRec = NewRec(comMenu->drawRec.topLeft + comMenu->drawRec.size, NewVec2(100, 20));
	comMenu->connectRec.topLeft -= comMenu->connectRec.size + NewVec2(5);
	
	// +==============================+
	// |     Update Other Things      |
	// +==============================+
	comMenu->numComListItems = app->availablePorts.count + ((app->comPort.isOpen && !IsComAvailable(app->comPort.name)) ? 1 : 0);
	
	// +==============================+
	// | Check Connect Button Pressed |
	// +==============================+
	if (input->mouseInsideWindow && IsInsideRec(comMenu->connectRec, RenderMousePos))
	{
		// AppOutput->cursorType = Cursor_Pointer;
		if (ButtonPressedUnhandled(MouseButton_Left))
		{
			DEBUG_WriteLine("Clicked connect!");
		}
	}
	
	
	// +==============================+
	// |    Update Com List Scroll    |
	// +==============================+
	{
		bool mouseInComList = (input->mouseInsideWindow && IsInsideRec(comMenu->comListRec, RenderMousePos));
		
		if (mouseInComList && input->scrollDelta.y != 0)
		{
			comMenu->comListScrollGoto += GC->scrollMultiplier * -input->scrollDelta.y;
		}
		if (comMenu->comListScrollGoto + comMenu->comListRec.height > comMenu->comListHeight)
		{
			comMenu->comListScrollGoto = comMenu->comListHeight - comMenu->comListRec.height;
		}
		if (comMenu->comListScrollGoto < 0)
		{
			// comMenu->comListScroll += comMenu->comListScrollGoto;
			comMenu->comListScrollGoto = 0;
		}
		
		if (comMenu->comListScroll != comMenu->comListScrollGoto)
		{
			r32 delta = comMenu->comListScrollGoto - comMenu->comListScroll;
			if (AbsR32(delta) < 1)
			{
				comMenu->comListScroll = comMenu->comListScrollGoto;
			}
			else
			{
				comMenu->comListScroll += delta / GC->viewSpeedDivider;
			}
		}
	}
	
	// +==================================+
	// | Check For Com List Item Clicked  |
	// +==================================+
	{
		r32 yOffset = -comMenu->comListScroll;
		for (u32 cIndex = 0; cIndex < comMenu->numComListItems; cIndex++)
		{
			rec itemRec = NewRec(comMenu->comListRec.x, comMenu->comListRec.y + yOffset, comMenu->comListRec.width, 0);
			r32 maxWidth = itemRec.width - (COM_LIST_ITEM_PADDING*2);
			
			char* portName = nullptr;
			if (cIndex < app->availablePorts.count) { portName = app->availablePorts[cIndex]; }
			else { portName = app->comPort.name; }
			const char* portUserName = GetPortUserName(portName);
			v2 textSize = MeasureFormattedString(&app->uiFont, NtStr(portUserName), maxWidth, true);
			const char* subText = portName;
			v2 subTextSize = Vec2_Zero;
			if (strcmp(portUserName, subText) != 0)
			{
				subTextSize = MeasureFormattedString(&app->uiFont, NtStr(subText), maxWidth, true);
			}
			itemRec.height = textSize.height + subTextSize.height + (COM_LIST_ITEM_PADDING*2);
			
			if (input->mouseInsideWindow && IsInsideRec(itemRec, RenderMousePos))
			{
				// AppOutput->cursorType = Cursor_Pointer;
				if (ButtonPressedUnhandled(MouseButton_Left))
				{
					DEBUG_PrintLine("Clicked on item %u", cIndex);
					comMenu->comListSelectedIndex = cIndex;
				}
			}
			
			yOffset += itemRec.height + 2;
		}
		comMenu->comListHeight = yOffset + comMenu->comListScroll;
	}
}

void ComMenuDraw(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	
	rec originalViewport = renderState->viewport;
	
	if (comMenu->animPercent > 0)
	{
		rec dropShadowRec = comMenu->drawRec;
		dropShadowRec.height = 15;
		dropShadowRec.y = comMenu->drawRec.y + comMenu->drawRec.height;
		RsDrawGradient(dropShadowRec, ColorTransparent(NewColor(Color_Black), 0.5f), NewColor(Color_TransparentBlack), Dir2_Down);
		dropShadowRec.y = comMenu->drawRec.y - dropShadowRec.height;
		RsDrawGradient(dropShadowRec, ColorTransparent(NewColor(Color_Black), 0.5f), NewColor(Color_TransparentBlack), Dir2_Up);
		
		RsSetViewport(comMenu->drawRec);
		
		Color_t menuBackColor1 = GC->colors.windowBackground1;
		Color_t menuBackColor2 = GC->colors.windowBackground2;
		Color_t menuBorderColor = GC->colors.windowOutline;
		// RsDrawRectangle(comMenu->drawRec, menuBackColor);
		RsDrawGradient(comMenu->drawRec, menuBackColor1, menuBackColor2, Dir2_Down);
		
		bool currentComSelected = false;
		// +==============================+
		// |      Draw the COM List       |
		// +==============================+
		{
			RsDrawButton(RecInflate(comMenu->comListRec, 1), GC->colors.textBackground, GC->colors.windowOutline, 1.0f);
			RsSetViewport(comMenu->comListRec);
			
			RsBindFont(&app->uiFont);
			r32 yOffset = -comMenu->comListScroll;
			for (u32 cIndex = 0; cIndex < comMenu->numComListItems; cIndex++)
			{
				rec itemRec = NewRec(comMenu->comListRec.x, comMenu->comListRec.y + yOffset, comMenu->comListRec.width, 0);
				r32 maxWidth = itemRec.width - (COM_LIST_ITEM_PADDING*2);
				
				char* portName = nullptr;
				if (cIndex < app->availablePorts.count) { portName = app->availablePorts[cIndex]; }
				else { portName = app->comPort.name; }
				const char* portUserName = GetPortUserName(portName);
				v2 textSize = MeasureFormattedString(&app->uiFont, NtStr(portUserName), maxWidth, true);
				const char* subText = portName;
				v2 subTextSize = Vec2_Zero;
				bool drawSubText = (strcmp(portUserName, subText) != 0);
				if (drawSubText)
				{
					subTextSize = MeasureFormattedString(&app->uiFont, NtStr(subText), maxWidth, true);
				}
				itemRec.height = textSize.height + subTextSize.height + (COM_LIST_ITEM_PADDING*2);
				
				bool isOpenPort = (app->comPort.isOpen && strcmp(portName, app->comPort.name) == 0);
				bool hovering = (input->mouseInsideWindow && IsInsideRec(itemRec, RenderMousePos));
				bool isSelectedItem = (comMenu->comListSelectedIndex >= 0 && cIndex == (u32)comMenu->comListSelectedIndex);
				if (isOpenPort && isSelectedItem)
				{
					currentComSelected = true;
				}
				
				Color_t itemTextColor = NewColor(Color_White);
				Color_t itemSubTextColor = GC->colors.windowBackground1;
				Color_t itemBackColor = NewColor(Color_TransparentBlack);
				Color_t itemBorderColor = NewColor(Color_TransparentBlack);
				if (isSelectedItem)
				{
					itemBackColor = GC->colors.buttonReady;
					itemTextColor = GC->colors.windowBackground1;
					itemBorderColor = NewColor(Color_White);
					if (hovering)
					{
						itemBackColor = ColorLerp(itemBackColor, NewColor(Color_Black), 0.2f);
					}
					if (isOpenPort)
					{
						itemBorderColor = GC->colors.buttonSelected;
					}
				}
				else if (isOpenPort)
				{
					itemTextColor = GC->colors.buttonSelected;
					itemBorderColor = GC->colors.buttonSelected;
					if (hovering)
					{
						itemBackColor = NewColor(Color_Black);
					}
				}
				else if (hovering)
				{
					itemBackColor = NewColor(Color_Black);
					itemBorderColor = NewColor(Color_Black);
				}
				
				RsDrawButton(RecInflateX(itemRec, 1), itemBackColor, itemBorderColor);
				
				v2 textPos = itemRec.topLeft + NewVec2(itemRec.width/2, COM_LIST_ITEM_PADDING + app->uiFont.maxExtendUp);
				RsDrawFormattedString(portUserName, textPos, maxWidth, itemTextColor, Alignment_Center, true);
				if (drawSubText)
				{
					textPos.y += textSize.y;
					RsDrawFormattedString(subText, textPos, maxWidth, itemSubTextColor, Alignment_Center, true);
				}
				
				yOffset += itemRec.height + 2;
			}
			RsBindFont(&app->mainFont);
			
			// v2 targetLine = NewVec2(
			// 	comMenu->comListRec.x,
			// 	comMenu->comListRec.y - comMenu->comListScroll + comMenu->comListScrollGoto
			// );
			// RsDrawLine(targetLine + NewVec2(0,  2), targetLine + NewVec2(comMenu->comListRec.width,  2), 4.0f, NewColor(Color_Red));
			// RsDrawLine(targetLine + NewVec2(0, -2), targetLine + NewVec2(comMenu->comListRec.width, -2), 4.0f, NewColor(Color_Blue));
			
			RsSetViewport(comMenu->drawRec);
		}
		
		// +==============================+
		// |     Draw Connect Button      |
		// +==============================+
		{
			Color_t btnColorBack = GC->colors.button;
			Color_t btnColorText = GC->colors.buttonText;
			Color_t btnColorBorder = GC->colors.buttonBorder;
			
			bool highlightConnect = (comMenu->comListSelectedIndex >= 0 && (u32)comMenu->comListSelectedIndex < comMenu->numComListItems && !currentComSelected);
			ButtonColorChoice(btnColorBack, btnColorText, btnColorBorder, comMenu->connectRec, false, highlightConnect);
			
			RsDrawButton(comMenu->connectRec, btnColorBack, btnColorBorder);
			
			v2 textPos = comMenu->connectRec.topLeft + NewVec2(comMenu->connectRec.width/2, comMenu->connectRec.height/2 - app->uiFont.lineHeight/2 + app->uiFont.maxExtendUp);
			textPos = NewVec2((r32)RoundR32(textPos.x), (r32)RoundR32(textPos.y));
			RsBindFont(&app->uiFont);
			RsDrawString("Connect", textPos, btnColorText, 1.0f, Alignment_Center);
			RsBindFont(&app->mainFont);
		}
		
		RsDrawButton(RecInflateX(comMenu->drawRec, 1), NewColor(Color_TransparentBlack), menuBorderColor, 1.0f);
	}
	
	RsSetViewport(originalViewport);
}
