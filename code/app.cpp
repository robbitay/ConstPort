/*
File:   app.cpp
Author: Taylor Robbins
Date:   06\06\2017
Description: 
	** Contains all the exported functions and #includes 
	** the rest of the source code files.
*/

#include "platformInterface.h"
#include "app_version.h"
#include "Colors.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "memoryArena.h"
#include "linkedList.h"

const PlatformInfo_t* Gl_PlatformInfo = nullptr;
const AppMemory_t*    Gl_AppMemory = nullptr;

#include "appHelpers.cpp"

//+================================================================+
//|                          Includes                              |
//+================================================================+
#include "appDefines.h"
#include "appStructs.h"
#include "lineList.h"
#include "appRenderState.h"
#include "appMenuHandler.h"
#include "appData.h"

AppData_t* GL_AppData = nullptr;

//+================================================================+
//|                       Source Files                             |
//+================================================================+
#include "appFontHelpers.cpp"
#include "lineList.cpp"
#include "appLoadingFunctions.cpp"
#include "appRenderState.cpp"
#include "appMenuHandler.cpp"

inline void DrawLine(AppData_t* appData, const Line_t* line, v2 startPos)
{
	//TODO: Add cool formatting stuff?
	
	Color_t color = Color_Foreground;
	if (line->numChars > 0)
	{
		if (line->chars[0] == 0x01)
		{
			color = Color_Highlight1;
		}
		else if (line->chars[0] == 0x02)
		{
			color = Color_Highlight2;
		}
		else if (line->chars[0] == 0x03)
		{
			color = Color_Highlight3;
		}
		else if (line->chars[0] == 0x04)
		{
			color = Color_Highlight4;
		}
	}
	
	appData->renderState.DrawString(line->chars, startPos, color, 1.0f);
}

void ClearConsole()
{
	AppData_t* appData = GL_AppData;
	
	DEBUG_WriteLine("Clearing Console");
	DestroyLineList(&appData->lineList);
	CreateLineList(&appData->lineList, &appData->memArena, "");
}

void RefreshComPortList()
{
	AppData_t* appData = GL_AppData;
	const PlatformInfo_t* PlatformInfo = Gl_PlatformInfo;
	
	appData->numComPortsAvailable = PlatformInfo->GetComPortListPntr(
		&appData->availableComPorts[0][0], 
		ArrayCount(appData->availableComPorts[0]), ArrayCount(appData->availableComPorts));
	
	DEBUG_PrintLine("Found %u com ports.", appData->numComPortsAvailable);
	for (u32 cIndex = 0; cIndex < appData->numComPortsAvailable; cIndex++)
	{
		DEBUG_PrintLine("[%u] \"%s\"", cIndex+1, appData->availableComPorts[cIndex]);
	}
}

void OpenComPort(i32 comPortIndex)
{
	AppData_t* appData = GL_AppData;
	const PlatformInfo_t* PlatformInfo = Gl_PlatformInfo;
	
	if (appData->comPort.isOpen)
	{
		PlatformInfo->CloseComPortPntr(&appData->comPort);
		DEBUG_WriteLine("Closed COM port");
	}
	
	ClearConsole();
	
	appData->comPort = PlatformInfo->OpenComPortPntr(appData->availableComPorts[comPortIndex],
		BaudRate_115200, false, true, Parity_None, 8, StopBits_1);
	
	if (appData->comPort.isOpen)
	{
		DEBUG_PrintLine("%s port opened successfully", appData->availableComPorts[comPortIndex]);
	}
	else
	{
		DEBUG_PrintLine("Couldn't open %s port.", appData->availableComPorts[comPortIndex]);
	}
}

