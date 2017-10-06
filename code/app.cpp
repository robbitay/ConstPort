/*
File:   app.cpp
Author: Taylor Robbins
Date:   06\06\2017
Description:
	** Contains all the exported functions and #includes
	** the rest of the source code files.
*/


#include <stdarg.h>
#include "my_assert.h"
#include "platformInterface.h"
#include "app_version.h"
#include "colors.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "memoryArena.h"
#include "tempMemory.h"
#include "tempMemory.cpp"
#include "linkedList.h"
#include "easing.h"
#include "charClasses.h"

const PlatformInfo_t* Gl_PlatformInfo = nullptr;
const    AppMemory_t* Gl_AppMemory    = nullptr;

#include "appHelpers.cpp"

//+================================================================+
//|                          Includes                              |
//+================================================================+
#include "appDefines.h"
#include "appStructs.h"
#include "lineList.h"
#include "appRenderState.h"
#include "appMenuHandler.h"
#include "appUiHandler.h"
#include "appRegularExpressions.h"
#include "appConfiguration.h"
#include "appData.h"

AppData_t*      GL_AppData = nullptr;
GlobalConfig_t* GC         = nullptr;

u32 GetElapsedString(u64 timespan, char* outputBuffer, u32 outputBufferSize)
{
	u32 result = 0;
	
	u32 numDays = (u32)(timespan/(60*60*24));
	u32 numHours = (u32)(timespan/(60*60)) - (numDays*24);
	u32 numMinutes = (u32)(timespan/60) - (numDays*60*24) - (numHours*60);
	u32 numSeconds = (u32)(timespan) - (numDays*60*60*24) - (numHours*60*60) - (numMinutes*60);
	if (numDays > 0)
	{
		result = snprintf(outputBuffer, outputBufferSize-1,
			"%ud %uh %um %us", numDays, numHours, numMinutes, numSeconds);
	}
	else if (numHours > 0)
	{
		result = snprintf(outputBuffer, outputBufferSize-1,
			"%uh %um %us", numHours, numMinutes, numSeconds);
	}
	else if (numMinutes > 0)
	{
		result = snprintf(outputBuffer, outputBufferSize-1,
			"%um %us", numMinutes, numSeconds);
	}
	else
	{
		result = snprintf(outputBuffer, outputBufferSize-1,
			"%us", numSeconds);
	}
	
	return result;
}

void StatusMessage(const char* functionName, StatusMessage_t messageType, const char* formatString, ...)
{
	AppData_t* appData = GL_AppData;
	const PlatformInfo_t* PlatformInfo = Gl_PlatformInfo;
	
	ClearArray(appData->statusMessage);
	va_list args;
	va_start(args, formatString);
	size_t length = vsnprintf(appData->statusMessage, ArrayCount(appData->statusMessage), formatString, args);
	appData->statusMessage[ArrayCount(appData->statusMessage)-1] = '\0';
	va_end(args);
	appData->statusMessageType = messageType;
	appData->statusMessageTime = PlatformInfo->localTime;
	
	DEBUG_PrintLine("[%s]: %s", functionName, appData->statusMessage);
}

#define StatusDebug(formatString, ...)   StatusMessage(__func__, StatusMessage_Debug,   formatString, ##__VA_ARGS__)
#define StatusInfo(formatString, ...)    StatusMessage(__func__, StatusMessage_Info,    formatString, ##__VA_ARGS__)
#define StatusSuccess(formatString, ...) StatusMessage(__func__, StatusMessage_Success, formatString, ##__VA_ARGS__)
#define StatusError(formatString, ...)   StatusMessage(__func__, StatusMessage_Error,   formatString, ##__VA_ARGS__)

//+================================================================+
//|                       Source Files                             |
//+================================================================+
#include "appConfiguration.cpp"
#include "appFontHelpers.cpp"
#include "lineList.cpp"
#include "appLoadingFunctions.cpp"
#include "appRenderState.cpp"
#include "appMenuHandler.cpp"
#include "appRenderLine.cpp"
#include "appUiHandler.cpp"
#include "appRegularExpressions.cpp"

void ClearConsole()
{
	AppData_t* appData = GL_AppData;
	
	DEBUG_WriteLine("Clearing Console");
	DestroyLineList(&appData->lineList);
	CreateLineList(&appData->lineList, &appData->mainHeap, "");
	
	appData->selectionStart = NewTextLocation(0, 0);
	appData->selectionEnd = NewTextLocation(0, 0);
	appData->uiElements.hoverLocation = NewTextLocation(0, 0);
	
	appData->genericCounter = 0;
}

void RefreshComPortList()
{
	AppData_t* appData = GL_AppData;
	const PlatformInfo_t* PlatformInfo = Gl_PlatformInfo;
	
	appData->numComPortsAvailable = PlatformInfo->GetComPortListPntr(
		&appData->availableComPorts[0], ArrayCount(appData->availableComPorts));
		
	StatusDebug("Found %u COM ports", appData->numComPortsAvailable);
	for (u32 cIndex = 0; cIndex < ArrayCount(appData->availableComPorts); cIndex++)
	{
		if (appData->availableComPorts[cIndex] == true)
		{
			DEBUG_PrintLine("\"%s\"Available!", GetComPortReadableName((ComPortIndex_t)cIndex));
		}
	}
}

void OpenComPort(ComPortIndex_t comPortIndex, ComSettings_t settings)
{
	AppData_t* appData = GL_AppData;
	const PlatformInfo_t* PlatformInfo = Gl_PlatformInfo;
	
	if (appData->comPort.isOpen)
	{
		PlatformInfo->CloseComPortPntr(&appData->comPort);
		StatusError("Closed %s", GetComPortReadableName(appData->comPort.index));
	}
	
	ClearConsole();
	
	appData->comPort = PlatformInfo->OpenComPortPntr(comPortIndex, settings);
	
	if (appData->comPort.isOpen)
	{
		StatusSuccess("%s Opened Successfully", GetComPortReadableName(comPortIndex));
	}
	else
	{
		StatusError("Couldn't open %s port.", GetComPortReadableName(comPortIndex));
	}
}

