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

void ClearConsole()
{
	DEBUG_WriteLine("Clearing Console");
	DestroyLineList(&app->lineList);
	InitializeLineList(&app->lineList, app->inputArenaBase, app->inputArenaSize);
	
	app->selectionStart = NewTextLocation(0, 0);
	app->selectionEnd = NewTextLocation(0, 0);
	app->uiElements.mouseTextLocation = NewTextLocation(0, 0);
	
	app->genericCounter = 0;
}

void RefreshComPortList()
{
	BoundedStrListDestroy(&app->availablePorts, &app->mainHeap);
	app->availablePorts = platform->GetComPortList(&app->mainHeap);
	
	StatusDebug("Found %u COM ports", app->availablePorts.count);
	for (u32 cIndex = 0; cIndex < app->availablePorts.count; cIndex++)
	{
		DEBUG_PrintLine("\"%s\"Available!", app->availablePorts[cIndex]);
	}
}

bool OpenComPort(const char* comPortName, ComSettings_t settings)
{
	if (app->comPort.isOpen && strcmp(app->comPort.name, comPortName) == 0)
	{
		//NOTE: If we want to open the same port again we have to close it first before opening it
		//      Otherwise we will try to open and only close the port if the open succeeds
		StatusError("Closed %s", app->comPort.name);
		platform->CloseComPort(&app->mainHeap, &app->comPort);
	}
	
	ComPort_t newComPort = platform->OpenComPort(&app->mainHeap, comPortName, settings);
	
	if (newComPort.isOpen)
	{
		if (app->comPort.isOpen)
		{
			StatusError("Closed %s", app->comPort.name);
			platform->CloseComPort(&app->mainHeap, &app->comPort);
		}
		ClearConsole();
		app->comPort = newComPort;
		
		PopupSuccess("\"%s\" Opened Successfully", comPortName);
		return true;
	}
	else
	{
		PopupError("Couldn't open \"%s\"!", comPortName);
		return false;
	}
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
		
		RefreshComPortList();
		
		comMenu->comListSelectedIndex = app->availablePorts.count;
		comMenu->comListOpenIndex = -1;
		if (app->comPort.isOpen)
		{
			comMenu->baudRateSelection = (u32)app->comPort.settings.baudRate;
			comMenu->numBitsSelection = app->comPort.settings.numBits;
			comMenu->paritySelection = (u32)app->comPort.settings.parity;
			comMenu->stopBitsSelection = (u32)app->comPort.settings.stopBits;
			for (u32 cIndex = 0; cIndex < app->availablePorts.count; cIndex++)
			{
				if (strcmp(app->availablePorts[cIndex], app->comPort.name) == 0)
				{
					comMenu->comListSelectedIndex = cIndex;
					comMenu->comListOpenIndex = cIndex;
				}
			}
		}
		else
		{
			comMenu->baudRateSelection = (u32)BaudRate_115200;
			comMenu->numBitsSelection = 8;
			comMenu->paritySelection = (u32)Parity_None;
			comMenu->stopBitsSelection = (u32)StopBits_1;
		}
		
		ChangeActiveElement(comMenu);
	}
	comMenu->open = true;
}