void ComMenuUpdate(const PlatformInfo_t* PlatformInfo, const AppInput_t* AppInput, MenuHandler_t* menuHandler, Menu_t* menu)
{
	AppData_t* appData = GL_AppData;
	
	if (menu->show)
	{
		v2 currentPos = menu->usableRec.topLeft + NewVec2(5, 5 + appData->testFont.maxExtendUp);
		r32 maxLength = 0;
		
		for (u32 cIndex = 0; cIndex < appData->numComPortsAvailable; cIndex++)
		{
			// v2 stringSize = MeasureString(&appData->testFont, appData->availableComPorts[cIndex]);
			rec stringRec = NewRectangle(currentPos.x, currentPos.y - appData->testFont.maxExtendUp, 0, appData->testFont.lineHeight);
			// stringRec = RectangleInflate(stringRec, 1);
			stringRec.width = menu->usableRec.width - 5*2;
			if (stringRec.y + stringRec.height - menu->drawRec.y > maxLength)
				maxLength = stringRec.y + stringRec.height - menu->drawRec.y;
			
			if (!AppInput->buttons[MouseButton_Left].isDown && AppInput->buttons[MouseButton_Left].transCount > 0 &&
				AppInput->mouseMaxDist[MouseButton_Left] < 10)
			{
				if (IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], stringRec))
				{
					// DEBUG_PrintLine("Selected item %u", cIndex);
					OpenComPort(cIndex);
					menu->show = false;
					break;
				}
			}
			
			currentPos.y += appData->testFont.lineHeight + 5;
		}
		
		menu->drawRec.height = maxLength + 5;
	}
}
void ComMenuRender(const PlatformInfo_t* PlatformInfo, const AppInput_t* AppInput, RenderState_t* renderState, MenuHandler_t* menuHandler, Menu_t* menu)
{
	AppData_t* appData = GL_AppData;
	
	v2 currentPos = menu->usableRec.topLeft + NewVec2(5, 5 + appData->testFont.maxExtendUp);
	for (u32 cIndex = 0; cIndex < appData->numComPortsAvailable; cIndex++)
	{
		const char* comString = appData->availableComPorts[cIndex];
		v2 stringSize = MeasureString(&appData->testFont, comString);
		rec stringRec = NewRectangle(currentPos.x, currentPos.y - appData->testFont.maxExtendUp, 0, appData->testFont.lineHeight);
		stringRec = RectangleInflate(stringRec, 1);
		stringRec.width = menu->usableRec.width - 5*2;
		v2 stringOffset = NewVec2(stringRec.width/2 - stringSize.x/2, 0);
		
		Color_t buttonColor = {Color_White};
		Color_t borderColor {Color_Black};
		Color_t textColor = {Color_Black};
		
		if (IsInsideRectangle(AppInput->mousePos, stringRec))
		{
			if (AppInput->buttons[MouseButton_Left].isDown && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], stringRec))
			{
				buttonColor = Color_Highlight3;
				borderColor = Color_Foreground;
				textColor = Color_Foreground;
			}
			//TODO: Add a check to see which COM port is open and highlight it accordingly
			// else if ()
			else
			{
				buttonColor = Color_Highlight1;
			}
		}
		
		renderState->DrawButton(stringRec, buttonColor, borderColor);
		renderState->DrawString(comString, currentPos + stringOffset, textColor);
		
		currentPos.y += appData->testFont.lineHeight + 5;
	}
}

//+================================================================+
//|                       App Get Version                          |
//+================================================================+
AppGetVersion_DEFINITION(App_GetVersion)
{
	Version_t version = {
		APP_VERSION_MAJOR,
		APP_VERSION_MINOR,
		APP_VERSION_BUILD,
	};
	
	if (resetApplication != nullptr)
	{
		*resetApplication = false;
	}
	
	return version;
}

//+================================================================+
//|                       App Initialize                           |
//+================================================================+
AppInitialize_DEFINITION(App_Initialize)
{
	Gl_PlatformInfo = PlatformInfo;
	Gl_AppMemory = AppMemory;
	DEBUG_WriteLine("Initializing Game...");
	
	AppData_t* appData = (AppData_t*)AppMemory->permanantPntr;
	ClearPointer(appData);
	GL_AppData = appData;
	
	void* arenaBase = (void*)(appData+1);
	u32 arenaSize = AppMemory->permanantSize - sizeof(AppData_t);
	InitializeMemoryArenaHeap(&appData->memArena, arenaBase, arenaSize);
	
	InitializeRenderState(PlatformInfo, &appData->renderState);
	InitializeMenuHandler(&appData->menuHandler, &appData->memArena);
	
	v2i screenSize = PlatformInfo->screenSize;
	Menu_t* comMenu = AddMenu(&appData->menuHandler, "COM Menu", NewRectangle((r32)screenSize.x / 2 - 50, (r32)screenSize.y / 2 - 150, 100, 300),
		ComMenuUpdate, ComMenuRender);
	comMenu->show = false;
	
	appData->simpleShader = LoadShader(
		"Resources/Shaders/simple-vertex.glsl",
		"Resources/Shaders/simple-fragment.glsl");
	appData->outlineShader = LoadShader(
		"Resources/Shaders/outline-vertex.glsl",
		"Resources/Shaders/outline-fragment.glsl");
	
	appData->testTexture = LoadTexture("Resources/Sprites/buttonIcon3.png");
	appData->scrollBarEndcapTexture = LoadTexture("Resources/Sprites/scrollBarEndcap.png", false, false);
	
	appData->testFont = LoadFont("Resources/Fonts/consola.ttf", 
		16, 1024, 1024, ' ', 96);
	
	appData->frameTexture = CreateTexture(nullptr, 2048, 2048);
	appData->frameBuffer = CreateFrameBuffer(&appData->frameTexture);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		DEBUG_WriteLine("FrameBuffer incomplete!");
	}
	
	FileInfo_t testFile = PlatformInfo->ReadEntireFilePntr("test.txt");
	CreateLineList(&appData->lineList, &appData->memArena, "");//(const char*)testFile.content);
	PlatformInfo->FreeFileMemoryPntr(&testFile);
	
	RefreshComPortList();
	
	DEBUG_WriteLine("Initialization Done!");
}