void ComMenuUpdate(const PlatformInfo_t* PlatformInfo, const AppInput_t* AppInput, MenuHandler_t* menuHandler, Menu_t* menuPntr)
{
	AppData_t* appData = GL_AppData;
	
	if (ButtonPressed(Button_Escape))
	{
		menuPntr->show = !menuPntr->show;
		if (menuPntr->show)
		{
			RefreshComPortList();
			appData->comMenuOptions = appData->comPort;
		}
	}
	
	if (menuPntr->show)
	{
		u32 numTabs = appData->numComPortsAvailable + (appData->comPort.isOpen ? 1 : 0);
		r32 tabWidth = menuPntr->usableRec.width / numTabs;
		{
			r32 tabMinimumWidth = MeasureString(&appData->testFont, "1234567890").x + COM_MENU_TAB_PADDING*2;
			if (tabWidth < tabMinimumWidth) { tabWidth = tabMinimumWidth; }
		}
		rec baudRateRec = NewRectangle(
			menuPntr->usableRec.x + COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			78, appData->testFont.lineHeight * NumBaudRates
		);
		rec numBitsRec = NewRectangle(
			baudRateRec.x + baudRateRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			55, appData->testFont.lineHeight * 8
		);
		rec parityTypesRec = NewRectangle(
			numBitsRec.x + numBitsRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			60, appData->testFont.lineHeight * NumParityTypes
		);
		rec stopBitsRec = NewRectangle(
			parityTypesRec.x + parityTypesRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			80, appData->testFont.lineHeight * NumStopBitTypes
		);
		
		// +==============================+
		// |   Calculate the Menu Width   |
		// +==============================+
		r32 menuWidth = (stopBitsRec.x + stopBitsRec.width + COM_MENU_OUTER_PADDING) - menuPntr->drawRec.x;
		if (menuWidth < tabWidth * numTabs)
		{
			menuWidth = tabWidth * numTabs;
		}
		
		// +==============================+
		// |   Calculate The Tab Height   |
		// +==============================+
		r32 tabHeight = COM_MENU_TAB_HEIGHT;
		for (u32 comIndex = ComPort_1; comIndex < NumComPorts; comIndex++)
		{
			if ((appData->availableComPorts[comIndex] || appData->comPort.index == (ComPortIndex_t)comIndex) &&
				strcmp(GC->comPortNames[comIndex], GetComPortReadableName((ComPortIndex_t)comIndex)) != 0)
			{
				v2 givenNameSize = MeasureFormattedString(&appData->testFont, GC->comPortNames[comIndex], tabWidth - COM_MENU_TAB_PADDING*2, true);
				if (givenNameSize.y + COM_MENU_TAB_PADDING*2 > tabHeight)
				{
					tabHeight = givenNameSize.y + COM_MENU_TAB_PADDING*2;
				}
			}
		}
		baudRateRec.y    += tabHeight;
		numBitsRec.y     += tabHeight;
		parityTypesRec.y += tabHeight;
		stopBitsRec.y    += tabHeight;
		
		// +==============================+
		// |  Calculate the Menu Height   |
		// +==============================+
		r32 menuHeight = (baudRateRec.y + baudRateRec.height + COM_MENU_OUTER_PADDING) - menuPntr->drawRec.y;
		
		// +==============================+
		// |     Update the menu size     |
		// +==============================+
		menuPntr->drawRec.size = NewVec2(menuWidth, menuHeight);
		UpdateMenuRecs(menuPntr);
		
		rec connectButtonRec = NewRectangle(
			menuPntr->usableRec.x + menuPntr->usableRec.width - CONNECT_BUTTON_WIDTH - COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + menuPntr->usableRec.height - CONNECT_BUTTON_HEIGHT - COM_MENU_OUTER_PADDING,
			CONNECT_BUTTON_WIDTH, CONNECT_BUTTON_HEIGHT
		);
		
		for (i32 baudIndex = 0; baudIndex < NumBaudRates; baudIndex++)
		{
			rec currentRec = NewRectangle(baudRateRec.x,
				baudRateRec.y + baudIndex*appData->testFont.lineHeight,
				baudRateRec.width, appData->testFont.lineHeight
			);
			if (IsInsideRectangle(AppInput->mousePos, currentRec) && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], currentRec) &&
				ButtonReleased(MouseButton_Left))
			{
				appData->comMenuOptions.settings.baudRate = (BaudRate_t)baudIndex;
			}
		}
		
		for (i32 bitIndex = 0; bitIndex < 8; bitIndex++)
		{
			rec currentRec = NewRectangle(numBitsRec.x,
				numBitsRec.y + bitIndex*appData->testFont.lineHeight,
				numBitsRec.width, appData->testFont.lineHeight
			);
			if (IsInsideRectangle(AppInput->mousePos, currentRec) && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], currentRec) &&
				ButtonReleased(MouseButton_Left))
			{
				appData->comMenuOptions.settings.numBits = (u8)(bitIndex+1);
			}
		}
		
		for (i32 parityIndex = 0; parityIndex < NumParityTypes; parityIndex++)
		{
			rec currentRec = NewRectangle(parityTypesRec.x,
				parityTypesRec.y + parityIndex*appData->testFont.lineHeight,
				parityTypesRec.width, appData->testFont.lineHeight
			);
			if (IsInsideRectangle(AppInput->mousePos, currentRec) && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], currentRec) &&
				ButtonReleased(MouseButton_Left))
			{
				appData->comMenuOptions.settings.parity = (Parity_t)parityIndex;
			}
		}
		
		for (i32 stopBitIndex = 0; stopBitIndex < NumStopBitTypes; stopBitIndex++)
		{
			rec currentRec = NewRectangle(stopBitsRec.x,
				stopBitsRec.y + stopBitIndex*appData->testFont.lineHeight,
				stopBitsRec.width, appData->testFont.lineHeight
			);
			if (IsInsideRectangle(AppInput->mousePos, currentRec) && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], currentRec) &&
				ButtonReleased(MouseButton_Left))
			{
				appData->comMenuOptions.settings.stopBits = (StopBits_t)stopBitIndex;
			}
		}
		
		//Check for tab Presses
		u32 tabIndex = 0;
		for (u32 comIndex = 0; comIndex < ArrayCount(appData->availableComPorts); comIndex++)
		{
			if (appData->availableComPorts[comIndex] == true ||
				(appData->comPort.isOpen && (ComPortIndex_t)comIndex == appData->comPort.index))
			{
				rec tabRec = NewRectangle(tabIndex * tabWidth, 0, tabWidth, tabHeight);
				tabRec.topLeft += menuPntr->usableRec.topLeft;
				
				if (ButtonReleased(MouseButton_Left) && AppInput->mouseMaxDist[MouseButton_Left] < 10 &&
					IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], tabRec))
				{
					appData->comMenuOptions.index = (ComPortIndex_t)comIndex;
					appData->comMenuOptions.isOpen = true;
				}
				
				tabIndex++;
			}
		}
		
		//Check for connect button press
		bool connectButtonPressed = (IsInsideRectangle(AppInput->mousePos, connectButtonRec) &&
			ButtonReleased(MouseButton_Left) && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], connectButtonRec));
		if (appData->comMenuOptions.isOpen &&
			(connectButtonPressed || ButtonReleased(Button_Enter)))
		{
			OpenComPort(appData->comMenuOptions.index, appData->comMenuOptions.settings);
			menuPntr->show = false;
		}
	}
}
void ComMenuRender(const PlatformInfo_t* PlatformInfo, const AppInput_t* AppInput, RenderState_t* renderState, MenuHandler_t* menuHandler, Menu_t* menuPntr)
{
	AppData_t* appData = GL_AppData;
	
	if (menuPntr->show)
	{
		u32 numTabs = appData->numComPortsAvailable + (appData->comPort.isOpen ? 1 : 0);
		r32 tabWidth = menuPntr->usableRec.width / numTabs;
		rec baudRateRec = NewRectangle(
			menuPntr->usableRec.x + COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			78, appData->testFont.lineHeight * NumBaudRates
		);
		rec numBitsRec = NewRectangle(
			baudRateRec.x + baudRateRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			55, appData->testFont.lineHeight * 8
		);
		rec parityTypesRec = NewRectangle(
			numBitsRec.x + numBitsRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			60, appData->testFont.lineHeight * NumParityTypes
		);
		rec stopBitsRec = NewRectangle(
			parityTypesRec.x + parityTypesRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			80, appData->testFont.lineHeight * NumStopBitTypes
		);
		
		// +==============================+
		// |   Calculate The Tab Height   |
		// +==============================+
		r32 tabHeight = COM_MENU_TAB_HEIGHT;
		for (u32 comIndex = ComPort_1; comIndex < NumComPorts; comIndex++)
		{
			if ((appData->availableComPorts[comIndex] || appData->comPort.index == (ComPortIndex_t)comIndex) &&
				strcmp(GC->comPortNames[comIndex], GetComPortReadableName((ComPortIndex_t)comIndex)) != 0)
			{
				v2 givenNameSize = MeasureFormattedString(&appData->testFont, GC->comPortNames[comIndex], tabWidth - COM_MENU_TAB_PADDING*2, true);
				if (givenNameSize.y + COM_MENU_TAB_PADDING*2 > tabHeight)
				{
					tabHeight = givenNameSize.y + COM_MENU_TAB_PADDING*2;
				}
			}
		}
		baudRateRec.y    += tabHeight;
		numBitsRec.y     += tabHeight;
		parityTypesRec.y += tabHeight;
		stopBitsRec.y    += tabHeight;
		
		rec connectButtonRec = NewRectangle(
			menuPntr->usableRec.x + menuPntr->usableRec.width - CONNECT_BUTTON_WIDTH - COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + menuPntr->usableRec.height - CONNECT_BUTTON_HEIGHT - COM_MENU_OUTER_PADDING,
			CONNECT_BUTTON_WIDTH, CONNECT_BUTTON_HEIGHT
		);
		
		// +==============================+
		// |  Draw the Baud Rate Options  |
		// +==============================+
		renderState->DrawString("Baud Rate", NewVec2(baudRateRec.x, baudRateRec.y - appData->testFont.maxExtendDown), GC->colors.foreground);
		renderState->DrawRectangle(baudRateRec, GC->colors.foreground);
		for (i32 baudIndex = 0; baudIndex < NumBaudRates; baudIndex++)
		{
			const char* baudString = GetBaudRateString((BaudRate_t)baudIndex);
			rec currentRec = NewRectangle(baudRateRec.x,
				baudRateRec.y + baudIndex*appData->testFont.lineHeight,
				baudRateRec.width, appData->testFont.lineHeight
			);
			v2 textPos = baudRateRec.topLeft + NewVec2(
				baudRateRec.width/2 - MeasureString(&appData->testFont, baudString).x/2,
				appData->testFont.maxExtendUp + baudIndex*appData->testFont.lineHeight);
			
			Color_t backColor, textColor, borderColor;
			ButtonColorChoice(backColor, textColor, borderColor, currentRec,
				((BaudRate_t)baudIndex == appData->comMenuOptions.settings.baudRate), false);
			
			renderState->DrawRectangle(currentRec, backColor);
			renderState->DrawString(baudString, textPos, textColor);
		}
		
		// +==============================+
		// |   Draw the # Bits Options    |
		// +==============================+
		renderState->DrawString("# Bits", NewVec2(numBitsRec.x, numBitsRec.y - appData->testFont.maxExtendDown), {GC->colors.foreground});
		renderState->DrawRectangle(numBitsRec, GC->colors.foreground);
		for (i32 bitIndex = 0; bitIndex < 8; bitIndex++)
		{
			char numBitsString[4] = {};
			snprintf(numBitsString, ArrayCount(numBitsString)-1, "%d", bitIndex+1);
			rec currentRec = NewRectangle(numBitsRec.x,
				numBitsRec.y + bitIndex*appData->testFont.lineHeight,
				numBitsRec.width, appData->testFont.lineHeight
			);
			v2 textPos = numBitsRec.topLeft + NewVec2(
				numBitsRec.width/2 - MeasureString(&appData->testFont, numBitsString).x/2,
				appData->testFont.maxExtendUp + bitIndex*appData->testFont.lineHeight);
			
			Color_t backColor, textColor, borderColor;
			ButtonColorChoice(backColor, textColor, borderColor, currentRec,
				((u8)(bitIndex+1) == appData->comMenuOptions.settings.numBits), false);
			
			renderState->DrawRectangle(currentRec, backColor);
			renderState->DrawString(numBitsString, textPos, textColor);
		}
		
		// +==============================+
		// |   Draw the Parity Options    |
		// +==============================+
		renderState->DrawString("Parity", NewVec2(parityTypesRec.x, parityTypesRec.y - appData->testFont.maxExtendDown), {GC->colors.foreground});
		renderState->DrawRectangle(parityTypesRec, GC->colors.foreground);
		for (i32 parityIndex = 0; parityIndex < NumParityTypes; parityIndex++)
		{
			const char* parityString = GetParityString((Parity_t)parityIndex);
			rec currentRec = NewRectangle(parityTypesRec.x,
				parityTypesRec.y + parityIndex*appData->testFont.lineHeight,
				parityTypesRec.width, appData->testFont.lineHeight
			);
			v2 textPos = parityTypesRec.topLeft + NewVec2(
				parityTypesRec.width/2 - MeasureString(&appData->testFont, parityString).x/2,
				appData->testFont.maxExtendUp + parityIndex*appData->testFont.lineHeight);
			
			Color_t backColor, textColor, borderColor;
			ButtonColorChoice(backColor, textColor, borderColor, currentRec,
				((Parity_t)parityIndex == appData->comMenuOptions.settings.parity), false);
			
			renderState->DrawRectangle(currentRec, backColor);
			renderState->DrawString(parityString, textPos, textColor);
		}
		
		// +==============================+
		// |  Draw the Stop Bits Options  |
		// +==============================+
		renderState->DrawString("Stop Bits", NewVec2(stopBitsRec.x, stopBitsRec.y - appData->testFont.maxExtendDown), {GC->colors.foreground});
		renderState->DrawRectangle(stopBitsRec, GC->colors.foreground);
		for (i32 stopBitIndex = 0; stopBitIndex < NumStopBitTypes; stopBitIndex++)
		{
			const char* stopBitsString = GetStopBitsString((StopBits_t)stopBitIndex);
			rec currentRec = NewRectangle(stopBitsRec.x,
				stopBitsRec.y + stopBitIndex*appData->testFont.lineHeight,
				stopBitsRec.width, appData->testFont.lineHeight
			);
			v2 textPos = stopBitsRec.topLeft + NewVec2(
				stopBitsRec.width/2 - MeasureString(&appData->testFont, stopBitsString).x/2,
				appData->testFont.maxExtendUp + stopBitIndex*appData->testFont.lineHeight);
			
			Color_t buttonColor, textColor, borderColor;
			ButtonColorChoice(buttonColor, textColor, borderColor, currentRec,
				((StopBits_t)stopBitIndex == appData->comMenuOptions.settings.stopBits), false);
			
			renderState->DrawRectangle(currentRec, buttonColor);
			renderState->DrawString(stopBitsString, textPos, textColor);
		}
		
		// +==============================+
		// |      Draw the Port Tabs      |
		// +==============================+
		u32 tabIndex = 0;
		for (u32 comIndex = 0; comIndex < ArrayCount(appData->availableComPorts); comIndex++)
		{
			if (appData->availableComPorts[comIndex] == true ||
				(appData->comPort.isOpen && (ComPortIndex_t)comIndex == appData->comPort.index))
			{
				v2 stringSize = MeasureFormattedString(&appData->testFont, GC->comPortNames[comIndex], tabWidth - COM_MENU_TAB_PADDING*2, true);
				rec tabRec = NewRectangle(tabIndex * tabWidth, 0, tabWidth, tabHeight);
				tabRec.topLeft += menuPntr->usableRec.topLeft;
				v2 stringPosition = tabRec.topLeft + NewVec2(tabRec.width/2, tabRec.height/2 - stringSize.y/2 + appData->testFont.maxExtendUp);
				
				Color_t buttonColor, textColor, borderColor;
				ButtonColorChoice(buttonColor, textColor, borderColor, tabRec,
					(appData->comMenuOptions.isOpen == true && (ComPortIndex_t)comIndex == appData->comMenuOptions.index), false);
				
				renderState->DrawButton(tabRec, buttonColor, borderColor);
				// renderState->DrawRectangle(NewRectangle(stringPosition.x - stringSize.x/2, stringPosition.y - appData->testFont.maxExtendUp, stringSize.x, stringSize.y), {Color_Red});
				renderState->DrawFormattedString(GC->comPortNames[comIndex], stringPosition, tabWidth - COM_MENU_TAB_PADDING*2, textColor, Alignment_Center, true);
				
				tabIndex++;
			}
		}
		
		//Draw the connect button
		{
			const char* connectButtonText = "Connect";
			v2 textSize = MeasureString(&appData->testFont, connectButtonText);
			v2 stringPosition = NewVec2(
				connectButtonRec.x + connectButtonRec.width/2 - textSize.x/2,
				connectButtonRec.y + connectButtonRec.height/2 + appData->testFont.lineHeight/2 - appData->testFont.maxExtendDown
			);
			bool settingsHaveChanged = ((!appData->comPort.isOpen && appData->comMenuOptions.isOpen) ||
				appData->comMenuOptions.settings != appData->comPort.settings ||
				appData->comMenuOptions.index != appData->comPort.index);
			
			Color_t buttonColor, borderColor, textColor;
			if (!appData->comMenuOptions.isOpen)
			{
				buttonColor = GC->colors.uiGray1;
				textColor   = {Color_Black};
				borderColor = {Color_Black};
			}
			else
			{
				ButtonColorChoice(buttonColor, textColor, borderColor, connectButtonRec, false, settingsHaveChanged);
			}
			
			renderState->DrawButton(connectButtonRec, buttonColor, borderColor);
			renderState->DrawString(connectButtonText, stringPosition, textColor);
		}
	}
}

