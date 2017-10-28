/*
File:   appUiHandler.cpp
Author: Taylor Robbins
Date:   06\17\2017
Description: 
	** Handles the sizing and interaction of all the major 
	** UI elements that aren't windows 

#included from app.cpp
*/

void InitializeUiElements(UiElements_t* ui)
{
	ClearPointer(ui);
	
	ui->buttonBaseTexture =                      LoadTexture("Resources/Sprites/buttonBase.png");
	ui->buttonDarkenTexture =                    LoadTexture("Resources/Sprites/buttonDarken.png");
	ui->buttonHighlightTexture =                 LoadTexture("Resources/Sprites/buttonLighten.png");
	ui->buttonTextures[Button_ComPort] =         LoadTexture("Resources/Sprites/buttonIcon4.png");
	// ui->buttonTextures[Button_ConsoleSettings] = LoadTexture("Resources/Sprites/buttonIcon1.png");
	ui->buttonTextures[Button_Settings] =        LoadTexture("Resources/Sprites/buttonIcon5.png");
	ui->buttonTextures[Button_About] =           LoadTexture("Resources/Sprites/buttonIcon2.png");
}

void RecalculateUiElements(UiElements_t* ui, bool resetFollowingEndOfFile)
{
	//Static sizing helpers
	ui->lineHeight = app->mainFont.lineHeight + GC->lineSpacing;
	
	//Static Rectangles
	ui->mainMenuRec = NewRectangle(
		0, 0,
		RenderScreenSize.x, 
		MAIN_MENU_HEIGHT
	);
	r32 ledExtend = (r32)GC->rxTxLedRingSize * 9;
	ui->rxLedRec = NewRectangle(
		ui->mainMenuRec.x + ui->mainMenuRec.width - RX_TX_LED_SIZE - ledExtend - 4,
		ui->mainMenuRec.y + ledExtend + 4,
		RX_TX_LED_SIZE, RX_TX_LED_SIZE
	);
	ui->txLedRec = NewRectangle(
		ui->rxLedRec.x - RX_TX_LED_SIZE - ledExtend*2 - 4,
		ui->rxLedRec.y,
		RX_TX_LED_SIZE, RX_TX_LED_SIZE
	);
	v2 clearStringSize = MeasureString(&app->uiFont, "Clear");
	ui->clearButtonRec = NewRectangle(
		ui->rxLedRec.x + ui->rxLedRec.width + ledExtend,
		ui->rxLedRec.y + ui->rxLedRec.height + ledExtend,
		clearStringSize.x + 4, clearStringSize.y + 4
	);
	if (ledExtend*4 + ui->rxLedRec.width + ui->txLedRec.width > ui->clearButtonRec.width)
	{ ui->clearButtonRec.width = ledExtend*4 + ui->rxLedRec.width + ui->txLedRec.width; }
	ui->clearButtonRec.x -= ui->clearButtonRec.width;
	r32 clearExtendsDown = (ui->clearButtonRec.y + ui->clearButtonRec.height + 4) - ui->mainMenuRec.y;
	if (clearExtendsDown > ui->mainMenuRec.height)
	{
		ui->mainMenuRec.height = clearExtendsDown;
	}
	for (u32 bIndex = 0; bIndex < NumMainMenuButtons; bIndex++)
	{
		r32 buttonSize = ui->mainMenuRec.height - MAIN_MENU_BUTTON_PADDING*2;
		ui->buttonRecs[bIndex] = NewRectangle(
			ui->mainMenuRec.x + MAIN_MENU_BUTTON_PADDING + (buttonSize+MAIN_MENU_BUTTON_PADDING) * bIndex,
			ui->mainMenuRec.y + MAIN_MENU_BUTTON_PADDING,
			buttonSize, buttonSize
		);
	}
	ui->saveButtonRec = NewRectangle(
		ui->clearButtonRec.x - SAVE_BUTTON_WIDTH - MAIN_MENU_BUTTON_PADDING,
		ui->clearButtonRec.y,
		SAVE_BUTTON_WIDTH, SAVE_BUTTON_HEIGHT
	);
	ui->statusBarRec = NewRectangle(
		0, 
		RenderScreenSize.y - app->uiFont.lineHeight, 
		RenderScreenSize.x, 
		app->uiFont.lineHeight
	);
	
	ui->scrollBarGutterRec = NewRectangle(
		RenderScreenSize.x - (r32)GC->scrollbarWidth - (r32)GC->scrollbarPadding*2, 
		ui->mainMenuRec.y + ui->mainMenuRec.height, 
		(r32)GC->scrollbarWidth + (r32)GC->scrollbarPadding*2, 
		0
	);
	ui->scrollBarGutterRec.height = ui->statusBarRec.y - ui->scrollBarGutterRec.y;
	ui->gotoEndButtonRec = NewRectangle(
		ui->statusBarRec.x + ui->statusBarRec.width - ui->statusBarRec.height,
		ui->statusBarRec.y,
		ui->statusBarRec.height,
		ui->statusBarRec.height
	);
	ui->gutterRec = NewRectangle(
		0, 
		ui->mainMenuRec.y + ui->mainMenuRec.height, 
		(r32)GC->minGutterWidth,
		0
	);
	ui->gutterRec.height = ui->statusBarRec.y - ui->gutterRec.y;
	if (GC->showLineNumbers)
	{
		ui->gutterRec.width = NumDecimalDigits(app->lineList.numLines + app->lineList.firstLineNum) * MeasureString(&app->mainFont, " ", 1).x + 2;
		if (ui->gutterRec.width < (r32)GC->minGutterWidth) ui->gutterRec.width = (r32)GC->minGutterWidth;
	}
	
	ui->textInputRec = NewRectangle(
		ui->gutterRec.x + ui->gutterRec.width,
		ui->statusBarRec.y,
		0,
		app->mainFont.lineHeight + 10
	);
	ui->textInputRec.width = ui->scrollBarGutterRec.x - ui->textInputRec.x;
	if (GC->showInputTextBox == false)
	{
		ui->textInputRec.height = 0;
	}
	ui->textInputRec.y -= ui->textInputRec.height;
	
	ui->sendButtonRec = NewRectangle(
		ui->textInputRec.x + ui->textInputRec.width,
		ui->textInputRec.y,
		0, ui->textInputRec.height
	);
	const char* sendButtonText = (ButtonDown(Button_Control) ? "Send HEX" : "Send");
	ui->sendButtonRec.width = MeasureString(&app->uiFont, sendButtonText).x + 10;
	ui->sendButtonRec.x    -= ui->sendButtonRec.width;
	ui->textInputRec.width -= ui->sendButtonRec.width;
	
	ui->viewRec = NewRectangle(
		ui->gutterRec.x + ui->gutterRec.width, 
		ui->mainMenuRec.y + ui->mainMenuRec.height,
		0, 0
	);
	ui->viewRec.width = ui->scrollBarGutterRec.x - ui->viewRec.x;
	ui->viewRec.height = ui->textInputRec.y - ui->viewRec.y;
	
	//Dynamic helpers
	ui->fileSize = MeasureLines(&app->lineList, ui->viewRec.width - GC->lineSpacing);
	ui->fileSize.x += 10;
	// DEBUG_PrintLine("FileSize = (%f, %f)", ui->fileSize.x, ui->fileSize.y);
	ui->maxScrollOffset = NewVec2(
		ui->fileSize.x - ui->viewRec.width,
		ui->fileSize.y - ui->viewRec.height
	);
	if (ui->scrollOffsetGoto.x > ui->maxScrollOffset.x) ui->scrollOffsetGoto.x = ui->maxScrollOffset.x;
	if (ui->scrollOffsetGoto.y > ui->maxScrollOffset.y) ui->scrollOffsetGoto.y = ui->maxScrollOffset.y;
	if (ui->scrollOffsetGoto.x < 0) ui->scrollOffsetGoto.x = 0;
	if (ui->scrollOffsetGoto.y < 0) ui->scrollOffsetGoto.y = 0;
	if (ui->scrollOffset.x > ui->maxScrollOffset.x) ui->scrollOffset.x = ui->maxScrollOffset.x;
	if (ui->scrollOffset.y > ui->maxScrollOffset.y) ui->scrollOffset.y = ui->maxScrollOffset.y;
	if (ui->scrollOffset.x < 0) ui->scrollOffset.x = 0;
	if (ui->scrollOffset.y < 0) ui->scrollOffset.y = 0;
	ui->scrollPercent.x = ui->scrollOffsetGoto.x / ui->maxScrollOffset.x;
	ui->scrollPercent.y = ui->scrollOffsetGoto.y / ui->maxScrollOffset.y;
	if (resetFollowingEndOfFile)
	{
		ui->followingEndOfFile = (ui->scrollOffsetGoto.y + ui->viewRec.height) >= (ui->fileSize.y - ui->lineHeight);
	}
	
	//Scroll Bar
	ui->scrollBarRec = NewRectangle(
		ui->scrollBarGutterRec.x + (r32)GC->scrollbarPadding, 0,
		(r32)GC->scrollbarWidth,
		ui->scrollBarGutterRec.height * (ui->viewRec.height / ui->fileSize.y)
	);
	if (ui->scrollBarRec.height < (r32)GC->minScrollbarHeight)
		ui->scrollBarRec.height = (r32)GC->minScrollbarHeight;
	if (ui->scrollBarRec.height > ui->scrollBarGutterRec.height)
		ui->scrollBarRec.height = ui->scrollBarGutterRec.height;
	ui->scrollBarRec.y = ui->scrollBarGutterRec.y + (ui->scrollBarGutterRec.height - ui->scrollBarRec.height) * ui->scrollPercent.y;
	
	v2 mouseViewPos = RenderMousePos - ui->viewRec.topLeft + ui->scrollOffset;
	ui->mouseTextLocation = GetTextLocation(&app->lineList, mouseViewPos, &ui->mouseTextLineLocation);
	ui->firstRenderLine = GetLineIndex(&app->lineList, ui->scrollOffset.y, &ui->firstRenderLineOffset);
	
	// DEBUG_PrintLine("scrollOffset = (%f, %f)", ui->scrollOffset.x, ui->scrollOffset.y);
	// DEBUG_PrintLine("MaxScrollOffset = (%f, %f)", ui->maxScrollOffset.x, ui->maxScrollOffset.y);
}