//+================================================================+
//|                        App Reloaded                            |
//+================================================================+
AppReloaded_DEFINITION(App_Reloaded)
{
	Gl_PlatformInfo = PlatformInfo;
	Gl_AppMemory = AppMemory;
	AppData_t* appData = (AppData_t*)AppMemory->permanantPntr;
	GL_AppData = appData;
	
	DEBUG_WriteLine("App Reloaded");
	
	//Make sure our callbacks still match the location of the functions in the new DLL
	Menu_t* menuPntr = GetMenuByName(&appData->menuHandler, "COM Menu");
	menuPntr->specialPntr = nullptr;
	menuPntr->updateFunctionPntr = ComMenuUpdate;
	menuPntr->renderFunctionPntr = ComMenuRender;
}

void DrawThings(const AppInput_t* AppInput)
{
	AppData_t* appData = GL_AppData;
	RenderState_t* rs = &appData->renderState;
	
	rs->BindTexture(&appData->testTexture);
	rs->DrawTexturedRec(NewRectangle(10, 10, 500, 500), {Color_White});
	rs->DrawTexturedRec(NewRectangle(90, 70, 500, 500), {Color_White});
	rs->DrawTexturedRec(NewRectangle(AppInput->mousePos.x - 250, AppInput->mousePos.y - 250, 500, 500), {Color_White});
}