void ComMenuHide(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	if (comMenu->open == true)
	{
		DEBUG_WriteLine("Hiding COM Menu");
		
		if (GC->showInputTextBox) { ChangeActiveElement(&app->inputBox); }
		else { ChangeActiveElement(&app->uiElements.viewRec); }
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
	comMenu->drawRec.y += app->uiElements.mainMenuRec.height;
	comMenu->drawRec.height -= app->uiElements.mainMenuRec.height;
	comMenu->drawRec.height -= RenderScreenSize.height - app->uiElements.statusBarRec.y;
	// comMenu->drawRec = RecInflateY(comMenu->drawRec, -(50));
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
	comMenu->drawRec.topLeft = NewVec2((r32)RoundR32(comMenu->drawRec.x), (r32)RoundR32(comMenu->drawRec.y));
	comMenu->drawRec.size = NewVec2((r32)RoundR32(comMenu->drawRec.width), (r32)RoundR32(comMenu->drawRec.height));
	
	// +==============================+
	// |   Update Other Rectangles    |
	// +==============================+
	comMenu->comListRec = comMenu->drawRec;
	comMenu->comListRec.width = (r32)RoundR32(2*comMenu->comListRec.width/5);
	
	comMenu->connectRec = NewRec(comMenu->drawRec.topLeft + comMenu->drawRec.size, Vec2_Zero);
	const char* connectStr = "Connect";
	if (comMenu->comListSelectedIndex >= 0 && comMenu->comListSelectedIndex == comMenu->comListOpenIndex) { connectStr = "Reconnect"; }
	comMenu->connectRec.width = MeasureString(&app->uiFont, connectStr).width + 10;
	comMenu->connectRec.height = app->uiFont.lineHeight + 5;
	comMenu->connectRec.topLeft -= comMenu->connectRec.size + NewVec2(5);
	
	const char* disconnectStr = "Disconnect";
	v2 disconnectStrSize = MeasureString(&app->uiFont, disconnectStr);
	comMenu->disconnectRec = NewRec(comMenu->comListRec.topLeft, comMenu->comListRec.size);
	comMenu->disconnectRec.height = app->uiFont.lineHeight + 5;
	comMenu->disconnectRec.y += comMenu->comListRec.height - comMenu->disconnectRec.height - 5;
	comMenu->disconnectRec = RecInflateX(comMenu->disconnectRec, -5);
	if (comMenu->disconnectRec.width > disconnectStrSize.width + 10)
	{
		comMenu->disconnectRec.width = disconnectStrSize.width + 10;
	}
	if (!app->comPort.isOpen || comMenu->open == false || comMenu->animPercent < 0.8f)
	{
		comMenu->disconnectRec.height = 0;
	}
	
	comMenu->baudRatesRec = NewRec(
		comMenu->comListRec.x + comMenu->comListRec.width + 5,
		comMenu->drawRec.y + app->uiFont.lineHeight + 5,
		comMenu->drawRec.width - comMenu->comListRec.width - 10,
		(app->uiFont.lineHeight+5) * NumBaudRates + 5
	);
	comMenu->numBitsRec = comMenu->baudRatesRec;
	comMenu->baudRatesRec.width /= 2;
	comMenu->numBitsRec.width /= 2;
	comMenu->numBitsRec.x += comMenu->numBitsRec.width;
	comMenu->numBitsRec.x += 5/2;
	comMenu->baudRatesRec.width -= 5/2;
	comMenu->numBitsRec.width -= 5/2;
	comMenu->numBitsRec.height = app->uiFont.lineHeight + 5;
	comMenu->parityRec = comMenu->numBitsRec;
	comMenu->parityRec.y += comMenu->numBitsRec.height + app->uiFont.lineHeight;
	comMenu->stopBitsRec = comMenu->parityRec;
	comMenu->stopBitsRec.y += comMenu->parityRec.height + app->uiFont.lineHeight;
	
	if (comMenu->baudRatesRec.y + comMenu->baudRatesRec.height >= comMenu->connectRec.y)
	{
		comMenu->baudRatesRec.height = comMenu->connectRec.y - comMenu->baudRatesRec.y;
		if (comMenu->baudRatesRec.height < 0) { comMenu->baudRatesRec.height = 0; }
	}
	if (comMenu->numBitsRec.y + comMenu->numBitsRec.height > comMenu->connectRec.y)
	{
		comMenu->numBitsRec.height = 0;
	}
	if (comMenu->parityRec.y + comMenu->parityRec.height > comMenu->connectRec.y)
	{
		comMenu->parityRec.height = 0;
	}
	if (comMenu->stopBitsRec.y + comMenu->stopBitsRec.height > comMenu->connectRec.y)
	{
		comMenu->stopBitsRec.height = 0;
	}
	
	// +==================================+
	// | Check Disconnect Button Pressed  |
	// +==================================+
	bool disconnectButtonPressed = false;
	if (comMenu->disconnectRec.height > 0 && input->mouseInsideWindow && IsInsideRec(comMenu->disconnectRec, RenderMousePos))
	{
		if (ButtonReleasedUnhandled(MouseButton_Left) && IsInsideRec(comMenu->disconnectRec, RenderMouseStartPos))
		{
			HandleButton(MouseButton_Left);
			disconnectButtonPressed = true;
		}
	}
	if (disconnectButtonPressed)
	{
		PopupError("Closed \"%s\"", app->comPort.name);
		platform->CloseComPort(&app->mainHeap, &app->comPort);
		ClearConsole();
		RefreshComPortList();
		comMenu->comListSelectedIndex = app->availablePorts.count;
	}
	
	// +==============================+
	// |     Update Other Things      |
	// +==============================+
	comMenu->numComListItems = app->availablePorts.count + ((app->comPort.isOpen && !IsComAvailable(app->comPort.name)) ? 1 : 0);
	
	// +==============================+
	// | Check Connect Button Pressed |
	// +==============================+
	if (comMenu->open)
	{
		bool connectButtonPressed = false;
		if (input->mouseInsideWindow && IsInsideRec(comMenu->connectRec, RenderMousePos))
		{
			// AppOutput->cursorType = Cursor_Pointer;
			if (ButtonReleasedUnhandled(MouseButton_Left) && IsInsideRec(comMenu->connectRec, RenderMouseStartPos))
			{
				HandleButton(MouseButton_Left);
				connectButtonPressed = true;
			}
		}
		
		if (connectButtonPressed || ButtonPressed(Button_Enter))
		{
			if (comMenu->comListSelectedIndex >= 0 && (u32)comMenu->comListSelectedIndex < comMenu->numComListItems)
			{
				const char* selectedPortName = nullptr;
				if ((u32)comMenu->comListSelectedIndex < app->availablePorts.count) { selectedPortName = app->availablePorts[comMenu->comListSelectedIndex]; }
				else { selectedPortName = app->comPort.name; }
				selectedPortName = ArenaString(TempArena, NtStr(selectedPortName));
				ComSettings_t comSettings = {};
				comSettings.baudRate = (BaudRate_t)comMenu->baudRateSelection;
				comSettings.numBits = (u8)comMenu->numBitsSelection;
				comSettings.parity = (Parity_t)comMenu->paritySelection;
				comSettings.stopBits = (StopBits_t)comMenu->stopBitsSelection;
				comSettings.flowControlEnabled = false;
				
				DEBUG_PrintLine("Opening %s...", selectedPortName);
				if (OpenComPort(selectedPortName, comSettings))
				{
					ComMenuHide(comMenu);
				}
			}
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
	
	// +==============================+
	// |   Update Baud Rate Scroll    |
	// +==============================+
	{
		bool mouseInBaudRates = (input->mouseInsideWindow && IsInsideRec(comMenu->baudRatesRec, RenderMousePos));
		
		if (mouseInBaudRates && input->scrollDelta.y != 0)
		{
			comMenu->baudRateScrollGoto += GC->scrollMultiplier * -input->scrollDelta.y;
		}
		if (comMenu->baudRateScrollGoto + comMenu->baudRatesRec.height > comMenu->baudRatesHeight)
		{
			comMenu->baudRateScrollGoto = comMenu->baudRatesHeight - comMenu->baudRatesRec.height;
		}
		if (comMenu->baudRateScrollGoto < 0)
		{
			// comMenu->baudRateScroll += comMenu->baudRateScrollGoto;
			comMenu->baudRateScrollGoto = 0;
		}
		
		if (comMenu->baudRateScroll != comMenu->baudRateScrollGoto)
		{
			r32 delta = comMenu->baudRateScrollGoto - comMenu->baudRateScroll;
			if (AbsR32(delta) < 1)
			{
				comMenu->baudRateScroll = comMenu->baudRateScrollGoto;
			}
			else
			{
				comMenu->baudRateScroll += delta / GC->viewSpeedDivider;
			}
		}
	}
	
	// +==================================+
	// | Check For Com List Item Clicked  |
	// +==================================+
	if (comMenu->comListRec.height > 0)
	{
		comMenu->comListOpenIndex = -1;
		r32 yOffset = -(r32)RoundR32(comMenu->comListScroll);
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
			
			if (app->comPort.isOpen && strcmp(portName, app->comPort.name) == 0)
			{
				comMenu->comListOpenIndex = (i32)cIndex;
			}
			
			if (input->mouseInsideWindow && IsInsideRec(itemRec, RenderMousePos) && !IsInsideRec(comMenu->disconnectRec, RenderMousePos))
			{
				// AppOutput->cursorType = Cursor_Pointer;
				if (ButtonReleasedUnhandled(MouseButton_Left))
				{
					HandleButton(MouseButton_Left);
					DEBUG_PrintLine("Clicked on item %u", cIndex);
					comMenu->comListSelectedIndex = cIndex;
				}
			}
			
			yOffset += itemRec.height + 2;
		}
		comMenu->comListHeight = yOffset + comMenu->comListScroll;
		if (comMenu->disconnectRec.height > 0)
		{
			comMenu->comListHeight += (comMenu->comListRec.y + comMenu->comListRec.height) - comMenu->disconnectRec.y;
		}
	}
	
	// +======================================+
	// | Check For Baud Rate Option Selected  |
	// +======================================+
	if (comMenu->baudRatesRec.height > 0)
	{
		r32 yOffset = 5/2 - (r32)RoundR32(comMenu->baudRateScroll);
		for (u32 bIndex = 0; bIndex < NumBaudRates; bIndex++)
		{
			rec itemRec = NewRec(comMenu->baudRatesRec.topLeft, Vec2_Zero);
			itemRec.width = comMenu->baudRatesRec.width;
			itemRec.height = app->uiFont.lineHeight + 5 - 1;
			itemRec.y += yOffset;
			
			if (input->mouseInsideWindow && IsInsideRec(itemRec, RenderMousePos))
			{
				if (ButtonPressed(MouseButton_Left))
				{
					comMenu->baudRateSelection = bIndex;
				}
			}
			
			yOffset += itemRec.height+1;
		}
		comMenu->baudRatesHeight = yOffset + comMenu->baudRateScroll;
	}
}

void ComMenuDraw(ComMenu_t* comMenu)
{
	Assert(comMenu != nullptr);
	
	rec originalViewport = renderState->viewport;
	RsBindFont(&app->uiFont);
	
	if (comMenu->animPercent > 0)
	{
		#if 0
		rec dropShadowRec = comMenu->drawRec;
		dropShadowRec.height = 15;
		dropShadowRec.y = comMenu->drawRec.y + comMenu->drawRec.height;
		RsDrawGradient(dropShadowRec, ColorTransparent(NewColor(Color_Black), 0.5f), NewColor(Color_TransparentBlack), Dir2_Down);
		dropShadowRec.y = comMenu->drawRec.y - dropShadowRec.height;
		RsDrawGradient(dropShadowRec, ColorTransparent(NewColor(Color_Black), 0.5f), NewColor(Color_TransparentBlack), Dir2_Up);
		#endif
		
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
		if (comMenu->comListRec.height > 0)
		{
			RsDrawButton(RecInflate(comMenu->comListRec, 1), GC->colors.textBackground, GC->colors.windowOutline, 1.0f);
			RsSetViewport(comMenu->comListRec);
			
			r32 yOffset = -(r32)RoundR32(comMenu->comListScroll);
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
				
				bool isOpenPort     = (comMenu->comListOpenIndex != -1 && cIndex == (u32)comMenu->comListOpenIndex);
				bool hovering       = (input->mouseInsideWindow && IsInsideRec(itemRec, RenderMousePos) && !IsInsideRec(comMenu->disconnectRec, RenderMousePos));
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
			
			// v2 targetLine = NewVec2(
			// 	comMenu->comListRec.x,
			// 	comMenu->comListRec.y - comMenu->comListScroll + comMenu->comListScrollGoto
			// );
			// RsDrawLine(targetLine + NewVec2(0,  2), targetLine + NewVec2(comMenu->comListRec.width,  2), 4.0f, NewColor(Color_Red));
			// RsDrawLine(targetLine + NewVec2(0, -2), targetLine + NewVec2(comMenu->comListRec.width, -2), 4.0f, NewColor(Color_Blue));
			
		}
		
		RsSetViewport(comMenu->drawRec);
		
		// +==============================+
		// |     Draw the Baud Rates      |
		// +==============================+
		if (comMenu->baudRatesRec.height > 0)
		{
			RsDrawString("Baud Rate", comMenu->baudRatesRec.topLeft + NewVec2(0, -app->uiFont.maxExtendDown), NewColor(Color_White), 1.0f, Alignment_Left);
			RsDrawButton(comMenu->baudRatesRec, NewColor(Color_White), NewColor(Color_Black), 1.0f);
			
			RsSetViewport(RecInflate(comMenu->baudRatesRec, -1));
			
			r32 yOffset = 5/2 - (r32)RoundR32(comMenu->baudRateScroll);
			for (u32 bIndex = 0; bIndex < NumBaudRates; bIndex++)
			{
				rec itemRec = NewRec(comMenu->baudRatesRec.topLeft, Vec2_Zero);
				itemRec.width = comMenu->baudRatesRec.width;
				itemRec.height = app->uiFont.lineHeight + 5 - 1;
				itemRec.y += yOffset;
				Color_t itemTextColor = NewColor(Color_Black);
				Color_t itemBackColor = NewColor(Color_White);
				Color_t itemBorderColor = NewColor(Color_TransparentBlack);
				ButtonColorChoice(itemBackColor, itemTextColor, itemBorderColor, itemRec, comMenu->baudRateSelection == bIndex, false);
				itemRec.height += 1;
				RsDrawButton(itemRec, itemBackColor, itemBorderColor, 1.0f);
				
				BaudRate_t baudRate = (BaudRate_t)bIndex;
				const char* baudRateStr = GetBaudRateString(baudRate);
				v2 strSize = MeasureString(&app->uiFont, baudRateStr);
				v2 strPos = itemRec.topLeft + NewVec2(comMenu->baudRatesRec.width/2, itemRec.height/2 - app->uiFont.lineHeight/2 + app->uiFont.maxExtendUp);
				strPos = NewVec2((r32)RoundR32(strPos.x), (r32)RoundR32(strPos.y));
				RsDrawString(baudRateStr, strPos, itemTextColor, 1.0f, Alignment_Center);
				
				yOffset += itemRec.height;
			}
			
			RsSetViewport(comMenu->drawRec);
		}
		
		// +==============================+
		// |  Draw the Num Bits Selector  |
		// +==============================+
		if (comMenu->numBitsRec.height > 0)
		{
			RsDrawString("# Bits", comMenu->numBitsRec.topLeft + NewVec2(0, -app->uiFont.maxExtendDown), NewColor(Color_White), 1.0f, Alignment_Left);
			RsDrawButton(comMenu->numBitsRec, NewColor(Color_White), NewColor(Color_Black), 1.0f);
		}
		
		// +==============================+
		// |   Draw the Parity Selector   |
		// +==============================+
		if (comMenu->parityRec.height > 0)
		{
			RsDrawString("Parity", comMenu->parityRec.topLeft + NewVec2(0, -app->uiFont.maxExtendDown), NewColor(Color_White), 1.0f, Alignment_Left);
			RsDrawButton(comMenu->parityRec, NewColor(Color_White), NewColor(Color_Black), 1.0f);
		}
		
		// +==============================+
		// | Draw the Stop Bits Selector  |
		// +==============================+
		if (comMenu->stopBitsRec.height > 0)
		{
			RsDrawString("Stop Bits", comMenu->stopBitsRec.topLeft + NewVec2(0, -app->uiFont.maxExtendDown), NewColor(Color_White), 1.0f, Alignment_Left);
			RsDrawButton(comMenu->stopBitsRec, NewColor(Color_White), NewColor(Color_Black), 1.0f);
		}
		
		// +==============================+
		// |     Draw Connect Button      |
		// +==============================+
		{
			Color_t btnColorBack = GC->colors.button;
			Color_t btnColorText = GC->colors.buttonText;
			Color_t btnColorBorder = GC->colors.buttonBorder;
			if (comMenu->comListSelectedIndex >= 0 && (u32)comMenu->comListSelectedIndex < comMenu->numComListItems)
			{
				bool highlightConnect = (comMenu->comListSelectedIndex != comMenu->comListOpenIndex);
				ButtonColorChoice(btnColorBack, btnColorText, btnColorBorder, comMenu->connectRec, false, highlightConnect);
			}
			else
			{
				btnColorBack = GC->colors.windowBackground2;
			}
			
			RsDrawButton(comMenu->connectRec, btnColorBack, btnColorBorder);
			
			const char* connectStr = "Connect";
			if (comMenu->comListSelectedIndex >= 0 && comMenu->comListSelectedIndex == comMenu->comListOpenIndex) { connectStr = "Reconnect"; }
			v2 textPos = comMenu->connectRec.topLeft + NewVec2(comMenu->connectRec.width/2, comMenu->connectRec.height/2 - app->uiFont.lineHeight/2 + app->uiFont.maxExtendUp);
			textPos = NewVec2((r32)RoundR32(textPos.x), (r32)RoundR32(textPos.y));
			RsBindFont(&app->uiFont);
			RsDrawString(connectStr, textPos, btnColorText, 1.0f, Alignment_Center);
			RsBindFont(&app->mainFont);
		}
		
		// +==============================+
		// |    Draw Disconnect Button    |
		// +==============================+
		if (comMenu->disconnectRec.height > 0)
		{
			Color_t btnColorBack = GC->colors.errorMessage;
			Color_t btnColorText = GC->colors.buttonText;
			Color_t btnColorBorder = GC->colors.buttonBorder;
			ButtonColorChoice(btnColorBack, btnColorText, btnColorBorder, comMenu->disconnectRec, false, false);
			
			RsDrawButton(comMenu->disconnectRec, btnColorBack, btnColorBorder);
			
			const char* disconnectStr = "Disconnect";
			v2 textPos = comMenu->disconnectRec.topLeft + NewVec2(comMenu->disconnectRec.width/2, comMenu->disconnectRec.height/2 - app->uiFont.lineHeight/2 + app->uiFont.maxExtendUp);
			textPos = NewVec2((r32)RoundR32(textPos.x), (r32)RoundR32(textPos.y));
			RsBindFont(&app->uiFont);
			RsDrawString(disconnectStr, textPos, btnColorText, 1.0f, Alignment_Center);
			RsBindFont(&app->mainFont);
		}
		
		RsDrawButton(RecInflateX(comMenu->drawRec, 1), NewColor(Color_TransparentBlack), menuBorderColor, 1.0f);
	}
	
	RsBindFont(&app->mainFont);
	RsSetViewport(originalViewport);
}