void ContextMenuUpdate(const PlatformInfo_t* PlatformInfo, const AppInput_t* AppInput, MenuHandler_t* menuHandler, Menu_t* menu)
{
	AppData_t* appData = GL_AppData;
	UiElements_t* ui = &appData->uiElements;
	
	v2 textSize = MeasureString(&appData->testFont, ui->contextStringBuffer);
	
	menu->drawRec.size = textSize;
	menu->drawRec = RectangleInflate(menu->drawRec, CONTEXT_MENU_PADDING);
	menu->drawRec.topLeft = ui->mousePos + NewVec2(0, -3 - menu->drawRec.height);
}
void ContextMenuRender(const PlatformInfo_t* PlatformInfo, const AppInput_t* AppInput, RenderState_t* renderState, MenuHandler_t* menuHandler, Menu_t* menu)
{
	AppData_t* appData = GL_AppData;
	UiElements_t* ui = &appData->uiElements;
	
	v2 textPos = menu->usableRec.topLeft + NewVec2(CONTEXT_MENU_PADDING, CONTEXT_MENU_PADDING + appData->testFont.maxExtendUp);
	appData->renderState.DrawString(ui->contextStringBuffer, textPos, GC->colors.foreground);
}

u32 SanatizeString(const char* charPntr, u32 numChars, char* outputBuffer = nullptr)
{
	u32 result = 0;
	
	char* outPntr = outputBuffer;
	for (u32 cIndex = 0; cIndex < numChars; cIndex++)
	{
		if (charPntr[cIndex] < ' ') 
		{
			//Ignore these characters
		}
		else
		{
			result++;
			if (outputBuffer != nullptr)
			{
				*outPntr = charPntr[cIndex];
				outPntr++;
			}
		}
	}
	
	return result;
}

bool ApplyTriggerEffects(Line_t* newLine, RegexTrigger_t* trigger)
{
	AppData_t* appData = GL_AppData;
	bool addLineToBuffer = true;
	
	for (u32 eIndex = 0; eIndex < trigger->numEffects; eIndex++)
	{
		const char* effectStr = trigger->effects[eIndex];
		
		if (strcmp(effectStr, "mark") == 0)
		{
			newLine->flags |= LineFlag_MarkBelow;
		}
		else if (strcmp(effectStr, "thick_mark") == 0)
		{
			newLine->flags |= LineFlag_MarkBelow | LineFlag_ThickMark;
		}
		else if (strcmp(effectStr, "clear_screen") == 0)
		{
			//TODO: Do something
		}
		else if (strcmp(effectStr, "new_line") == 0)
		{
			//TODO: Do something
		}
		else if (strcmp(effectStr, "clear_line") == 0)
		{
			addLineToBuffer = false;
		}
		else if (strcmp(effectStr, "count") == 0)
		{
			appData->genericCounter++;
		}
		else if (strstr(effectStr, "=") != nullptr)
		{
			char* nameStr = nullptr;
			char* valueStr = nullptr;
			
			bool foundEquals = false;
			u32 valueStartIndex = 0;
			for (u32 cIndex = 0; effectStr[cIndex] != '\0'; cIndex++)
			{
				char c = effectStr[cIndex];
				if (!foundEquals)
				{
					if (IsCharClassWhitespace(c) || c == '=')
					{
						if (nameStr == nullptr)
						{
							nameStr = TempString(cIndex+1);
							memcpy(nameStr, &effectStr[0], cIndex);
							nameStr[cIndex] = '\0';
						}
					}
					if (c == '=') { foundEquals = true; }
				}
				else
				{
					if (IsCharClassWhitespace(c) == false)
					{
						valueStartIndex = cIndex;
						break;
					}
				}
			}
			Assert(foundEquals);
			
			if (valueStartIndex != 0)
			{
				u32 valueStrLength = (u32)strlen(effectStr) - valueStartIndex;
				valueStr = TempString(valueStrLength+1);
				memcpy(valueStr, &effectStr[valueStartIndex], valueStrLength);
				valueStr[valueStrLength] = '\0';
			}
			
			if (nameStr == nullptr || nameStr[0] == '\0')
			{
				DEBUG_PrintLine("No name found in effect: \"%s\"", effectStr);
			}
			else if (valueStr == nullptr || valueStr[0] == '\0')
			{
				DEBUG_PrintLine("No value found in effect: \"%s\"", effectStr);
			}
			else
			{
				if (strcmp(nameStr, "background_color") == 0 ||
					strcmp(nameStr, "foreground_color") == 0)
				{
					Color_t colorValue = {};
					if (TryParseColor(valueStr, (u32)strlen(valueStr), &colorValue) == ConfigError_None)
					{
						if (strcmp(nameStr, "background_color") == 0)
						{
							// DEBUG_PrintLine("Background = %s", valueStr);
							newLine->backgroundColor = colorValue;
						}
						else if (strcmp(nameStr, "foreground_color") == 0)
						{
							// DEBUG_PrintLine("Foreground = %s", valueStr);
							newLine->matchColor = colorValue;
						}
					}
					else
					{
						DEBUG_PrintLine("Could not parse color in effect: %s = \"%s\"", nameStr, valueStr);
					}
				}
				
				else if (strcmp(nameStr, "status") == 0)
				{
					StatusInfo(valueStr);
				}
				else
				{
					DEBUG_PrintLine("Unknown effect in regex trigger: \"%s\"", nameStr);
				}
			}
		}
		else
		{
			DEBUG_PrintLine("Unknown effect in regex trigger: \"%s\"", effectStr);
		}
	}
	
	return addLineToBuffer;
}

void DataReceived(const char* dataBuffer, i32 numBytes)
{
	const PlatformInfo_t* PlatformInfo = Gl_PlatformInfo;
	AppData_t* appData = GL_AppData;
	Line_t* lastLine = GetLastLine(&appData->lineList);
	
	for (i32 cIndex = 0; cIndex < numBytes; cIndex++)
	{
		char newChar = dataBuffer[cIndex];
		if (newChar == '\n')
		{
			Line_t* finishedLine = lastLine;
			
			if (finishedLine->timestamp == 0)
			{
				finishedLine->timestamp = GetTimestamp(PlatformInfo->localTime);
			}
			
			if (appData->writeToFile)
			{
				//Write the line to the outputFile
				
				char timestampBuffer[256]; ClearArray(timestampBuffer);
				RealTime_t lineTime = RealTimeAt(finishedLine->timestamp);
				u32 timestampLength = snprintf(timestampBuffer, sizeof(timestampBuffer)-1,
					"[%s %02u:%02u:%02u%s (%s %s, %04u)] ",
					GetDayOfWeekStr(GetDayOfWeek(lineTime)),
					Convert24HourTo12Hour(lineTime.hour), lineTime.minute, lineTime.second,
					IsPostMeridian(lineTime.hour) ? "pm" : "am",
					GetMonthStr((Month_t)lineTime.month), GetDayOfMonthString(lineTime.day), lineTime.year);
				
				PlatformInfo->AppendFilePntr(&appData->outputFile, timestampBuffer, timestampLength);
				
				for (u32 cIndex2 = 0; cIndex2 < finishedLine->numChars; cIndex2++)
				{
					if (finishedLine->chars[cIndex2] == 0x01)
					{
						finishedLine->chars[cIndex2] = ' ';
					}
					if (finishedLine->chars[cIndex2] == 0x02)
					{
						finishedLine->chars[cIndex2] = ' ';
					}
					if (finishedLine->chars[cIndex2] == 0x03)
					{
						finishedLine->chars[cIndex2] = ' ';
					}
					if (finishedLine->chars[cIndex2] == 0x04)
					{
						finishedLine->chars[cIndex2] = ' ';
					}
					if (finishedLine->chars[cIndex2] == 0x05)
					{
						finishedLine->chars[cIndex2] = ' ';
					}
				}
				PlatformInfo->AppendFilePntr(&appData->outputFile, finishedLine->chars, finishedLine->numChars);
				PlatformInfo->AppendFilePntr(&appData->outputFile, "\r\n", 2);
				
				LineReset(&appData->lineList, finishedLine);
			}
			else
			{
				//Throw the line on the end of the list
				
				// +====================================+
				// | Check Line Against Regex Triggers  |
				// +====================================+
				bool addLineToBuffer = true;
				for (u32 rIndex = 0; rIndex < GC->numTriggers; rIndex++)
				{
					RegexTrigger_t* trigger = &GC->triggers[rIndex];
					
					if (trigger->runAtEol)
					{
						bool appliedToThisComPort = (trigger->numComPorts == 0 || appData->comPort.isOpen == false);
						if (appliedToThisComPort == false)
						{
							for (u32 comListIndex = 0; comListIndex < trigger->numComPorts; comListIndex++)
							{
								const char* supportedName = trigger->comPorts[comListIndex];
								if (strcmp(supportedName, GetComPortReadableName(appData->comPort.index)) == 0)
								{
									appliedToThisComPort = true;
									break;
								}
								else if (strcmp(supportedName, GC->comPortNames[appData->comPort.index]) == 0)
								{
									appliedToThisComPort = true;
									break;
								}
							}
						}
						
						if (appliedToThisComPort)
						{
							const char* regexStr = nullptr;
							if (trigger->expression != nullptr)
							{
								regexStr = trigger->expression;
							}
							else if (trigger->expressionName != nullptr)
							{
								regexStr = GetRegularExpression(&appData->regexList, trigger->expressionName);
							}
							
							if (regexStr != nullptr)
							{
								bool expressionMatched = TestRegularExpression(regexStr, finishedLine->chars, finishedLine->numChars);
								if (expressionMatched)
								{
									addLineToBuffer &= ApplyTriggerEffects(finishedLine, trigger);
								}
							}
						}
					}
				}
				
				if (addLineToBuffer)
				{
					lastLine = AddLineToList(&appData->lineList, "");
				}
				else
				{
					lastLine->numChars = 0;
					lastLine->chars[0] = '\0';
				}
			}
		}
		else if (newChar == '\b')
		{
			if (lastLine->numChars > 0)
			{
				lastLine->numChars--;
				lastLine->chars[lastLine->numChars] = '\0';
			}
		}
		else
		{
			LineAppend(&appData->lineList, lastLine, newChar);
		}
	}
	
	appData->rxShiftRegister |= 0x80;
}

