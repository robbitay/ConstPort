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
#include "appData.h"


//+================================================================+
//|                       Source Files                             |
//+================================================================+
#include "lineList.cpp"
#include "appFontHelpers.cpp"
#include "appLoadingFunctions.cpp"
#include "appRenderState.cpp"

inline void DrawLine(AppData_t* appData, const Line_t* line, v2 startPos)
{
	//TODO: Add cool formatting stuff?
	
	Color_t color = Color_Foreground;
	if (line->numChars > 0)
	{
		if (line->chars[0] == '@')
		{
			color = Color_Highlight1;
		}
		else if (line->chars[0] == '#')
		{
			color = Color_Highlight2;
		}
		else if (line->chars[0] == '!')
		{
			color = Color_Highlight3;
		}
	}
	
	appData->renderState.DrawString(line->chars, startPos, color, 1.0f);
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
	
	void* arenaBase = (void*)(appData+1);
	u32 arenaSize = AppMemory->permanantSize - sizeof(AppData_t);
	InitializeMemoryArenaHeap(&appData->memArena, arenaBase, arenaSize);
	
	InitializeRenderState(PlatformInfo, &appData->renderState);
	
	appData->simpleShader = LoadShader(PlatformInfo,
		"Resources/Shaders/simple-vertex.glsl",
		"Resources/Shaders/simple-fragment.glsl");
	
	appData->testTexture = LoadTexture(PlatformInfo,
		"Resources/Sprites/test.png");
	appData->scrollBarEndcapTexture = LoadTexture(PlatformInfo,
		"Resources/Sprites/scrollBarEndcap.png", false, false);
	
	appData->testFont = LoadFont(PlatformInfo,
		"Resources/Fonts/consola.ttf", 16,
		1024, 1024, ' ', 96);
	
	FileInfo_t testFile = PlatformInfo->ReadEntireFilePntr("test.txt");
	CreateLineList(&appData->lineList, &appData->memArena, "");//(const char*)testFile.content);
	PlatformInfo->FreeFileMemoryPntr(&testFile);
	
	DEBUG_WriteLine("Initialization Done!");
}

