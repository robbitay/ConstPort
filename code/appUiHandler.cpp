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
		RenderScreenSize.y - ui->statusBarRec.height - (ui->mainMenuRec.y + ui->mainMenuRec.height)
	);
	ui->gotoEndButtonRec = NewRectangle(
		ui->scrollBarGutterRec.x,
		ui->scrollBarGutterRec.y + ui->scrollBarGutterRec.height,
		ui->scrollBarGutterRec.width,
		RenderScreenSize.y - ui->scrollBarGutterRec.y + ui->scrollBarGutterRec.height
	);
	ui->gutterRec = NewRectangle(
		0, 
		ui->mainMenuRec.y + ui->mainMenuRec.height, 
		(r32)GC->minGutterWidth,
		RenderScreenSize.y - ui->statusBarRec.height - (ui->mainMenuRec.y + ui->mainMenuRec.height)
	);
	if (GC->showLineNumbers)
	{
		ui->gutterRec.width = NumDecimalDigits(app->lineList.numLines) * MeasureString(&app->mainFont, " ", 1).x + 2;
		if (ui->gutterRec.width < (r32)GC->minGutterWidth) ui->gutterRec.width = (r32)GC->minGutterWidth;
	}
	ui->viewRec = NewRectangle(
		ui->gutterRec.width, 
		ui->mainMenuRec.y + ui->mainMenuRec.height,
		RenderScreenSize.x - ui->gutterRec.width - ui->scrollBarGutterRec.width, 
		RenderScreenSize.y - ui->statusBarRec.height - (ui->mainMenuRec.y + ui->mainMenuRec.height)
	);
	
	//Dynamic helpers
	ui->fileSize = MeasureLines(&app->lineList, &app->mainFont);
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
	
	//NOTE: Since MeasureLines also captures the position of the hoverLocation and scrollOffset of the first line that
	//		needs to be rendered it is dependant on the scrollOffset to be in it's proper position for these calculations
	//		Therefore we have to run it once more to update those locations correctly after min and max offset have been accounted for
	MeasureLines(&app->lineList, &app->mainFont);
	
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