//NOTE: This function serves as a measuring function AS WELL AS
//		a buffer filling function if not passed nullptr for bufferOutput
u32 GetSelection(char* bufferOutput = nullptr)
{
	const PlatformInfo_t* PlatformInfo = Gl_PlatformInfo;
	AppData_t* appData = GL_AppData;
	
	TextLocation_t minLocation = TextLocationMin(appData->selectionStart, appData->selectionEnd);
	TextLocation_t maxLocation = TextLocationMax(appData->selectionStart, appData->selectionEnd);
	
	if (minLocation.lineNum == maxLocation.lineNum &&
		minLocation.charIndex == maxLocation.charIndex)
	{
		//No selection made
		return 0;
	}
	
	u8 newLineSize = (PlatformInfo->platformType == Platform_Windows) ? 2 : 1;
	char newLine[2] = {};
	if (PlatformInfo->platformType == Platform_Windows)
	{
		newLine[0] = '\r';
		newLine[1] = '\n';
	}
	else
	{
		newLine[0] = '\n';
	}
	u32 bufferLength = 0;
	char* outputPntr = bufferOutput;
	
	if (minLocation.lineNum == maxLocation.lineNum)
	{
		Line_t* linePntr = GetLineAt(&appData->lineList, minLocation.lineNum);
		bufferLength = maxLocation.charIndex - minLocation.charIndex;
		
		if (bufferOutput != nullptr)
		{
			memcpy(outputPntr, &linePntr->chars[minLocation.charIndex], bufferLength);
			outputPntr += bufferLength;
		}
	}
	else
	{
		{ //First Line
			Line_t* minLinePntr = GetLineAt(&appData->lineList, minLocation.lineNum);
			bufferLength += SanatizeString(&minLinePntr->chars[minLocation.charIndex], minLinePntr->numChars - minLocation.charIndex);
			bufferLength += newLineSize;
			if (bufferOutput != nullptr)
			{
				outputPntr += SanatizeString(&minLinePntr->chars[minLocation.charIndex], minLinePntr->numChars - minLocation.charIndex, outputPntr);
				memcpy(outputPntr, newLine, newLineSize);
				outputPntr += newLineSize;
			}
		}
		
		//In Between Lines
		for (i32 lineIndex = minLocation.lineNum+1; lineIndex < maxLocation.lineNum && lineIndex < appData->lineList.numLines; lineIndex++)
		{
			Line_t* linePntr = GetLineAt(&appData->lineList, lineIndex);
			bufferLength += SanatizeString(linePntr->chars, linePntr->numChars);
			bufferLength += newLineSize;
			if (bufferOutput != nullptr)
			{
				outputPntr += SanatizeString(linePntr->chars, linePntr->numChars, outputPntr);
				memcpy(outputPntr, newLine, newLineSize);
				outputPntr += newLineSize;
			}
		}
		
		{ //Last Line
			Line_t* maxLinePntr = GetLineAt(&appData->lineList, maxLocation.lineNum);
			bufferLength += SanatizeString(maxLinePntr->chars, maxLocation.charIndex);
			if (bufferOutput != nullptr)
			{
				outputPntr += SanatizeString(maxLinePntr->chars, maxLocation.charIndex, outputPntr);
			}
		}
	}
	
	bufferLength += 1; //For null terminator
	if (bufferOutput != nullptr)
	{
		*outputPntr = '\0';
	}
	
	return bufferLength;
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
	GC = &appData->globalConfig;
	TempArena = &appData->tempArena;
	
	Assert(AppMemory->permanantSize > INPUT_ARENA_SIZE);
	
	// +==================================+
	// |          Memory Arenas           |
	// +==================================+
	u8* extraSpaceStart = ((u8*)appData) + sizeof(AppData_t);
	
	InitializeMemoryArenaLinear(&appData->inputArena, extraSpaceStart, INPUT_ARENA_SIZE);
	
	u32 mainHeapSize = AppMemory->permanantSize - sizeof(AppData_t) - INPUT_ARENA_SIZE;
	InitializeMemoryArenaHeap(&appData->mainHeap, extraSpaceStart + INPUT_ARENA_SIZE, mainHeapSize);
	
	InitializeMemoryArenaTemp(&appData->tempArena, AppMemory->transientPntr, AppMemory->transientSize, TRANSIENT_MAX_NUMBER_MARKS);
	TempPushMark();
	
	DEBUG_PrintLine("Input Arena: %s", FormattedSizeStr(INPUT_ARENA_SIZE));
	DEBUG_PrintLine("Main Heap:   %s", FormattedSizeStr(mainHeapSize));
	DEBUG_PrintLine("Temp Arena:  %s", FormattedSizeStr(AppMemory->transientSize));
	
	// +================================+
	// |    External Initializations    |
	// +================================+
	LoadGlobalConfiguration(PlatformInfo, &appData->globalConfig, &appData->mainHeap);
	InitializeUiElements(&appData->uiElements);
	InitializeRenderState(PlatformInfo, &appData->renderState);
	InitializeMenuHandler(&appData->menuHandler, &appData->mainHeap);
	InitializeRegexList(&appData->regexList, &appData->mainHeap);
	LoadRegexFile(&appData->regexList, "Resources/Configuration/RegularExpressions.rgx", &appData->mainHeap);
	CreateLineList(&appData->lineList, &appData->mainHeap, "");
	
	v2i screenSize = PlatformInfo->screenSize;
	Menu_t* comMenu = AddMenu(&appData->menuHandler, "COM Menu", NewRectangle((r32)screenSize.x / 2 - 50, (r32)screenSize.y / 2 - 150, 400, 300),
		ComMenuUpdate, ComMenuRender);
	comMenu->show = false;
	Menu_t* contextMenu = AddMenu(&appData->menuHandler, "Context Menu", NewRectangle(0, 0, 100, 100),
		ContextMenuUpdate, ContextMenuRender);
	contextMenu->titleBarSize = 0;
	contextMenu->show = true;
	
	// +================================+
	// |          Load Content          |
	// +================================+
	appData->simpleShader = LoadShader(
		"Resources/Shaders/simple-vertex.glsl",
		"Resources/Shaders/simple-fragment.glsl");
	appData->outlineShader = LoadShader(
		"Resources/Shaders/outline-vertex.glsl",
		"Resources/Shaders/outline-fragment.glsl");
	
	appData->testTexture = LoadTexture("Resources/Sprites/buttonIcon3.png");
	appData->scrollBarEndcapTexture = LoadTexture("Resources/Sprites/scrollBarEndcap.png", false, false);
	
	appData->testFont = LoadFont("Resources/Fonts/consola.ttf",
		(r32)GC->fontSize, 1024, 1024, ' ', 96);
	
	// +================================+
	// |          Frame Buffer          |
	// +================================+
	appData->frameTexture = CreateTexture(nullptr, 2048, 2048);
	appData->frameBuffer = CreateFrameBuffer(&appData->frameTexture);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		DEBUG_WriteLine("FrameBuffer incomplete!");
	}
	
	// +================================+
	// |      Other Initialization      |
	// +================================+
	appData->comPort.settings.baudRate = BaudRate_115200;
	appData->comPort.settings.parity = Parity_None;
	appData->comPort.settings.stopBits = StopBits_1;
	appData->comPort.settings.numBits = 8;
	
	RefreshComPortList();
	
	TempPopMark();
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
	GC = &appData->globalConfig;
	TempArena = &appData->tempArena;
	
	StatusDebug("App Reloaded");
	
	//Make sure our callbacks still match the location of the functions in the new DLL
	Menu_t* menuPntr = GetMenuByName(&appData->menuHandler, "COM Menu");
	menuPntr->specialPntr = nullptr;
	menuPntr->updateFunctionPntr = (void*)ComMenuUpdate;
	menuPntr->renderFunctionPntr = (void*)ComMenuRender;
	menuPntr = GetMenuByName(&appData->menuHandler, "Context Menu");
	menuPntr->specialPntr = nullptr;
	menuPntr->updateFunctionPntr = (void*)ContextMenuUpdate;
	menuPntr->renderFunctionPntr = (void*)ContextMenuRender;
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
	GC = &appData->globalConfig;
	TempArena = &appData->tempArena;
	UiElements_t* ui = &appData->uiElements;
	RenderState_t* rs = &appData->renderState;
	
	TempPushMark();
	
	ClearArray(AppOutput->windowTitle);
	if (appData->comPort.isOpen)
	{
		snprintf(AppOutput->windowTitle, sizeof(AppOutput->windowTitle)-1,
			"%s - Const Port", GC->comPortNames[appData->comPort.index]);
	}
	else
	{
		snprintf(AppOutput->windowTitle, sizeof(AppOutput->windowTitle)-1,
			"Const Port [Disconnected]");
	}
	
	RecalculateUiElements(AppInput, ui, true);
	
	Menu_t* comMenu = GetMenuByName(&appData->menuHandler, "COM Menu");
	Menu_t* contextMenu = GetMenuByName(&appData->menuHandler, "Context Menu");
	Menu_t* hoverMenu = GetMenuAtPoint(&appData->menuHandler, AppInput->mousePos);
	ui->mouseInMenu = (hoverMenu != nullptr && hoverMenu != contextMenu);
	Color_t color1 = ColorFromHSV((i32)(PlatformInfo->programTime*180) % 360, 1.0f, 1.0f);
	Color_t color2 = ColorFromHSV((i32)(PlatformInfo->programTime*180 + 125) % 360, 1.0f, 1.0f);
	Color_t selectionColor = ColorLerp(GC->colors.selection1, GC->colors.selection2, (Sin32((r32)PlatformInfo->programTime*6.0f) + 1.0f) / 2.0f);
	Color_t hoverLocColor  = ColorLerp(GC->colors.foreground, GC->colors.background, (Sin32((r32)PlatformInfo->programTime*8.0f) + 1.0f) / 2.0f);
	// Color_t selectionColor = ColorFromHSV(180, 1.0f, (r32)(Sin32((r32)PlatformInfo->programTime*5) + 1.0f) / 2.0f);
	
	//+================================+
	//|  Context Menu Showing/Filling  |
	//+================================+
	contextMenu->show = false;
	if (ui->mouseInMenu == false &&
		ButtonDown(Button_Control) &&// && mousePos.x <= ui->gutterRec.width)
		(IsInsideRectangle(ui->mousePos, ui->viewRec) || IsInsideRectangle(ui->mousePos, ui->gutterRec)))
	{
		Line_t* linePntr = GetLineAt(&appData->lineList, ui->hoverLocation.lineNum);
		
		if (linePntr != nullptr && linePntr->timestamp != 0)
		{
			RealTime_t lineTime = RealTimeAt(linePntr->timestamp);
			
			if (ButtonDown(Button_Shift))
			{
				snprintf(ui->contextStringBuffer, sizeof(ui->contextStringBuffer)-1,
					"%s %u:%02u:%02u%s (%s %s, %u)",
					GetDayOfWeekStr(GetDayOfWeek(lineTime)),
					Convert24HourTo12Hour(lineTime.hour), lineTime.minute, lineTime.second,
					IsPostMeridian(lineTime.hour) ? "pm" : "am",
					GetMonthStr((Month_t)lineTime.month), GetDayOfMonthString(lineTime.day), lineTime.year);
			}
			else
			{
				i64 secondsDifference = SubtractTimes(PlatformInfo->localTime, lineTime, TimeUnit_Seconds);
				i64 absDifference = Abs64i(secondsDifference);
				u32 numCharacters = GetElapsedString((u64)absDifference, &ui->contextStringBuffer[0], ArrayCount(ui->contextStringBuffer));
				strncpy(&ui->contextStringBuffer[numCharacters], " Ago", sizeof(ui->contextStringBuffer) - numCharacters - 1);
			}
			
			contextMenu->show = true;
		}
	}
	
	// +==============================+
	// |    Read From The COM Port    |
	// +==============================+
	if (appData->comPort.isOpen)
	{
		i32 readResult = 1;
		while (readResult > 0)
		{
			char buffer[4096] = {};
			readResult = PlatformInfo->ReadComPortPntr(&appData->comPort, buffer, ArrayCount(buffer)-1);
			if (readResult > 0)
			{
				// DEBUG_PrintLine("Read %d bytes \"%.*s\"", readResult, readResult, buffer);
				
				if (appData->programInstance.isOpen == false ||
					GC->sendComDataToPython == false ||
					GC->alsoShowComData == true)
				{
					DataReceived(buffer, readResult);
				}
				
				if (appData->programInstance.isOpen && GC->sendComDataToPython)
				{
					DEBUG_PrintLine("Writing to program instance \"%.*s\"", readResult, buffer);
					
					u32 numBytesWritten = PlatformInfo->WriteProgramInputPntr(&appData->programInstance, &buffer[0], readResult);
					if ((i32)numBytesWritten != readResult)
					{
						DEBUG_PrintLine("Only wrote %u/%u bytes to program instance", numBytesWritten, readResult);
					}
				}
			}
			else if (readResult < 0)
			{
				DEBUG_PrintLine("COM port read Error!: %d", readResult);
			}
		}
	}
	
	// +==============================+
	// |  Read From Program Instance  |
	// +==============================+
	if (appData->programInstance.isOpen)
	{
		char readBuffer[256] = {};
		
		u32 numBytesRead = 1;
		while (numBytesRead > 0)
		{
			numBytesRead = PlatformInfo->ReadProgramOutputPntr(&appData->programInstance, readBuffer, ArrayCount(readBuffer));
			
			if (numBytesRead > 0)
			{
				DEBUG_PrintLine("Read %u bytes from program: \"%.*s\"", numBytesRead, numBytesRead, readBuffer);
				
				if (GC->showPythonOutput)
				{
					DataReceived(readBuffer, numBytesRead);
				}
			}
			// else if (ButtonDown(Button_Control))
			// {
			// 	DEBUG_WriteLine("Nothing");
			// }
		}
		
		ProgramStatus_t programStatus = PlatformInfo->GetProgramStatusPntr(&appData->programInstance);
		if (programStatus != ProgramStatus_Running)
		{
			DEBUG_WriteLine("Program instance finished!");
			PlatformInfo->CloseProgramInstancePntr(&appData->programInstance);
		}
	}
	
	// +============================================+
	// | Read Keyboard Input and Route Accordingly  |
	// +============================================+
	if (true)
	{
		bool writeToComPort = (appData->comPort.isOpen && (appData->programInstance.isOpen == false || GC->sendInputToPython == false || GC->alsoSendInputToCom));
		bool writeToProgram = (appData->programInstance.isOpen && GC->sendInputToPython);
		bool echoInput = GC->autoEchoInput;
		
		if (AppInput->textInputLength > 0)
		{
			// DEBUG_PrintLine("Writing \"%.*s\"", AppInput->textInputLength, AppInput->textInput);
			if (echoInput) { DataReceived(&AppInput->textInput[0], AppInput->textInputLength); }
			if (writeToComPort)
			{
				PlatformInfo->WriteComPortPntr(&appData->comPort, &AppInput->textInput[0], AppInput->textInputLength);
				appData->txShiftRegister |= 0x80;
			}
			if (writeToProgram) { PlatformInfo->WriteProgramInputPntr(&appData->programInstance, &AppInput->textInput[0], AppInput->textInputLength); }
		}
		
		if (!comMenu->show && ButtonPressed(Button_Enter))
		{
			DEBUG_WriteLine("Writing New Line");
			
			char newChar = '\n';
			if (echoInput) { DataReceived("\n", 1); }
			if (writeToComPort)
			{
				PlatformInfo->WriteComPortPntr(&appData->comPort, &newChar, 1);
				appData->txShiftRegister |= 0x80;
			}
			if (writeToProgram) { PlatformInfo->WriteProgramInputPntr(&appData->programInstance, &newChar, 1); }
		}
		
		if (!comMenu->show && ButtonPressed(Button_Backspace))
		{
			DEBUG_WriteLine("Writing Backspace");
			
			char newChar = '\b';
			if (echoInput) { DataReceived("\b", 1); }
			if (writeToComPort)
			{
				PlatformInfo->WriteComPortPntr(&appData->comPort, &newChar, 1);
				appData->txShiftRegister |= 0x80;
			}
			if (writeToProgram)
			{
				PlatformInfo->WriteProgramInputPntr(&appData->programInstance, &newChar, 1);
			}
		}
	}
	
	//+==================================+
	//|        Recenter COM menu         |
	//+==================================+
	if (PlatformInfo->windowResized)
	{
		comMenu->drawRec.topLeft = NewVec2(
			PlatformInfo->screenSize.x / 2 - comMenu->drawRec.width/2,
			PlatformInfo->screenSize.y / 2 - comMenu->drawRec.height/2);
	}
	
	//+================================+
	//|         Show COM Menu          |
	//+================================+
	if (ButtonPressed(Button_O) &&
		ButtonDown(Button_Control))
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
			appData->comMenuOptions = appData->comPort;
		}
	}
	
	//+======================================+
	//| Clear Console and Copy to Clipboard  |
	//+======================================+
	if (ButtonPressed(Button_C) &&
		ButtonDown(Button_Control))
	{
		if (ButtonDown(Button_Shift))
		{
			ClearConsole();
		}
		else
		{
			u32 selectionSize = GetSelection();
			if (selectionSize != 0)
			{
				TempPushMark();
				
				char* selectionTempBuffer = TempString(selectionSize+1);
				GetSelection(selectionTempBuffer);
				
				PlatformInfo->CopyToClipboardPntr(selectionTempBuffer, selectionSize);
				
				TempPopMark();
			}
		}
	}
	
	//+==================================+
	//|   Reload Global Configuration    |
	//+==================================+
	if (ButtonPressed(Button_R) &&
		ButtonDown(Button_Control))
	{
		DisposeGlobalConfig(&appData->globalConfig);
		LoadGlobalConfiguration(PlatformInfo, &appData->globalConfig, &appData->mainHeap);
		DisposeRegexFile(&appData->regexList);
		LoadRegexFile(&appData->regexList, "Resources/Configuration/RegularExpressions.rgx", &appData->mainHeap);
	}
	
	//+==================================+
	//|       Select/Deselect all        |
	//+==================================+
	if (ButtonPressed(Button_A) && ButtonDown(Button_Control))
	{
		if (appData->selectionStart.lineNum == appData->selectionEnd.lineNum &&
			appData->selectionStart.charIndex == appData->selectionEnd.charIndex)
		{
			//Select all
			appData->selectionStart = NewTextLocation(0, 0);
			Line_t* lastLinePntr = GetLastLine(&appData->lineList);
			appData->selectionEnd = NewTextLocation(appData->lineList.numLines-1, lastLinePntr->numChars);
			
			StatusInfo("Selected Everything");
		}
		else
		{
			//Deselect all
			appData->selectionStart = NewTextLocation(0, 0);
			appData->selectionEnd = NewTextLocation(0, 0);
			
			StatusError("Unselected Everything");
		}
	}
	
	// +==================================+
	// |      Toggle Output To File       |
	// +==================================+
	if (ButtonPressed(Button_F) && ButtonDown(Button_Control))
	{
		char outputFileName[32]; ClearArray(outputFileName);
		const char* comPortName = GetComPortReadableName(appData->comPort.index);
		strcpy(&outputFileName[0], comPortName);
		strcpy(&outputFileName[strlen(comPortName)], "_Output.txt");
		
		DEBUG_PrintLine("Outputting to file: \"%s\"", outputFileName);
		
		if (appData->writeToFile)
		{
			PlatformInfo->CloseFilePntr(&appData->outputFile);
			appData->writeToFile = false;
			StatusSuccess("Stopped outputting to file");
		}
		else
		{
			if (PlatformInfo->OpenFilePntr(outputFileName, &appData->outputFile))
			{
				StatusSuccess("Opened file successfully");
				appData->writeToFile = true;
				const char* newString = "\r\n\r\n[File Opened for Writing]\r\n";
				PlatformInfo->AppendFilePntr(&appData->outputFile, newString, (u32)strlen(newString));
			}
			else
			{
				StatusError("Error opening \"%s\" for writing", outputFileName);
			}
		}
	}
	
	//+==================================+
	//| Quick keys for opening COM ports |
	//+==================================+
	if (ButtonDown(Button_Control))
	{
		ComPortIndex_t cIndex = NumComPorts;
		
		if (ButtonReleased(Button_1)) cIndex = ComPort_1;
		if (ButtonReleased(Button_2)) cIndex = ComPort_2;
		if (ButtonReleased(Button_3)) cIndex = ComPort_3;
		if (ButtonReleased(Button_4)) cIndex = ComPort_4;
		if (ButtonReleased(Button_5)) cIndex = ComPort_5;
		if (ButtonReleased(Button_6)) cIndex = ComPort_6;
		if (ButtonReleased(Button_7)) cIndex = ComPort_7;
		if (ButtonReleased(Button_8)) cIndex = ComPort_8;
		if (ButtonReleased(Button_9)) cIndex = ComPort_9;
		
		if (cIndex != NumComPorts)
		{
			if (comMenu->show)
				comMenu->show = false;
			
			RefreshComPortList();
			
			if (appData->availableComPorts[cIndex] == false)
			{
				StatusError("%s not Available!", GetComPortReadableName(cIndex));
			}
			else
			{
				OpenComPort(cIndex, appData->comPort.settings);
			}
		}
	}
	
	// +================================+
	// |      Toggle Debug Overlay      |
	// +================================+
	if (ButtonPressed(Button_F11))
	{
		appData->showDebugMenu = !appData->showDebugMenu;
	}
	
	RecalculateUiElements(AppInput, ui, false);
	
	if (ButtonDown(Button_Right))
	{
		ui->scrollOffsetGoto.x += ButtonDown(Button_Shift) ? 16 : 5;
	}
	if (ButtonDown(Button_Left))
	{
		ui->scrollOffsetGoto.x -= ButtonDown(Button_Shift) ? 16 : 5;
	}
	if (ButtonDown(Button_Down))
	{
		ui->scrollOffsetGoto.y += ButtonDown(Button_Shift) ? 16 : 5;
	}
	if (ButtonDown(Button_Up))
	{
		ui->scrollOffsetGoto.y -= ButtonDown(Button_Shift) ? 16 : 5;
		ui->followingEndOfFile = false;
	}
	if (AppInput->scrollDelta.y != 0)
	{
		if (ButtonDown(Button_Shift))
		{
			ui->scrollOffsetGoto.x -= AppInput->scrollDelta.y * (r32)GC->scrollMultiplier;
		}
		else
		{
			ui->scrollOffsetGoto.y -= AppInput->scrollDelta.y * (r32)GC->scrollMultiplier;
			
			if (AppInput->scrollDelta.y > 0)
			{
				ui->followingEndOfFile = false;
			}
		}
	}
	if (AppInput->scrollDelta.x != 0)
	{
		ui->scrollOffsetGoto.x -= AppInput->scrollDelta.x * (r32)GC->scrollMultiplier;
	}
	
	bool gotoEndButtonPressed = (IsInsideRectangle(AppInput->mousePos, ui->gotoEndButtonRec) &&
		ButtonReleased(MouseButton_Left) &&
		IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], ui->gotoEndButtonRec));
	if (gotoEndButtonPressed || ButtonPressed(Button_End))
	{
		ui->followingEndOfFile = true;
	}
	if (ButtonPressed(Button_Home))
	{
		ui->scrollOffsetGoto.y = 0;
		ui->followingEndOfFile = false;
	}
	if (ButtonPressed(Button_PageUp))
	{
		ui->scrollOffsetGoto.y -= ui->viewRec.height;
		ui->followingEndOfFile = false;
	}
	if (ButtonPressed(Button_PageDown))
	{
		ui->scrollOffsetGoto.y += ui->viewRec.height;
	}
	
	// +==================================+
	// | Start/Stop Test Program Instance |
	// +==================================+
	if (ButtonPressed(Button_P) && ButtonDown(Button_Control) &&
		GC->pythonScriptEnabled)
	{
		if (appData->programInstance.isOpen)
		{
			StatusInfo("Closing python instance");
			PlatformInfo->CloseProgramInstancePntr(&appData->programInstance);
		}
		else
		{
			if (GC->pythonScript != nullptr)
			{
				char* commandStr = TempPrint("python %s", GC->pythonScript);
				StatusInfo("Running System Command: \"%s\"", commandStr);
				appData->programInstance = PlatformInfo->StartProgramInstancePntr(commandStr);
				if (appData->programInstance.isOpen == false)
				{
					StatusError("Python exec failed: \"%s\"", commandStr);
				}
			}
			else
			{
				StatusError("No python script defined!");
			}
		}
	}
	
	
	
	// +==================================+
	// |     Test Regular Expression      |
	// +==================================+
	if (ButtonPressed(Button_E) && ButtonDown(Button_Control))
	{
		u32 selectionLength = GetSelection(nullptr);
		if (selectionLength > 0)
		{
			const char* countExpression = GetRegularExpression(&appData->regexList, GC->genericCountRegexName);
			if (countExpression != nullptr)
			{
				TempPushMark();
				
				char* selectionBuffer = TempString(selectionLength);
				GetSelection(selectionBuffer);
				
				TestRegularExpression(countExpression, selectionBuffer, selectionLength);
				
				TempPopMark();
			}
			else
			{
				DEBUG_WriteLine("Could not get Generic Count Regular Expression");
			}
		}
	}
	
	//+==================================+
	//|        Main Menu Buttons         |
	//+==================================+
	if (ui->mouseInMenu == false &&
		ButtonReleased(MouseButton_Left) && IsInsideRectangle(ui->mousePos, ui->mainMenuRec) &&
		IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], ui->mainMenuRec))
	{
		for (u32 bIndex = 0; bIndex < ArrayCount(ui->buttonRecs); bIndex++)
		{
			if (IsInsideRectangle(ui->mousePos, ui->buttonRecs[bIndex]) &&
				IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], ui->buttonRecs[bIndex]))
			{
				switch (bIndex)
				{
					case Button_ComPort:
					{
						if (comMenu->show)
						{
							comMenu->show = false;
						}
						else
						{
							comMenu->show = true;
							
							RefreshComPortList();
							appData->comMenuOptions = appData->comPort;
						}
					} break;
					
					case Button_Settings:
					{
						//TODO: Create file if it doesn't exist
						if (ButtonDown(Button_Shift) && ButtonDown(Button_Control))
						{
							PlatformInfo->LaunchFilePntr("Resources\\Configuration\\PlatformConfig.json");
							PlatformInfo->LaunchFilePntr("Resources\\Configuration\\RegularExpressions.rgx");
							PlatformInfo->LaunchFilePntr("Resources\\Configuration\\GlobalConfig.json");
						}
						else if (ButtonDown(Button_Shift))
						{
							PlatformInfo->LaunchFilePntr("Resources\\Configuration\\PlatformConfig.json");
						}
						else if (ButtonDown(Button_Control))
						{
							PlatformInfo->LaunchFilePntr("Resources\\Configuration\\RegularExpressions.rgx");
						}
						else
						{
							PlatformInfo->LaunchFilePntr("Resources\\Configuration\\GlobalConfig.json");
						}
					} break;
					
					default:
					{
						StatusError("Button does nothing.");
					};
				};
			}
		}
	}
	
	//+================================+
	//|       Clear Button Press       |
	//+================================+
	if (IsInsideRectangle(AppInput->mousePos, ui->clearButtonRec))
	{
		if (ButtonReleased(MouseButton_Left) &&
			IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], ui->clearButtonRec))
		{
			ClearConsole();
		}
	}
	
	//+==================================+
	//|    Save To File Button Press     |
	//+==================================+
	bool saveButtonPressed = (IsInsideRectangle(AppInput->mousePos, ui->saveButtonRec) &&
		ButtonReleased(MouseButton_Left) &&
		IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], ui->saveButtonRec));
	if (saveButtonPressed ||
		(ButtonDown(Button_Control) && ButtonPressed(Button_S)))
	{
		char fileNameBuffer[256] = {};
		u32 fileNameLength = snprintf(&fileNameBuffer[0], sizeof(fileNameBuffer),
			"ConstPortSave_%02u-%02u-%u_%u-%02u-%02u.txt",
			PlatformInfo->localTime.year, PlatformInfo->localTime.month, PlatformInfo->localTime.day,
			PlatformInfo->localTime.hour, PlatformInfo->localTime.minute, PlatformInfo->localTime.second);
		
		u32 selectionSize = GetSelection(nullptr);
		if (selectionSize > 0)
		{
			TempPushMark();
			
			char* fileBuffer = TempString(selectionSize);
			GetSelection(fileBuffer);
			
			//NOTE: GetSelection adds a \0 on the end so need to remove it
			DEBUG_PrintLine("Saving %u bytes to %s", selectionSize-1, fileNameBuffer);
			PlatformInfo->WriteEntireFilePntr(fileNameBuffer, fileBuffer, selectionSize-1);
			DEBUG_WriteLine("Done!");
			
			TempPopMark();
			
			if (PlatformInfo->LaunchFilePntr(fileNameBuffer))
			{
				DEBUG_WriteLine("Opened output file for viewing");
			}
			else
			{
				DEBUG_WriteLine("Could not open output file");
			}
			
			StatusSuccess("Saved to %s", fileNameBuffer);
		}
	}
	
	//+==========================================+
	//| Scrollbar Interaction and Text Selection |
	//+==========================================+
	if (ButtonDown(MouseButton_Left) && !ui->mouseInMenu)
	{
		//Handle scrollbar interaction with mouse
		if (IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], ui->scrollBarGutterRec) &&
			ui->scrollBarRec.height < ui->scrollBarGutterRec.height)
		{
			if (AppInput->buttons[MouseButton_Left].transCount > 0)//Pressed the button down
			{
				ui->mouseScrollbarOffset = ui->mousePos.y - ui->scrollBarRec.y;
				if (IsInsideRectangle(ui->mousePos, ui->scrollBarRec))
				{
					ui->startedOnScrollbar = true;
				}
				else
				{
					ui->startedOnScrollbar = false;
					if (ui->mouseScrollbarOffset > 0)
					{
						ui->scrollOffsetGoto.y += ui->viewRec.height;
					}
					else
					{
						ui->scrollOffsetGoto.y -= ui->viewRec.height;
						ui->followingEndOfFile = false;
					}
				}
			}
			else if (ui->startedOnScrollbar) //holding the button
			{
				r32 newPixelLocation = ui->mousePos.y - ui->mouseScrollbarOffset - ui->scrollBarGutterRec.y;
				if (newPixelLocation > ui->scrollBarGutterRec.y + (ui->scrollBarGutterRec.height - ui->scrollBarRec.height))
				{
					newPixelLocation = ui->scrollBarGutterRec.y + (ui->scrollBarGutterRec.height - ui->scrollBarRec.height);
				}
				if (newPixelLocation < 0)
				{
					newPixelLocation = 0;
				}
				
				ui->scrollOffset.y = (newPixelLocation / (ui->scrollBarGutterRec.height - ui->scrollBarRec.height)) * ui->maxScrollOffset.y;
				ui->scrollOffsetGoto.y = ui->scrollOffset.y;
				
				if (ui->scrollOffsetGoto.y < ui->maxScrollOffset.y - appData->testFont.lineHeight)
				{
					ui->followingEndOfFile = false;
				}
			}
		}
		else if (IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], ui->viewRec))
		{
			if (AppInput->buttons[MouseButton_Left].transCount > 0)//Pressed the button down
			{
				appData->selectionStart = ui->hoverLocation;
				appData->selectionEnd = ui->hoverLocation;
			}
			else //if (IsInsideRectangle(ui->mousePos, ui->viewRec)) //Mouse Button Holding
			{
				appData->selectionEnd = ui->hoverLocation;
			}
		}
	}
	
	//+================================+
	//|   Mark lines using the mouse   |
	//+================================+
	if (ui->mouseInMenu == false &&
		IsInsideRectangle(ui->mousePos, ui->gutterRec) && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], ui->gutterRec))
	{
		if (ButtonReleased(MouseButton_Left) &&
			ui->markIndex >= 0 && ui->markIndex < appData->lineList.numLines)
		{
			Line_t* linePntr = GetLineAt(&appData->lineList, ui->markIndex);
			
			if (!IsFlagSet(linePntr->flags, LineFlag_MarkBelow) ||
				(ButtonDown(Button_Shift) && !IsFlagSet(linePntr->flags, LineFlag_ThickMark)))
			{
				FlagSet(linePntr->flags, LineFlag_MarkBelow);
				if (ButtonDown(Button_Shift))
				{
					FlagSet(linePntr->flags, LineFlag_ThickMark);
				}
				else
				{
					FlagUnset(linePntr->flags, LineFlag_ThickMark);
				}
			}
			else
			{
				FlagUnset(linePntr->flags, LineFlag_MarkBelow);
			}
		}
	}
	
	if (ButtonDown(Button_Control) && ButtonPressed(Button_M))
	{
		i32 lastIndex = appData->lineList.numLines - 1;
		Line_t* linePntr = GetLineAt(&appData->lineList, lastIndex);
		if (linePntr->numChars == 0 && lastIndex > 0)
		{
			linePntr = GetLineAt(&appData->lineList, lastIndex - 1);
		}
		
		//Set mark
		if (!IsFlagSet(linePntr->flags, LineFlag_MarkBelow) ||
				(ButtonDown(Button_Shift) && !IsFlagSet(linePntr->flags, LineFlag_ThickMark)))
		{
			FlagSet(linePntr->flags, LineFlag_MarkBelow);
			if (ButtonDown(Button_Shift))
			{
				FlagSet(linePntr->flags, LineFlag_ThickMark);
			}
			else
			{
				FlagUnset(linePntr->flags, LineFlag_ThickMark);
			}
		}
		else
		{
			FlagUnset(linePntr->flags, LineFlag_MarkBelow);
		}
	}
	
	//+================================+
	//|          Update Menus          |
	//+================================+
	MenuHandlerUpdate(PlatformInfo, AppInput, &appData->menuHandler);
	
	UpdateUiElements(AppInput, ui);
	RecalculateUiElements(AppInput, ui, false);
	if (ui->followingEndOfFile)
	{
		ui->scrollOffsetGoto.y = ui->maxScrollOffset.y;
	}
	
	//+==================================+
	//|           Cursor Type            |
	//+==================================+
	if (IsInsideRectangle(ui->mousePos, ui->viewRec) && !ui->mouseInMenu && GC->showTextCursor)
	{
		AppOutput->cursorType = Cursor_Text;
	}
	else
	{
		AppOutput->cursorType = Cursor_Default;
	}
	
	//+--------------------------------------+
	//|           Rendering Setup            |
	//+--------------------------------------+
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	rs->SetViewport(NewRectangle(0, 0, (r32)PlatformInfo->screenSize.x, (r32)PlatformInfo->screenSize.y));
	
	glClearColor((GC->colors.background.r/255.f), (GC->colors.background.g/255.f), (GC->colors.background.b/255.f), 1.0f);
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
	
	// rs->DrawGradient(NewRectangle(0, 0, 300, 300), color1, color2, Direction2D_Right);
	
	//+--------------------------------------+
	//|            Render Lines              |
	//+--------------------------------------+
	{
		i32 firstLine = max(0, ui->firstRenderLine);
		
		rs->SetViewMatrix(Matrix4Translate(NewVec3(ui->viewRec.x - ui->scrollOffset.x, ui->viewRec.y - ui->scrollOffset.y, 0)));
		{//Items drawn relative to view
			
			v2 currentPos = NewVec2((r32)GC->lineSpacing, ui->scrollOffset.y - ui->firstRenderLineOffset + appData->testFont.maxExtendUp);
			for (i32 lineIndex = firstLine; lineIndex < appData->lineList.numLines; lineIndex++)
			{
				Line_t* linePntr = GetLineAt(&appData->lineList, lineIndex);
				
				r32 lineHeight = RenderLine(AppInput, linePntr, currentPos, true);
				//Draw line highlight
				if (GC->highlightHoverLine &&
					lineIndex == ui->hoverLocation.lineNum &&
					IsInsideRectangle(ui->mousePos, ui->viewRec) &&
					!ui->mouseInMenu)
				{
					rec backRec = NewRectangle(
						currentPos.x + ui->scrollOffset.x,
						currentPos.y - appData->testFont.maxExtendUp,
						ui->viewRec.width, lineHeight
					);
					rs->DrawRectangle(backRec, GC->colors.hoverLine);
				}
				
				RenderLine(AppInput, linePntr, currentPos, false);
				
				if (GC->showHoverCursor &&
					lineIndex == ui->hoverLocation.lineNum &&
					IsInsideRectangle(ui->mousePos, ui->viewRec) &&
					!ui->mouseInMenu)
				{
					v2 skipSize = MeasureString(&appData->testFont, linePntr->chars, ui->hoverLocation.charIndex);
					rec cursorRec = NewRectangle(
						currentPos.x + skipSize.x,
						currentPos.y - appData->testFont.maxExtendUp,
						1, appData->testFont.lineHeight
					);
					rs->DrawRectangle(cursorRec, hoverLocColor);
				}
				
				currentPos.y += lineHeight + GC->lineSpacing;
				if (currentPos.y - appData->testFont.maxExtendUp >= ui->scrollOffset.y + ui->viewRec.height)
				{
					//We've reached the bottom of the view
					break;
				}
			}
			
		}
		rs->SetViewMatrix(Matrix4_Identity);
	}
	
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
		i32 firstLine = max(0, ui->firstRenderLine);
		
		rs->SetViewMatrix(Matrix4Translate(NewVec3(ui->viewRec.x - ui->scrollOffset.x, ui->viewRec.y - ui->scrollOffset.y, 0)));
		{//Items drawn relative to view
			
			v2 currentPos = NewVec2(0, ui->scrollOffset.y - ui->firstRenderLineOffset + appData->testFont.maxExtendUp);
			for (i32 lineIndex = firstLine;
				lineIndex < appData->lineList.numLines && lineIndex <= maxLocation.lineNum;
				lineIndex++)
			{
				Line_t* linePntr = GetLineAt(&appData->lineList, lineIndex);
				
				if (lineIndex >= minLocation.lineNum && lineIndex <= maxLocation.lineNum)
				{
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
						selectionSize = MeasureString(&appData->testFont, linePntr->chars);
						// selectionSize.x += MeasureString(&appData->testFont, " ", 1).x;
					}
					
					rec backRec = NewRectangle(GC->lineSpacing + currentPos.x + skipSize.x, currentPos.y - appData->testFont.maxExtendUp, selectionSize.x, appData->testFont.lineHeight);//linePntr->lineHeight);
					backRec = RectangleInflate(backRec, (r32)GC->lineSpacing/2);
					rs->DrawRectangle(backRec, selectionColor);
					
					if (currentPos.y - appData->testFont.maxExtendUp >= ui->scrollOffset.y + ui->viewRec.height)
					{
						//We've reached the bottom of the view
						break;
					}
				}
				
				currentPos.y += linePntr->lineHeight + GC->lineSpacing;
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
	
	rs->SetSecondaryColor(NewColor(0, 0, 0, 20)); //TODO: Add this as a configuration option
	rs->BindTexture(&appData->frameTexture);
	rs->DrawTexturedRec(NewRectangle(0, ui->screenSize.y, (r32)appData->frameTexture.width, (r32)-appData->frameTexture.height), {Color_White});
	
	rs->BindShader(&appData->simpleShader);
	rs->UpdateShader();
	
	//+--------------------------------------+
	//|    Render Line Gutter Elements       |
	//+--------------------------------------+
	rs->DrawGradient(ui->gutterRec, GC->colors.uiGray1, GC->colors.uiGray3, Direction2D_Right);
	{
		i32 firstLine = max(0, ui->firstRenderLine);
		
		rs->SetViewMatrix(Matrix4Translate(NewVec3(ui->gutterRec.x, ui->gutterRec.y - ui->scrollOffset.y, 0)));
		{//Items drawn relative to view
			
			v2 currentPos = NewVec2(0, ui->scrollOffset.y - ui->firstRenderLineOffset + appData->testFont.maxExtendUp);
			for (i32 lineIndex = firstLine; lineIndex < appData->lineList.numLines; lineIndex++)
			{
				Line_t* linePntr = GetLineAt(&appData->lineList, lineIndex);
				
				RenderLineGutter(AppInput, linePntr, currentPos, lineIndex, linePntr->lineHeight);
				
				currentPos.y += linePntr->lineHeight + GC->lineSpacing;
				if (currentPos.y - appData->testFont.maxExtendUp >= ui->scrollOffset.y + ui->viewRec.height)
				{
					//We've reached the bottom of the view
					break;
				}
			}
			
		}
		rs->SetViewMatrix(Matrix4_Identity);
	}
	
	//+--------------------------------------+
	//|           Render Scrollbar           |
	//+--------------------------------------+
	{
		rs->DrawGradient(NewRectangle(ui->scrollBarGutterRec.x - 8, ui->scrollBarGutterRec.y, 8, ui->scrollBarGutterRec.height),
			{Color_TransparentBlack}, {Color_HalfTransparentBlack}, Direction2D_Right);
		rs->DrawGradient(ui->scrollBarGutterRec, GC->colors.background, GC->colors.uiGray3, Direction2D_Right);
		
		rec centerScrollBarRec = ui->scrollBarRec;
		centerScrollBarRec.y += ui->scrollBarRec.width;
		centerScrollBarRec.height -= 2 * ui->scrollBarRec.width;
		rec startCapRec = NewRectangle(ui->scrollBarRec.x, ui->scrollBarRec.y, ui->scrollBarRec.width, ui->scrollBarRec.width);
		rec endCapRec = NewRectangle(ui->scrollBarRec.x, ui->scrollBarRec.y + ui->scrollBarRec.height - ui->scrollBarRec.width, ui->scrollBarRec.width, ui->scrollBarRec.width);
		endCapRec.y += endCapRec.height;
		endCapRec.height = -endCapRec.height;
		rs->DrawRectangle(RectangleInflate(centerScrollBarRec, 1), GC->colors.uiGray4);
		rs->BindAlphaTexture(&appData->scrollBarEndcapTexture);
		rs->DrawRectangle(RectangleInflate(startCapRec, 1), GC->colors.uiGray4);
		rs->DrawRectangle(RectangleInflate(endCapRec, 1), GC->colors.uiGray4);
		
		rs->DrawGradient(startCapRec, GC->colors.uiGray1, GC->colors.uiGray3, Direction2D_Right);
		rs->DrawGradient(endCapRec, GC->colors.uiGray1, GC->colors.uiGray3, Direction2D_Right);
		rs->DisableAlphaTexture();
		rs->DrawGradient(centerScrollBarRec, GC->colors.uiGray1, GC->colors.uiGray3, Direction2D_Right);
	}
	
	//+--------------------------------------+
	//|          Render Status Bar           |
	//+--------------------------------------+
	{
		rs->DrawGradient(ui->statusBarRec, GC->colors.uiGray1, GC->colors.uiGray3, Direction2D_Right);
		// rs->DrawGradient(NewRectangle(10, 10, 300, 300), color1, color2, Direction2D_Right);
		// rs->PrintString(
		// 	NewVec2(0, ui->screenSize.y-appData->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 	"Heap: %u/%u used", appData->mainHeap.used, appData->mainHeap.size);
		// rs->PrintString(
		// 	NewVec2(0, ui->screenSize.y-appData->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 	"Line %d Char %d", ui->hoverLocation.lineNum+1, ui->hoverLocation.charIndex);
		// rs->PrintString(
		// 	NewVec2(0, ui->screenSize.y-appData->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 	"Offset: %f", ui->firstRenderLineOffset);
		// rs->PrintString(
		// 	NewVec2(0, ui->screenSize.y-appData->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 	"%s %u:%02u%s (%s %s, %u) [%u]",
		// 	GetDayOfWeekStr(GetDayOfWeek(PlatformInfo->localTime)),
		// 	Convert24HourTo12Hour(PlatformInfo->localTime.hour), PlatformInfo->localTime.minute,
		// 	IsPostMeridian(PlatformInfo->localTime.hour) ? "pm" : "am",
		// 	GetMonthStr((Month_t)PlatformInfo->localTime.month), GetDayOfMonthString(PlatformInfo->localTime.day), PlatformInfo->localTime.year,
		// 	GetTimestamp(PlatformInfo->localTime));
		// rs->PrintString(
		// 	NewVec2(0, ui->screenSize.y-appData->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 	"First: %d Last: %d", firstLine, lastLine);
		// PrintString(appData, appData->testFont,
		// 	NewVec2(0, ui->screenSize.y-appData->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 	"%u Lines Offset: %f (%fpx long)", appData->lineList.numLines, ui->scrollOffset, ui->fileHeight);
		// Line_t* linePntr = GetLineAt(&appData->lineList, ui->hoverLocation.lineNum);
		// if (linePntr != nullptr)
		// {
		// 	Line_t* lineBefore = (Line_t*)linePntr->header.lastItem;
		// 	Line_t* lineAfter = (Line_t*)linePntr->header.nextItem;
		
		// 	rs->PrintString(
		// 		NewVec2(0, ui->screenSize.y-appData->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 		"%08X <- #%u %08X -> %08X", lineBefore, ui->hoverLocation.lineNum, linePntr, lineAfter);
		// }
		
		Line_t* lastLine = GetLastLine(&appData->lineList);
		// DEBUG_PrintLine("Last Item: %p", lastLine->header.lastItem);
		if (lastLine->timestamp == 0 && appData->lineList.numLines > 1)
		{
			lastLine = GetLineAt(&appData->lineList, appData->lineList.numLines-1 - 1);
		}
		
		//Print the status message
		if (GetTimestamp(appData->statusMessageTime) != 0)
		{
			i64 secondsDifference = SubtractTimes(PlatformInfo->localTime, appData->statusMessageTime, TimeUnit_Seconds);
			if (secondsDifference >= 0 && secondsDifference < GC->statusMessageTime)
			{
				Color_t messageColor = GC->colors.foreground;
				
				if (appData->statusMessageType == StatusMessage_Debug)
				{
					messageColor = GC->colors.uiLightGray1;
				}
				else if (appData->statusMessageType == StatusMessage_Info)
				{
					messageColor = GC->colors.foreground;
				}
				else if (appData->statusMessageType == StatusMessage_Success)
				{
					messageColor = GC->colors.highlight2;
				}
				else if (appData->statusMessageType == StatusMessage_Error)
				{
					messageColor = GC->colors.highlight3;
				}
				
				rs->DrawString(appData->statusMessage, NewVec2(5, ui->screenSize.y-appData->testFont.maxExtendDown), messageColor, 1.0f);
			}
		}
		
		//Draw Goto End Button
		{
			Color_t buttonColor = GC->colors.uiGray1;
			Color_t outlineColor = GC->colors.background;
			
			if (IsInsideRectangle(AppInput->mousePos, ui->gotoEndButtonRec))
			{
				if (ButtonDown(MouseButton_Left) &&
					IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], ui->gotoEndButtonRec))
				{
					buttonColor = GC->colors.highlight3;
					outlineColor = GC->colors.foreground;
				}
				else
				{
					buttonColor = GC->colors.uiLightGray1;
				}
			}
			
			rs->DrawButton(ui->gotoEndButtonRec, buttonColor, outlineColor);
		}
		
		if (appData->comPort.isOpen && GC->showComNameInStatusBar)
		{
			v2 comNameSize = MeasureString(&appData->testFont, GC->comPortNames[appData->comPort.index]);
			rs->DrawString(GC->comPortNames[appData->comPort.index],
				NewVec2(ui->gotoEndButtonRec.x - comNameSize.x - 5, ui->screenSize.y-appData->testFont.maxExtendDown),
				GC->colors.foreground, 1.0f);
		}
	}
	
	//+--------------------------------------+
	//|           Render Main Menu           |
	//+--------------------------------------+
	rs->DrawGradient(ui->mainMenuRec, GC->colors.uiGray1, GC->colors.uiGray3, Direction2D_Down);
	rs->DrawRectangle(NewRectangle(0, ui->mainMenuRec.height-1, ui->mainMenuRec.width, 1), GC->colors.uiGray4);
	
	for (u32 bIndex = 0; bIndex < NumMainMenuButtons; bIndex++)
	{
		rec buttonRec = ui->buttonRecs[bIndex];
		Color_t baseColor = {Color_White};
		Color_t highlightColor = {Color_Red};
		Color_t iconColor = {Color_White};
		
		if (IsInsideRectangle(ui->mousePos, buttonRec) && !ui->mouseInMenu)
		{
			if (ButtonDown(MouseButton_Left) && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], buttonRec))
			{
				// iconColor = Color_Highlight2;
				highlightColor = {Color_Black};//Color_Highlight4;
			}
		}
		highlightColor.a = 200;
		
		rs->BindTexture(&ui->buttonBaseTexture);
		rs->DrawTexturedRec(buttonRec, baseColor);
		
		if (IsInsideRectangle(ui->mousePos, buttonRec) && !ui->mouseInMenu)
		{
			rs->BindTexture(&ui->buttonHighlightTexture);
			rs->DrawTexturedRec(buttonRec, highlightColor);
		}
		
		rs->BindTexture(&ui->buttonTextures[bIndex]);
		rs->DrawTexturedRec(buttonRec, iconColor);
	}
	
	// +================================+
	// |         Rx and Tx LEDs         |
	// +================================+
	{
		if (appData->rxTxShiftCountdown == 0)
		{
			appData->rxTxShiftCountdown = GC->rxTxLedDelay;
			
			appData->rxShiftRegister = (u8)(appData->rxShiftRegister >> 1);
			appData->txShiftRegister = (u8)(appData->txShiftRegister >> 1);
		}
		else
		{
			appData->rxTxShiftCountdown--;
		}
		
		Color_t centerColor = GC->colors.uiGray4;
		if ((appData->rxShiftRegister&0x80) > 0 ||
			(appData->rxShiftRegister&0x40) > 0)
		{
			centerColor = GC->colors.receiveLed;
		}
		rs->DrawRectangle(ui->rxLedRec, centerColor);
		centerColor = GC->colors.uiGray4;
		if ((appData->txShiftRegister&0x80) > 0 ||
			(appData->txShiftRegister&0x40) > 0)
		{
			centerColor = GC->colors.transmitLed;
		}
		rs->DrawRectangle(ui->txLedRec, centerColor);
		for (u32 shift = 0; shift < sizeof(u8)*8; shift++)
		{
			if (IsFlagSet(appData->rxShiftRegister, (1<<shift)))
			{
				rec deflatedRec = RectangleInflate(ui->rxLedRec, (r32)(8-shift) * 1);
				rs->DrawButton(deflatedRec, {Color_TransparentBlack}, GC->colors.receiveLed, 1);
			}
			
			if (IsFlagSet(appData->txShiftRegister, (1<<shift)))
			{
				rec deflatedRec = RectangleInflate(ui->txLedRec, (r32)(8-shift) * 1);
				rs->DrawButton(deflatedRec, {Color_TransparentBlack}, GC->colors.transmitLed, 1);
			}
		}
	}
	
	// +================================+
	// |          Clear Button          |
	// +================================+
	{
		const char* clearStr = "Clear";
		v2 textSize = MeasureString(&appData->testFont, clearStr);
		v2 textPos = NewVec2(
			ui->clearButtonRec.x + ui->clearButtonRec.width/2 - textSize.x/2,
			ui->clearButtonRec.y + ui->clearButtonRec.height/2 + appData->testFont.lineHeight/2 - appData->testFont.maxExtendDown
		);
		
		Color_t buttonColor, textColor, borderColor;
		ButtonColorChoice(buttonColor, textColor, borderColor, ui->clearButtonRec, false, false);
		
		rs->DrawButton(ui->clearButtonRec, buttonColor, borderColor);
		rs->DrawString(clearStr, textPos, textColor);
	}
	
	// +==============================+
	// |     Python Running Label     |
	// +==============================+
	if (appData->programInstance.isOpen)
	{
		const char* runningStr = "Py";
		v2 strSize = MeasureString(&appData->testFont, runningStr);
		v2 textPos = NewVec2(
			ui->clearButtonRec.x - strSize.x - 5,
			ui->mainMenuRec.y + appData->testFont.maxExtendUp
		);
		
		rs->DrawString(runningStr, textPos, GC->colors.foreground);
	}
	
	// +==================================+
	// |         Generic Counter          |
	// +==================================+
	if (GetRegularExpression(&appData->regexList, GC->genericCountRegexName) != nullptr)
	{
		rs->PrintString(NewVec2(230, appData->testFont.maxExtendUp + 10), GC->colors.foreground, 1.0f,
						"Counter: %u", appData->genericCounter);
	}
	
	// +==================================+
	// |       Save To File Button        |
	// +==================================+
	if (appData->selectionStart.lineNum != appData->selectionEnd.lineNum ||
		appData->selectionStart.charIndex != appData->selectionEnd.charIndex)
	{
		const char* clearStr = "Save To File";
		v2 textSize = MeasureString(&appData->testFont, clearStr);
		v2 textPos = NewVec2(
			ui->saveButtonRec.x + ui->saveButtonRec.width/2 - textSize.x/2,
			ui->saveButtonRec.y + ui->saveButtonRec.height/2 + appData->testFont.lineHeight/2 - appData->testFont.maxExtendDown
		);
		
		Color_t buttonColor, textColor, borderColor;
		ButtonColorChoice(buttonColor, textColor, borderColor, ui->saveButtonRec, false, false);
		
		rs->DrawButton(ui->saveButtonRec, buttonColor, borderColor);
		rs->DrawString(clearStr, textPos, textColor);
	}
	
	MenuHandlerDrawMenus(PlatformInfo, AppInput, &appData->renderState, &appData->menuHandler);
	
	// +================================+
	// |       Draw Debug Overlay       |
	// +================================+
	#if DEBUG
	if (appData->showDebugMenu)
	{
		rec overlayRec = NewRectangle(10, 10, (r32)PlatformInfo->screenSize.x - 10*2, (r32)PlatformInfo->screenSize.y - 10*2);
		
		rs->DrawButton(overlayRec, ColorTransparent({Color_Black}, 0.5f), ColorTransparent({Color_White}, 0.5f));
		
		v2 textPos = NewVec2(overlayRec.x + 5, overlayRec.y + 5 + appData->testFont.maxExtendUp);
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Input Arena: %u/%u (%.3f%%)",
			appData->inputArena.used, appData->inputArena.size,
			(r32)appData->inputArena.used / (r32)appData->inputArena.size * 100.0f);
		textPos.y += appData->testFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Main Heap: %u/%u (%.3f%%)",
			appData->mainHeap.used, appData->mainHeap.size,
			(r32)appData->mainHeap.used / (r32)appData->mainHeap.size * 100.0f);
		textPos.y += appData->testFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Temp Arena: %u/%u (%.3f%%)",
			ArenaGetHighWaterMark(TempArena), TempArena->size,
			(r32)ArenaGetHighWaterMark(TempArena) / (r32)TempArena->size * 100.0f);
		textPos.y += appData->testFont.lineHeight;
		
	}
	#endif
	
	// Assert(true == false);
	
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Left], AppInput->mouseMaxDist[MouseButton_Left], {Color_Red});
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Right], AppInput->mouseMaxDist[MouseButton_Right], {Color_Blue});
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Middle], AppInput->mouseMaxDist[MouseButton_Middle], {Color_Green});
	
	// rs->DrawRectangle(ui->statusBarRec, {Color_Yellow});
	// rs->DrawRectangle(ui->scrollBarGutterRec, {Color_Red});
	// rs->DrawRectangle(ui->scrollBarRec, {Color_Blue});
	// rs->DrawRectangle(NewRectangle(0, 0, ui->gutterRec.width, screenSize.y - ui->statusBarRec.height), {Color_Orange});
	// rs->DrawRectangle(ui->statusBarRec, {Color_Yellow});
	
	TempPopMark();
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
	GC = &appData->globalConfig;
	TempArena = &appData->tempArena;
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
	GC = &appData->globalConfig;
	TempArena = &appData->tempArena;
	
	DEBUG_WriteLine("Application closing!");
	
	
}

#if (DEBUG && USE_ASSERT_FAILURE_FUNCTION)
//This function is declared in my_assert.h and needs to be implemented by us for a debug build to compile successfully
void AssertFailure(const char* function, const char* filename, int lineNumber, const char* expressionStr)
{
	u32 fileNameStart = 0;
	for (u32 cIndex = 0; filename[cIndex] != '\0'; cIndex++)
	{
		if (filename[cIndex] == '\\' || filename[cIndex] == '/')
		{
			fileNameStart = cIndex+1;
		}
	}
	DEBUG_PrintLine("Assertion Failure! %s in \"%s\" line %d: (%s) is not true", function, &filename[fileNameStart], lineNumber, expressionStr);
}
#endif