void UpdateUiElements(UiElements_t* ui)
{
	v2 gotoOffset = (ui->scrollOffsetGoto - ui->scrollOffset);
	
	if (Abs32(gotoOffset.x) < 1)
	{
		ui->scrollOffset.x = ui->scrollOffsetGoto.x;
	}
	else
	{
		ui->scrollOffset.x += gotoOffset.x / (r32)GC->viewSpeedDivider;
	}
	
	if (Abs32(gotoOffset.y) < 1)
	{
		ui->scrollOffset.y = ui->scrollOffsetGoto.y;
	}
	else
	{
		ui->scrollOffset.y += gotoOffset.y / (r32)GC->viewSpeedDivider;
	}
}

void SetCheckbox(Checkbox_t* checkboxPntr, bool checked)
{
	Assert(checkboxPntr != nullptr);
	
	if (checkboxPntr->checked != checked)
	{
		checkboxPntr->checked = checked;
		checkboxPntr->changeTime = platform->programTime;
		checkboxPntr->mouseHasLeft = false;
	}
}

void ToggleCheckbox(Checkbox_t* checkboxPntr)
{
	Assert(checkboxPntr != nullptr);
	
	SetCheckbox(checkboxPntr, !checkboxPntr->checked);
}

void UpdateCheckbox(Checkbox_t* checkboxPntr)
{
	Assert(checkboxPntr != nullptr);
	
	bool mouseInside = IsInsideRectangle(RenderMousePos, checkboxPntr->drawRec);
	bool mouseStartedInside = IsInsideRectangle(RenderMouseStartPos, checkboxPntr->drawRec);
	
	if (!mouseInside) { checkboxPntr->mouseHasLeft = true; }
	
	if (checkboxPntr->enabled)
	{
		if (mouseInside && mouseStartedInside && ButtonReleased(MouseButton_Left))
		{
			ToggleCheckbox(checkboxPntr);
		}
	}
	
}

