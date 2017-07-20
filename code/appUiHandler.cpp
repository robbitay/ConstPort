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
	ui->buttonTextures[Button_ConsoleSettings] = LoadTexture("Resources/Sprites/buttonIcon1.png");
	ui->buttonTextures[Button_Settings] =        LoadTexture("Resources/Sprites/buttonIcon5.png");
	ui->buttonTextures[Button_Help] =            LoadTexture("Resources/Sprites/buttonIcon2.png");
}

void RecalculateUiElements(const AppInput_t* AppInput, UiElements_t* ui, bool resetFollowingEndOfFile)
{
	AppData_t* appData = GL_AppData;
	
	//Static sizing helpers
	ui->mousePos = AppInput->mousePos;
	ui->screenSize = NewVec2((r32)Gl_PlatformInfo->screenSize.x, (r32)Gl_PlatformInfo->screenSize.y);
	ui->lineHeight = appData->testFont.lineHeight + LINE_SPACING;
	
	//Static Rectangles
	ui->mainMenuRec = NewRectangle(
		0, 0,
		ui->screenSize.x, 
		MAIN_MENU_HEIGHT
	);
	for (u32 bIndex = 0; bIndex < NumMainMenuButtons; bIndex++)
	{
		r32 buttonSize = ui->mainMenuRec.height - MAIN_MENU_BUTTON_PADDING*2;
		ui->buttonRecs[bIndex] = NewRectangle(
			ui->mainMenuRec.x + MAIN_MENU_BUTTON_PADDING + (buttonSize+MAIN_MENU_BUTTON_PADDING) * bIndex,
			ui->mainMenuRec.y + MAIN_MENU_BUTTON_PADDING,
			buttonSize, buttonSize
		);
	}
	ui->rxLedRec = NewRectangle(
		ui->mainMenuRec.x + ui->mainMenuRec.width - RX_TX_LED_SIZE - 16,
		ui->mainMenuRec.y + 10,
		RX_TX_LED_SIZE, RX_TX_LED_SIZE
	);
	ui->txLedRec = NewRectangle(
		ui->rxLedRec.x - RX_TX_LED_SIZE - 16 - 4,
		ui->rxLedRec.y,
		RX_TX_LED_SIZE, RX_TX_LED_SIZE
	);
	ui->clearButtonRec = NewRectangle(
		ui->mainMenuRec.x + ui->mainMenuRec.width - CLEAR_BUTTON_WIDTH - MAIN_MENU_BUTTON_PADDING,
		ui->rxLedRec.y + ui->rxLedRec.height + 8 ,
		CLEAR_BUTTON_WIDTH, CLEAR_BUTTON_HEIGHT
	);
	ui->statusBarRec = NewRectangle(
		0, 
		ui->screenSize.y - appData->testFont.lineHeight, 
		ui->screenSize.x, 
		appData->testFont.lineHeight
	);
	ui->scrollBarGutterRec = NewRectangle(
		ui->screenSize.x - SCROLLBAR_WIDTH - SCROLLBAR_PADDING*2, 
		ui->mainMenuRec.y + ui->mainMenuRec.height, 
		SCROLLBAR_WIDTH + SCROLLBAR_PADDING*2, 
		ui->screenSize.y - ui->statusBarRec.height - (ui->mainMenuRec.y + ui->mainMenuRec.height)
	);
	ui->gutterRec = NewRectangle(
		0, 
		ui->mainMenuRec.y + ui->mainMenuRec.height, 
		NumDecimalDigits(appData->lineList.numLines) * MeasureString(&appData->testFont, " ", 1).x + 2,
		ui->screenSize.y - ui->statusBarRec.height - (ui->mainMenuRec.y + ui->mainMenuRec.height)
	);
	if (ui->gutterRec.width < MIN_GUTTER_WIDTH) ui->gutterRec.width = MIN_GUTTER_WIDTH;
	ui->viewRec = NewRectangle(
		ui->gutterRec.width, 
		ui->mainMenuRec.y + ui->mainMenuRec.height,
		ui->screenSize.x - ui->gutterRec.width - ui->scrollBarGutterRec.width, 
		ui->screenSize.y - ui->statusBarRec.height - (ui->mainMenuRec.y + ui->mainMenuRec.height)
	);
	
	//Dynamic helpers
	ui->fileSize = MeasureLines(AppInput, &appData->lineList, &appData->testFont);
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
		ui->scrollBarGutterRec.x + SCROLLBAR_PADDING, 0,
		SCROLLBAR_WIDTH,
		ui->scrollBarGutterRec.height * (ui->viewRec.height / ui->fileSize.y)
	);
	if (ui->scrollBarRec.height < MIN_SCROLLBAR_HEIGHT)
		ui->scrollBarRec.height = MIN_SCROLLBAR_HEIGHT;
	if (ui->scrollBarRec.height > ui->scrollBarGutterRec.height)
		ui->scrollBarRec.height = ui->scrollBarGutterRec.height;
	ui->scrollBarRec.y = ui->scrollBarGutterRec.y + (ui->scrollBarGutterRec.height - ui->scrollBarRec.height) * ui->scrollPercent.y;
	
	//NOTE: Since MeasureLines also captures the position of the hoverLocation and scrollOffset of the first line that
	//		needs to be rendered it is dependant on the scrollOffset to be in it's proper position for these calculations
	//		Therefore we have to run it once more to update those locations correctly after min and max offset have been accounted for
	MeasureLines(AppInput, &appData->lineList, &appData->testFont);
	
	// DEBUG_PrintLine("scrollOffset = (%f, %f)", ui->scrollOffset.x, ui->scrollOffset.y);
	// DEBUG_PrintLine("MaxScrollOffset = (%f, %f)", ui->maxScrollOffset.x, ui->maxScrollOffset.y);
}

void UpdateUiElements(const AppInput_t* AppInput, UiElements_t* ui)
{
	AppData_t* appData = GL_AppData;
	
	v2 gotoOffset = (ui->scrollOffsetGoto - ui->scrollOffset);
	
	if (Abs32(gotoOffset.x) < 1)
	{
		ui->scrollOffset.x = ui->scrollOffsetGoto.x;
	}
	else
	{
		ui->scrollOffset.x += gotoOffset.x / SCROLL_SPEED_DIVIDER;
	}
	
	if (Abs32(gotoOffset.y) < 1)
	{
		ui->scrollOffset.y = ui->scrollOffsetGoto.y;
	}
	else
	{
		ui->scrollOffset.y += gotoOffset.y / SCROLL_SPEED_DIVIDER;
	}
}