//+================================================================+
//|                         App Update                             |
//+================================================================+
AppUpdate_DEFINITION(App_Update)
{
	Gl_PlatformInfo = PlatformInfo;
	Gl_AppMemory = AppMemory;
	AppData_t* appData = (AppData_t*)AppMemory->permanantPntr;
	GL_AppData = appData;
	
	Menu_t* comMenu = GetMenuByName(&appData->menuHandler, "COM Menu");
	Color_t color1 = ColorFromHSV((i32)(PlatformInfo->programTime*180) % 360, 1.0f, 1.0f);
	Color_t color2 = ColorFromHSV((i32)(PlatformInfo->programTime*180 + 125) % 360, 1.0f, 1.0f);
	v2 mousePos = AppInput->mousePos;
	v2 mouseStartPos = AppInput->mouseStartPos[MouseButton_Left];
	v2 screenSize = NewVec2((r32)PlatformInfo->screenSize.x, (r32)PlatformInfo->screenSize.y);
	r32 fileHeight = (appData->lineList.numLines * (appData->testFont.lineHeight + 2));
	rec toolbarRec = NewRectangle(
		0, 
		screenSize.y-appData->testFont.lineHeight, 
		screenSize.x, 
		appData->testFont.lineHeight);
	rec scrollBarGutterRec = NewRectangle(
		screenSize.x - SCROLLBAR_WIDTH - SCROLLBAR_PADDING*2, 
		0, 
		SCROLLBAR_WIDTH + SCROLLBAR_PADDING*2, 
		screenSize.y - toolbarRec.height);
	char gutterNumberBuffer[10] = {};
	r32 gutterWidth = 0;
	for (i32 lineIndex = 0; lineIndex < appData->lineList.numLines; lineIndex++)
	{
		ClearArray(gutterNumberBuffer);
		snprintf(gutterNumberBuffer, ArrayCount(gutterNumberBuffer)-1, "%u", lineIndex+1);
		v2 stringSize = MeasureString(&appData->testFont, gutterNumberBuffer);
		if (stringSize.x > gutterWidth)
		{
			gutterWidth = stringSize.x;
		}
	}
	gutterWidth += 2;
	if (gutterWidth < MIN_GUTTER_WIDTH)
		gutterWidth = MIN_GUTTER_WIDTH;
	rec gutterRec = NewRectangle(0, 0, gutterWidth, screenSize.y - toolbarRec.height);
	rec viewRec = NewRectangle(
		gutterWidth, 0,
		screenSize.x - gutterWidth - scrollBarGutterRec.width, 
		screenSize.y - toolbarRec.height);
	TextLocation_t hoverLocation = PointToTextLocation(&appData->lineList, &appData->testFont, 
		mousePos - NewVec2(LINE_SPACING + gutterWidth, -appData->scrollOffset));
	rec scrollBarRec = NewRectangle(
		scrollBarGutterRec.x + SCROLLBAR_PADDING,
		0,
		SCROLLBAR_WIDTH,
		scrollBarGutterRec.height * (viewRec.height / fileHeight));
	if (scrollBarRec.height < MIN_SCROLLBAR_HEIGHT)
		scrollBarRec.height = MIN_SCROLLBAR_HEIGHT;
	if (scrollBarRec.height > scrollBarGutterRec.height)
		scrollBarRec.height = scrollBarGutterRec.height;
	r32 maxScrollOffset = fileHeight - viewRec.height;
	r32 scrollPercent = appData->scrollOffsetGoto / maxScrollOffset;
	scrollBarRec.y = scrollBarGutterRec.y + (scrollBarGutterRec.height - scrollBarRec.height) * scrollPercent;
	
	if (AppInput->scrollDelta.y != 0)
	{
		appData->scrollOffsetGoto += -AppInput->scrollDelta.y * SCROLL_MULTIPLIER;
	}
	
	#if 0
	if (AppInput->buttons[Button_Enter].transCount > 0 && AppInput->buttons[Button_Enter].isDown)
	{
		FileInfo_t testFile = PlatformInfo->ReadEntireFilePntr("test.txt");
		DestroyLineList(&appData->lineList);
		CreateLineList(&appData->lineList, &appData->memArena, (const char*)testFile.content);
		PlatformInfo->FreeFileMemoryPntr(&testFile);
	}
	#endif
	
	if (appData->comPort.isOpen)
	{
		bool viewAtEnd = (appData->scrollOffsetGoto + viewRec.height) >= (fileHeight - appData->testFont.lineHeight);
		
		i32 readResult = 1;
		while (readResult > 0)
		{
			char buffer[4096] = {};
			readResult = PlatformInfo->ReadComPortPntr(&appData->comPort, buffer, ArrayCount(buffer)-1);
			if (readResult > 0)
			{
				// DEBUG_PrintLine("Read %d bytes \"%.*s\"", readResult, readResult, buffer);
				
				Line_t* lastLine = GetLastLine(&appData->lineList);
				
				for (i32 cIndex = 0; cIndex < readResult; cIndex++)
				{
					char newChar = buffer[cIndex];
					if (newChar == '\n')
					{
						Line_t* finishedLine = lastLine;
						if (finishedLine->numChars >= 4 && strncmp(finishedLine->chars, "lib:", 4) == 0)
						{
							finishedLine->color = Color_Highlight1;
						}
						
						lastLine = AddLineToList(&appData->lineList, "");
						lastLine->color = Color_Foreground;
						if (viewAtEnd)
						{
							appData->scrollOffsetGoto += 19;//appData->testFont.lineHeight;
						}
					}
					else
					{
						LineAppend(&appData->lineList, lastLine, newChar);
					}
				}
				
				// LineAppend(&appData->lineList, lastLine, ']');
				// LineAppend(&appData->lineList, lastLine, ']');
			}
			else if (readResult < 0)
			{
				DEBUG_PrintLine("COM port read Error!: %d", readResult);
			}
		}
		
		if (AppInput->textInputLength > 0)
		{
			DEBUG_PrintLine("Writing \"%.*s\"", AppInput->textInputLength, AppInput->textInput);
			
			PlatformInfo->WriteComPortPntr(&appData->comPort, &AppInput->textInput[0], AppInput->textInputLength);
		}
		
		if (AppInput->buttons[Button_Enter].transCount > 0 && AppInput->buttons[Button_Enter].isDown)
		{
			DEBUG_WriteLine("Writing New Line");
			
			char newChar = '\n';
			PlatformInfo->WriteComPortPntr(&appData->comPort, &newChar, 1);
		}
	}
	
	if (AppInput->buttons[Button_Down].isDown)
	{
		appData->scrollOffsetGoto += AppInput->buttons[Button_Shift].isDown ? 16 : 5;
	}
	if (AppInput->buttons[Button_Up].isDown)
	{
		appData->scrollOffsetGoto -= AppInput->buttons[Button_Shift].isDown ? 16 : 5;
	}
	
	//Clear Console
	if (AppInput->buttons[Button_C].transCount > 0 && AppInput->buttons[Button_C].isDown &&
		AppInput->buttons[Button_Control].isDown && AppInput->buttons[Button_Shift].isDown)
	{
		ClearConsole();
	}
	
	//Find available COM ports
	if (AppInput->buttons[Button_O].transCount > 0 && AppInput->buttons[Button_O].isDown &&
		AppInput->buttons[Button_Control].isDown)
	{
		Assert(comMenu != nullptr);
		
		if (comMenu->show)
		{
			comMenu->show = false;
		}
		else
		{
			comMenu->show = true;
			
			RefreshComPortList();
		}
	}
	
	//Quick keys for opening COM ports
	if (AppInput->buttons[Button_Control].isDown)
	{
		i32 cIndex = -1;
		
		if (AppInput->buttons[Button_1].transCount > 0 && AppInput->buttons[Button_1].isDown) cIndex = 0;
		if (AppInput->buttons[Button_2].transCount > 0 && AppInput->buttons[Button_2].isDown) cIndex = 1;
		if (AppInput->buttons[Button_3].transCount > 0 && AppInput->buttons[Button_3].isDown) cIndex = 2;
		if (AppInput->buttons[Button_4].transCount > 0 && AppInput->buttons[Button_4].isDown) cIndex = 3;
		if (AppInput->buttons[Button_5].transCount > 0 && AppInput->buttons[Button_5].isDown) cIndex = 4;
		if (AppInput->buttons[Button_6].transCount > 0 && AppInput->buttons[Button_6].isDown) cIndex = 5;
		if (AppInput->buttons[Button_7].transCount > 0 && AppInput->buttons[Button_7].isDown) cIndex = 6;
		if (AppInput->buttons[Button_8].transCount > 0 && AppInput->buttons[Button_8].isDown) cIndex = 7;
		if (AppInput->buttons[Button_9].transCount > 0 && AppInput->buttons[Button_9].isDown) cIndex = 8;
		
		if (cIndex >= (i32)appData->numComPortsAvailable)
		{
			DEBUG_PrintLine("Cannot open COM port #%d", cIndex+1);
		}
		else if (cIndex != -1)
		{
			OpenComPort(cIndex);
		}
	}
	
	if (PlatformInfo->windowResized)
	{
		comMenu->drawRec.topLeft = NewVec2(
			PlatformInfo->screenSize.x / 2 - comMenu->drawRec.width/2,
			PlatformInfo->screenSize.y / 2 - comMenu->drawRec.height/2);
	}
		
	MenuHandlerUpdate(PlatformInfo, AppInput, &appData->menuHandler);
	
	if (AppInput->buttons[MouseButton_Left].isDown)
	{
		//Handle scrollbar interaction with mouse
		if (IsInsideRectangle(mouseStartPos, scrollBarGutterRec))
		{
			if (AppInput->buttons[MouseButton_Left].transCount > 0)//Pressed the button down
			{
				appData->mouseScrollbarOffset = mousePos.y - scrollBarRec.y;
				if (IsInsideRectangle(mousePos, scrollBarRec))
				{
					appData->startedOnScrollbar = true;
				}
				else 
				{
					appData->startedOnScrollbar = false;
					if (appData->mouseScrollbarOffset > 0)
					{
						appData->scrollOffset += viewRec.height;
					}
					else
					{
						appData->scrollOffset -= viewRec.height;
					}
					appData->scrollOffsetGoto = appData->scrollOffset;
				}
			}
			else if (appData->startedOnScrollbar) //holding the button
			{
				r32 newPixelLocation = mousePos.y - appData->mouseScrollbarOffset;
				if (newPixelLocation > scrollBarGutterRec.y + (scrollBarGutterRec.height - scrollBarRec.height))
				{
					newPixelLocation = scrollBarGutterRec.y + (scrollBarGutterRec.height - scrollBarRec.height);
				}
				if (newPixelLocation < 0)
				{
					newPixelLocation = 0;
				}
				
				appData->scrollOffset = (newPixelLocation / (scrollBarGutterRec.height - scrollBarRec.height)) * maxScrollOffset;
				appData->scrollOffsetGoto = appData->scrollOffset;
			}
		}
		else if (IsInsideRectangle(mouseStartPos, viewRec))
		{
			if (AppInput->buttons[MouseButton_Left].transCount > 0)//Pressed the button down
			{
				appData->selectionStart = hoverLocation;
				appData->selectionEnd = hoverLocation;
			}
			else if (IsInsideRectangle(mousePos, viewRec)) //Mouse Button Holding
			{
				appData->selectionEnd = hoverLocation;
			}
		}
	}
	
	fileHeight = (appData->lineList.numLines * (appData->testFont.lineHeight + 2));
	maxScrollOffset = fileHeight - viewRec.height;
	if (appData->scrollOffsetGoto > maxScrollOffset)
		appData->scrollOffsetGoto = maxScrollOffset;
	if (appData->scrollOffsetGoto < 0)
			appData->scrollOffsetGoto = 0;
	if (appData->scrollOffset > maxScrollOffset)
		appData->scrollOffset = maxScrollOffset;
	if (appData->scrollOffset < 0)
			appData->scrollOffset = 0;
	
	r32 gotoOffset = (appData->scrollOffsetGoto - appData->scrollOffset);
	if (Abs32(gotoOffset) < 5)
	{
		appData->scrollOffset = appData->scrollOffsetGoto;
	}
	else
	{
		appData->scrollOffset += gotoOffset / SCROLL_SPEED_DIVIDER;
	}
	
	scrollPercent = appData->scrollOffsetGoto / maxScrollOffset;
	scrollBarRec.y = scrollBarGutterRec.y + (scrollBarGutterRec.height - scrollBarRec.height) * scrollPercent;
	
	//+--------------------------------------+
	//|           Rendering Setup            |
	//+--------------------------------------+
	RenderState_t* rs = &appData->renderState;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	rs->SetViewport(NewRectangle(0, 0, screenSize.x, screenSize.y));
	
	glClearColor((Color_Background.r/255.f), (Color_Background.g/255.f), (Color_Background.b/255.f), 1.0f);
	// glClearColor((200/255.f), (200/255.f), (200/255.f), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	rs->BindFrameBuffer(nullptr);
	rs->BindShader(&appData->simpleShader);
	rs->BindFont(&appData->testFont);
	rs->SetGradientEnabled(false);
	
	Matrix4_t worldMatrix, viewMatrix, projMatrix;
	viewMatrix = Matrix4_Identity;
	projMatrix = Matrix4Scale(NewVec3(2.0f/PlatformInfo->screenSize.x, -2.0f/PlatformInfo->screenSize.y, 1.0f));
	projMatrix = Mat4Mult(projMatrix, Matrix4Translate(NewVec3(-PlatformInfo->screenSize.x/2.0f, -PlatformInfo->screenSize.y/2.0f, 0.0f)));
	rs->SetViewMatrix(viewMatrix);
	rs->SetProjectionMatrix(projMatrix);
	
	rs->DrawGradient(gutterRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	// rs->DrawGradient(NewRectangle(0, 0, 300, 300), color1, color2, Direction2D_Right);
	
	//+--------------------------------------+
	//|            Render Lines              |
	//+--------------------------------------+
	{
		r32 lineHeight = appData->testFont.lineHeight + LINE_SPACING;
		i32 firstLine = max(0, (i32)((r32)appData->scrollOffset / lineHeight));
		i32 lastLine = min(appData->lineList.numLines, (i32)((r32)(appData->scrollOffset + viewRec.height) / lineHeight));
		
		rs->SetViewMatrix(Matrix4Translate(NewVec3(0, -appData->scrollOffset, 0)));
		{//Items drawn relative to view
			
			v2 currentPos = NewVec2(gutterWidth + LINE_SPACING, firstLine * lineHeight + appData->testFont.maxExtendUp);
			for (i32 lineIndex = firstLine; lineIndex < appData->lineList.numLines && lineIndex <= lastLine; lineIndex++)
			{
				Line_t* linePntr = GetLineAt(&appData->lineList, lineIndex);
				v2 lineSize = MeasureLine(&appData->testFont, linePntr);
				rec backRec = NewRectangle(currentPos.x, currentPos.y - appData->testFont.maxExtendUp, lineSize.x, appData->testFont.lineHeight);
				backRec = RectangleInflate(backRec, 1);
				// rs->DrawGradient(backRec, {0x80494949}, {0x80404040}, Direction2D_Down);
				
				rs->PrintString(NewVec2(0, currentPos.y), {Color_White}, 1.0f, "%u", lineIndex+1);
				
				DrawLine(appData, linePntr, currentPos);
				
				if (lineIndex == hoverLocation.lineNum)
				{
					v2 skipSize = MeasureString(&appData->testFont, linePntr->chars, hoverLocation.charIndex);
					rec cursorRec = backRec;
					cursorRec.x += skipSize.x;
					cursorRec.width = 1;
					rs->DrawGradient(cursorRec, color1, color2, Direction2D_Down);
				}
				
				currentPos.y += lineHeight;
			}
			
		}
		rs->SetViewMatrix(Matrix4_Identity);
	}
	
	//+--------------------------------------+
	//|         Render UI Elements           |
	//+--------------------------------------+
	rs->DrawGradient(toolbarRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	// rs->DrawGradient(NewRectangle(10, 10, 300, 300), color1, color2, Direction2D_Right);
	// rs->PrintString( 
	// 	NewVec2(0, screenSize.y-appData->testFont.maxExtendDown), {Color_White}, 1.0f, 
	// 	"Heap: %u/%u used", appData->memArena.used, appData->memArena.size);
	rs->PrintString( 
		NewVec2(0, screenSize.y-appData->testFont.maxExtendDown), {Color_White}, 1.0f, 
		"Hover Location: Line %d Char %d", hoverLocation.lineNum+1, hoverLocation.charIndex);
	// rs->PrintString( 
	// 	NewVec2(0, screenSize.y-appData->testFont.maxExtendDown), {Color_White}, 1.0f, 
	// 	"First: %d Last: %d", firstLine, lastLine);
	// PrintString(appData, appData->testFont, 
	// 	NewVec2(0, screenSize.y-appData->testFont.maxExtendDown), {Color_White}, 1.0f, 
	// 	"%u Lines Offset: %f (%fpx long)", appData->lineList.numLines, appData->scrollOffset, fileHeight);
	
	//Draw Scrollbar
	rs->DrawGradient(NewRectangle(scrollBarGutterRec.x - 8, scrollBarGutterRec.y, 8, scrollBarGutterRec.height), 
		{Color_TransparentBlack}, {Color_HalfTransparentBlack}, Direction2D_Right);
	rs->DrawGradient(scrollBarGutterRec, Color_Background, Color_UiGray3, Direction2D_Right);
	
	rec centerScrollBarRec = scrollBarRec;
	centerScrollBarRec.y += scrollBarRec.width;
	centerScrollBarRec.height -= 2 * scrollBarRec.width;
	rec startCapRec = NewRectangle(scrollBarRec.x, scrollBarRec.y, scrollBarRec.width, scrollBarRec.width);
	rec endCapRec = NewRectangle(scrollBarRec.x, scrollBarRec.y + scrollBarRec.height - scrollBarRec.width, scrollBarRec.width, scrollBarRec.width);
	endCapRec.y += endCapRec.height;
	endCapRec.height = -endCapRec.height;
	rs->DrawRectangle(RectangleInflate(centerScrollBarRec, 1), Color_UiGray4);
	rs->BindAlphaTexture(&appData->scrollBarEndcapTexture);
	rs->DrawRectangle(RectangleInflate(startCapRec, 1), Color_UiGray4);
	rs->DrawRectangle(RectangleInflate(endCapRec, 1), Color_UiGray4);
	
	rs->DrawGradient(startCapRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	rs->DrawGradient(endCapRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	rs->DisableAlphaTexture();
	rs->DrawGradient(centerScrollBarRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	
	MenuHandlerDrawMenus(PlatformInfo, AppInput, &appData->renderState, &appData->menuHandler);
	
	// DrawThings();
	
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Left], AppInput->mouseMaxDist[MouseButton_Left], {Color_Red});
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Right], AppInput->mouseMaxDist[MouseButton_Right], {Color_Blue});
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Middle], AppInput->mouseMaxDist[MouseButton_Middle], {Color_Green});
	
	// rs->DrawRectangle(toolbarRec, {Color_Yellow});
	// rs->DrawRectangle(scrollBarGutterRec, {Color_Red});
	// rs->DrawRectangle(scrollBarRec, {Color_Blue});
	// rs->DrawRectangle(NewRectangle(0, 0, gutterWidth, screenSize.y - toolbarRec.height), {Color_Orange});
	// rs->DrawRectangle(toolbarRec, {Color_Yellow});
	
	rs->BindFrameBuffer(&appData->frameBuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//+--------------------------------------+
	//|          Render Selection            |
	//+--------------------------------------+
	if (appData->selectionStart.lineNum != appData->selectionEnd.lineNum ||
		appData->selectionStart.charIndex != appData->selectionEnd.charIndex)
	{
		TextLocation_t minLocation = TextLocationMin(appData->selectionStart, appData->selectionEnd);
		TextLocation_t maxLocation = TextLocationMax(appData->selectionStart, appData->selectionEnd); 
		r32 lineHeight = appData->testFont.lineHeight + LINE_SPACING;
		i32 firstLine = max(0, max(minLocation.lineNum, (i32)((r32)appData->scrollOffset / lineHeight)));
		i32 lastLine = min(appData->lineList.numLines, min(maxLocation.lineNum, (i32)((r32)(appData->scrollOffset + viewRec.height) / lineHeight)));
		
		rs->SetViewMatrix(Matrix4Translate(NewVec3(0, -appData->scrollOffset, 0)));
		{//Items drawn relative to view
			
			v2 currentPos = NewVec2(gutterWidth + LINE_SPACING, firstLine * lineHeight + appData->testFont.maxExtendUp);
			for (i32 lineIndex = firstLine; lineIndex < appData->lineList.numLines && lineIndex <= lastLine; lineIndex++)
			{
				Line_t* linePntr = GetLineAt(&appData->lineList, lineIndex);
				v2 skipSize = Vec2_Zero;
				v2 selectionSize = Vec2_Zero;
				if (lineIndex == minLocation.lineNum &&
					lineIndex == maxLocation.lineNum)
				{
					skipSize = MeasureString(&appData->testFont, linePntr->chars, minLocation.charIndex);
					selectionSize = MeasureString(&appData->testFont, &linePntr->chars[minLocation.charIndex], maxLocation.charIndex - minLocation.charIndex);
				}
				else if (lineIndex == minLocation.lineNum)
				{
					skipSize = MeasureString(&appData->testFont, linePntr->chars, minLocation.charIndex);
					selectionSize = MeasureString(&appData->testFont, &linePntr->chars[minLocation.charIndex]);
					// selectionSize.x += MeasureString(&appData->testFont, " ", 1).x;
				}
				else if (lineIndex == maxLocation.lineNum)
				{
					selectionSize = MeasureString(&appData->testFont, linePntr->chars, maxLocation.charIndex);
				}
				else
				{
					Assert(lineIndex > minLocation.lineNum && lineIndex < maxLocation.lineNum);
					
					selectionSize = MeasureString(&appData->testFont, linePntr->chars);
					// selectionSize.x += MeasureString(&appData->testFont, " ", 1).x;
				}
				
				rec backRec = NewRectangle(currentPos.x + skipSize.x, currentPos.y - appData->testFont.maxExtendUp, selectionSize.x, appData->testFont.lineHeight);
				backRec = RectangleInflate(backRec, LINE_SPACING);
				rs->DrawRectangle(backRec, color1);
				
				currentPos.y += lineHeight;
			}
			
		}
		rs->SetViewMatrix(Matrix4_Identity);
	}
	
	//+--------------------------------------+
	//|           Post Processing            |
	//+--------------------------------------+
	rs->BindFrameBuffer(nullptr);
	rs->BindShader(&appData->outlineShader);
	rs->UpdateShader();
	
	rs->SetSecondaryColor(NewColor(0, 0, 0, 20));
	rs->BindTexture(&appData->frameTexture);
	rs->DrawTexturedRec(NewRectangle(0, screenSize.y, (r32)appData->frameTexture.width, (r32)-appData->frameTexture.height), {Color_White});
}

//+================================================================+
//|                   App Get Sound Samples                        |
//+================================================================+
AppGetSoundSamples_DEFINITION(App_GetSoundSamples)
{
	Gl_PlatformInfo = PlatformInfo;
	Gl_AppMemory = AppMemory;
	AppData_t* appData = (AppData_t*)AppMemory->permanantPntr;
	GL_AppData = appData;
	
	
}

//+================================================================+
//|                        App Closing                             |
//+================================================================+
AppClosing_DEFINITION(App_Closing)
{
	Gl_PlatformInfo = PlatformInfo;
	Gl_AppMemory = AppMemory;
	AppData_t* appData = (AppData_t*)AppMemory->permanantPntr;
	GL_AppData = appData;
	
	DEBUG_WriteLine("Application closing!");
	
	
}