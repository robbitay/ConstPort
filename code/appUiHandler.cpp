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

void RecalculateUiElements(v2 mousePos, UiElements_t* ui)
{
	AppData_t* appData = GL_AppData;
	
	//Static sizing helpers
	ui->mousePos = mousePos;
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
	ui->fileHeight = (appData->lineList.numLines * ui->lineHeight);
	ui->maxScrollOffset = ui->fileHeight - ui->viewRec.height;
	if (ui->scrollOffsetGoto > ui->maxScrollOffset) ui->scrollOffsetGoto = ui->maxScrollOffset;
	if (ui->scrollOffsetGoto < 0) ui->scrollOffsetGoto = 0;
	if (ui->scrollOffset > ui->maxScrollOffset) ui->scrollOffset = ui->maxScrollOffset;
	if (ui->scrollOffset < 0) ui->scrollOffset = 0;
	ui->scrollPercent = ui->scrollOffsetGoto / ui->maxScrollOffset;
	ui->followingEndOfFile = (ui->scrollOffsetGoto + ui->viewRec.height) >= (ui->fileHeight - ui->lineHeight);
	
	//Scroll Bar
	ui->scrollBarRec = NewRectangle(
		ui->scrollBarGutterRec.x + SCROLLBAR_PADDING, 0,
		SCROLLBAR_WIDTH,
		ui->scrollBarGutterRec.height * (ui->viewRec.height / ui->fileHeight)
	);
	if (ui->scrollBarRec.height < MIN_SCROLLBAR_HEIGHT)
		ui->scrollBarRec.height = MIN_SCROLLBAR_HEIGHT;
	if (ui->scrollBarRec.height > ui->scrollBarGutterRec.height)
		ui->scrollBarRec.height = ui->scrollBarGutterRec.height;
	ui->scrollBarRec.y = ui->scrollBarGutterRec.y + (ui->scrollBarGutterRec.height - ui->scrollBarRec.height) * ui->scrollPercent;
}

void UpdateUiElements(const AppInput_t* AppInput, UiElements_t* ui)
{
	AppData_t* appData = GL_AppData;
	
	r32 gotoOffset = (ui->scrollOffsetGoto - ui->scrollOffset);
	if (Abs32(gotoOffset) < 5)
	{
		ui->scrollOffset = ui->scrollOffsetGoto;
	}
	else
	{
		ui->scrollOffset += gotoOffset / SCROLL_SPEED_DIVIDER;
	}
}