void DrawCheckbox(Checkbox_t* checkboxPntr, RenderState_t* rs, Font_t* labelFont)
{
	Assert(checkboxPntr != nullptr);
	Assert(rs != nullptr);
	
	bool mouseInside = IsInsideRectangle(RenderMousePos, checkboxPntr->drawRec);
	bool mouseStartedInside = IsInsideRectangle(RenderMouseStartPos, checkboxPntr->drawRec);
	
	Assert(platform->programTime >= checkboxPntr->changeTime);
	u64 timeSinceClick = platform->programTime - checkboxPntr->changeTime;
	float animAmount = Clamp32((r32)timeSinceClick / GC->checkboxAnimTime, 0.0f, 1.0f);
	if (!checkboxPntr->checked)
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
	Color_t selectedColor = checkboxPntr->activeColor;
	Color_t currentColor = unselectedColor;
	Color_t textColor = unselectedColor;
	if (checkboxPntr->enabled)
	{
		if (mouseInside)
		{
			outlineColor = checkboxPntr->activeColor;
			// if (ButtonDown(MouseButton_Left) && IsInsideRectangle(RenderMouseStartPos, checkboxPntr->drawRec))
			// {
			// 	outlineColor = GC->colors.buttonPress;
			// }
		}
		currentColor = ColorLerp(unselectedColor, selectedColor, animAmount);
		
		textColor = currentColor;
		if (mouseInside && (checkboxPntr->mouseHasLeft || checkboxPntr->checked))
		{
			textColor = checkboxPntr->activeColor;
		}
		if (checkboxPntr->mouseHasLeft == false && checkboxPntr->checked == false)
		{
			outlineColor = currentColor;
		}
		if (animAmount != 1.0f && checkboxPntr->checked == true)
		{
			outlineColor = checkboxPntr->activeColor;
		}
	}
	
	if (animAmount > 0)
	{
		rs->SetCircleRadius(animAmount*1.1f, 0.0f);
		rs->DrawRectangle(checkboxPntr->drawRec, currentColor);
		rs->SetCircleRadius(0.0f, 0.0f);
	}
	if (checkboxPntr->enabled)
	{
		rs->DrawButton(checkboxPntr->drawRec, {Color_TransparentBlack}, outlineColor, 1);
	}
	else
	{
		rs->DrawRectangle(checkboxPntr->drawRec, outlineColor);
	}
	
	const Font_t* actualFont = rs->boundFont;
	const Font_t* originalFont = rs->boundFont;
	if (labelFont != nullptr)
	{
		actualFont = labelFont;
		rs->BindFont(labelFont);
	}
	
	v2 labelPos = NewVec2(
		checkboxPntr->drawRec.x + checkboxPntr->drawRec.width + 5,
		checkboxPntr->drawRec.y + checkboxPntr->drawRec.height/2 + actualFont->lineHeight/2 - actualFont->maxExtendDown
	);
	rs->DrawString(checkboxPntr->label, labelPos, textColor, 1.0f, Alignment_Left);
	
	rs->BindFont(originalFont);
}