//+================================================================+
//|                         App Update                             |
//+================================================================+
AppUpdate_DEFINITION(App_Update)
{
	Gl_PlatformInfo = PlatformInfo;
	Gl_AppMemory = AppMemory;
	AppData_t* appData = (AppData_t*)AppMemory->permanantPntr;
	
	Color_t color1 = ColorFromHSV((i32)(PlatformInfo->programTime*180) % 360, 1.0f, 1.0f);
	Color_t color2 = ColorFromHSV((i32)(PlatformInfo->programTime*180 + 125) % 360, 1.0f, 1.0f);
	v2 mousePos = AppInput->mousePos;
	v2 mouseStartPos = AppInput->mouseStartPos[MouseButton_Left];
	v2 screenSize = NewVec2((r32)PlatformInfo->screenSize.x, (r32)PlatformInfo->screenSize.y);
	r32 fileHeight = (appData->lineList.numLines * (appData->testFont.lineHeight + 2));
	rec toolbarRec = NewRectangle(
		0, 
		screenSize.y-appData->testFont.lineHeight, 
		screenSize.x - (SCROLLBAR_WIDTH+SCROLLBAR_PADDING*2), 
		appData->testFont.lineHeight);
	rec scrollBarGutterRec = NewRectangle(
		screenSize.x - SCROLLBAR_WIDTH - SCROLLBAR_PADDING*2, 0, 
		SCROLLBAR_WIDTH + SCROLLBAR_PADDING*2, screenSize.y);
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
	rec gutterRec = NewRectangle(0, 0, gutterWidth, screenSize.y - toolbarRec.height);
	rec viewRec = NewRectangle(
		gutterWidth, 0,
		screenSize.x - gutterWidth - scrollBarGutterRec.width, 
		screenSize.y - toolbarRec.height);
	rec scrollBarRec = NewRectangle(
		scrollBarGutterRec.x + SCROLLBAR_PADDING,
		0,
		SCROLLBAR_WIDTH,
		scrollBarGutterRec.height * (viewRec.height / fileHeight));
	r32 maxScrollOffset = fileHeight - viewRec.height;
	r32 scrollPercent = appData->scrollOffset / maxScrollOffset;
	scrollBarRec.y = scrollBarGutterRec.y + (scrollBarGutterRec.height - scrollBarRec.height) * scrollPercent;
	
	if (AppInput->scrollDelta.y != 0)
	{
		appData->scrollOffset += -AppInput->scrollDelta.y * SCROLL_MULTIPLIER;
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
	
	if (AppInput->buttons[Button_Backspace].transCount > 0 && AppInput->buttons[Button_Backspace].isDown)
	{
		if (appData->comPort.isOpen)
		{
			PlatformInfo->CloseComPortPntr(&appData->comPort);
			DEBUG_WriteLine("Closed COM port");
		}
		
		appData->comPort = PlatformInfo->OpenComPortPntr("COM7",
			BaudRate_115200, false, true, Parity_None, 8, StopBits_1);
		
		if (appData->comPort.isOpen)
		{
			DEBUG_WriteLine("COM port opened successfully");
		}
		else
		{
			DEBUG_WriteLine("Couldn't open COM port.");
		}
	}
	
	if (appData->comPort.isOpen)
	{
		bool viewAtEnd = (appData->scrollOffset + viewRec.height) >= (fileHeight - appData->testFont.lineHeight);
		
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
							appData->scrollOffset += 19;//appData->testFont.lineHeight;
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
		appData->scrollOffset += AppInput->buttons[Button_Shift].isDown ? 16 : 5;
	}
	if (AppInput->buttons[Button_Up].isDown)
	{
		appData->scrollOffset -= AppInput->buttons[Button_Shift].isDown ? 16 : 5;
		if (appData->scrollOffset < 0)
			appData->scrollOffset = 0;
	}
	
	//Clear Console
	if (AppInput->buttons[Button_C].transCount > 0 && AppInput->buttons[Button_C].isDown &&
		AppInput->buttons[Button_Control].isDown && AppInput->buttons[Button_Shift].isDown)
	{
		DEBUG_WriteLine("Clearing Console");
		
		DestroyLineList(&appData->lineList);
		CreateLineList(&appData->lineList, &appData->memArena, "");
	}
	
	//Find available COM ports
	if (AppInput->buttons[Button_O].transCount > 0 && AppInput->buttons[Button_O].isDown &&
		AppInput->buttons[Button_Control].isDown)
	{
		appData->numComPortsAvailable = PlatformInfo->GetComPortListPntr(
			&appData->availableComPorts[0][0], 
			ArrayCount(appData->availableComPorts[0]), ArrayCount(appData->availableComPorts));
		
		DEBUG_PrintLine("Found %u com ports.", appData->numComPortsAvailable);
		for (u32 cIndex = 0; cIndex < appData->numComPortsAvailable; cIndex++)
		{
			DEBUG_PrintLine("[%u] \"%s\"", cIndex+1, appData->availableComPorts[cIndex]);
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
			if (appData->comPort.isOpen)
			{
				PlatformInfo->CloseComPortPntr(&appData->comPort);
				DEBUG_WriteLine("Closed COM port");
			}
			
			appData->comPort = PlatformInfo->OpenComPortPntr(appData->availableComPorts[cIndex],
				BaudRate_115200, false, true, Parity_None, 8, StopBits_1);
			
			if (appData->comPort.isOpen)
			{
				DEBUG_PrintLine("%s port opened successfully", appData->availableComPorts[cIndex]);
			}
			else
			{
				DEBUG_PrintLine("Couldn't open %s port.", appData->availableComPorts[cIndex]);
			}
		}
	}
		
	
	
	//Handle scrollbar interaction with mouse
	if (AppInput->buttons[MouseButton_Left].isDown)
	{
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
			}
		}
	}
	
	fileHeight = (appData->lineList.numLines * (appData->testFont.lineHeight + 2));
	maxScrollOffset = fileHeight - viewRec.height;
	if (appData->scrollOffset < 0)
			appData->scrollOffset = 0;
	if (appData->scrollOffset > maxScrollOffset)
		appData->scrollOffset = maxScrollOffset;
	scrollPercent = appData->scrollOffset / maxScrollOffset;
	scrollBarRec.y = scrollBarGutterRec.y + (scrollBarGutterRec.height - scrollBarRec.height) * scrollPercent;
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	glClearColor((Color_Background.r/255.f), (Color_Background.g/255.f), (Color_Background.b/255.f), 1.0f);
	// glClearColor((200/255.f), (200/255.f), (200/255.f), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	appData->renderState.BindShader(&appData->simpleShader);
	appData->renderState.BindFont(&appData->testFont);
	appData->renderState.SetGradientEnabled(false);
	
	Matrix4_t worldMatrix, viewMatrix, projMatrix;
	viewMatrix = Matrix4_Identity;
	projMatrix = Matrix4Scale(NewVec3(2.0f/PlatformInfo->screenSize.x, -2.0f/PlatformInfo->screenSize.y, 1.0f));
	projMatrix = Mat4Mult(projMatrix, Matrix4Translate(NewVec3(-PlatformInfo->screenSize.x/2.0f, -PlatformInfo->screenSize.y/2.0f, 0.0f)));
	appData->renderState.SetViewMatrix(viewMatrix);
	appData->renderState.SetProjectionMatrix(projMatrix);
	
	appData->renderState.DrawGradient(gutterRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	// appData->renderState.DrawGradient(NewRectangle(0, 0, 300, 300), color1, color2, Direction2D_Right);
	
	r32 lineHeight = appData->testFont.lineHeight + 2;
	i32 firstLine = max(0, (i32)((r32)appData->scrollOffset / lineHeight));
	i32 lastLine = min(appData->lineList.numLines, (i32)((r32)(appData->scrollOffset + viewRec.height) / lineHeight));
	{//Items drawn relative to view
		viewMatrix = Matrix4Translate(NewVec3(0, -appData->scrollOffset, 0));
		appData->renderState.SetViewMatrix(viewMatrix);
		
		v2 currentPos = NewVec2(gutterWidth + 2, appData->testFont.maxExtendUp);
		currentPos.y += firstLine * lineHeight;
		for (i32 lineIndex = firstLine; lineIndex < appData->lineList.numLines && lineIndex <= lastLine; lineIndex++)
		{
			Line_t* linePntr = GetLineAt(&appData->lineList, lineIndex);
			v2 lineSize = MeasureLine(&appData->testFont, linePntr);
			rec backRec = NewRectangle(currentPos.x, currentPos.y - appData->testFont.maxExtendUp, lineSize.x, appData->testFont.lineHeight);
			backRec = RectangleInflate(backRec, 1);
			// appData->renderState.DrawGradient(backRec, {0x80494949}, {0x80404040}, Direction2D_Down);
			
			appData->renderState.PrintString(NewVec2(0, currentPos.y), {Color_White}, 1.0f, "%u", lineIndex+1);
			
			DrawLine(appData, linePntr, currentPos);
			
			currentPos.y += lineHeight;
		}
		viewMatrix = Matrix4_Identity;
		appData->renderState.SetViewMatrix(viewMatrix);
	}
	
	appData->renderState.DrawGradient(toolbarRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	// appData->renderState.DrawGradient(NewRectangle(10, 10, 300, 300), color1, color2, Direction2D_Right);
	appData->renderState.PrintString( 
		NewVec2(0, screenSize.y-appData->testFont.maxExtendDown), {Color_White}, 1.0f, 
		"Heap: %u/%u used", appData->memArena.used, appData->memArena.size);
	// appData->renderState.PrintString( 
	// 	NewVec2(0, screenSize.y-appData->testFont.maxExtendDown), {Color_White}, 1.0f, 
	// 	"First: %d Last: %d", firstLine, lastLine);
	// PrintString(appData, appData->testFont, 
	// 	NewVec2(0, screenSize.y-appData->testFont.maxExtendDown), {Color_White}, 1.0f, 
	// 	"%u Lines Offset: %f (%fpx long)", appData->lineList.numLines, appData->scrollOffset, fileHeight);
	
	//Draw Scrollbar
	appData->renderState.DrawGradient(NewRectangle(scrollBarGutterRec.x - 8, 0, 8, screenSize.y), 
		{Color_TransparentBlack}, {Color_HalfTransparentBlack}, Direction2D_Right);
	appData->renderState.DrawGradient(scrollBarGutterRec, Color_Background, Color_UiGray3, Direction2D_Right);
	
	rec centerScrollBarRec = scrollBarRec;
	centerScrollBarRec.y += scrollBarRec.width;
	centerScrollBarRec.height -= 2 * scrollBarRec.width;
	rec startCapRec = NewRectangle(scrollBarRec.x, scrollBarRec.y, scrollBarRec.width, scrollBarRec.width);
	rec endCapRec = NewRectangle(scrollBarRec.x, scrollBarRec.y + scrollBarRec.height - scrollBarRec.width, scrollBarRec.width, scrollBarRec.width);
	endCapRec.y += endCapRec.height;
	endCapRec.height = -endCapRec.height;
	appData->renderState.DrawRectangle(RectangleInflate(centerScrollBarRec, 1), Color_UiGray4);
	appData->renderState.BindAlphaTexture(&appData->scrollBarEndcapTexture);
	appData->renderState.DrawRectangle(RectangleInflate(startCapRec, 1), Color_UiGray4);
	appData->renderState.DrawRectangle(RectangleInflate(endCapRec, 1), Color_UiGray4);
	
	appData->renderState.DrawGradient(startCapRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	appData->renderState.DrawGradient(endCapRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	appData->renderState.DisableAlphaTexture();
	appData->renderState.DrawGradient(centerScrollBarRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	
	// DrawTexture(appData, appData->glyphTexture, NewRectangle(10, 10, 500, 500), {Color_White});
	
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Left], AppInput->mouseMaxDist[MouseButton_Left], {Color_Red});
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Right], AppInput->mouseMaxDist[MouseButton_Right], {Color_Blue});
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Middle], AppInput->mouseMaxDist[MouseButton_Middle], {Color_Green});
	
	// appData->renderState.DrawRectangle(scrollBarGutterRec, {Color_Red});
	// appData->renderState.DrawRectangle(scrollBarRec, {Color_Blue});
	// appData->renderState.DrawRectangle(NewRectangle(0, 0, gutterWidth, screenSize.y), {Color_Orange});
	// appData->renderState.DrawRectangle(toolbarRec, {Color_Yellow});
	// appData->renderState.DrawRectangle(toolbarRec, {Color_Yellow});
}

//+================================================================+
//|                   App Get Sound Samples                        |
//+================================================================+
AppGetSoundSamples_DEFINITION(App_GetSoundSamples)
{
	Gl_PlatformInfo = PlatformInfo;
	Gl_AppMemory = AppMemory;
	AppData_t* appData = (AppData_t*)AppMemory->permanantPntr;
	
}

//+================================================================+
//|                        App Closing                             |
//+================================================================+
AppClosing_DEFINITION(App_Closing)
{
	Gl_PlatformInfo = PlatformInfo;
	Gl_AppMemory = AppMemory;
	AppData_t* appData = (AppData_t*)AppMemory->permanantPntr;
	
	DEBUG_WriteLine("Application closing!");
	
	
}