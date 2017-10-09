/*
File:   app.cpp
Author: Taylor Robbins
Date:   06\06\2017
Description:
	** Contains all the exported functions and #includes
	** the rest of the source code files.
*/


#include <stdarg.h>
#define USE_ASSERT_FAILURE_FUNCTION true
#if WINDOWS_COMPILATION
#include "win32_assert.h"
#elif OSX_COMPILATION
#include "osx_assert.h"
#else
#error No supported platform defined. No Assert macro used.
#endif

#include "platformInterface.h"
#include "app_version.h"
#include "colors.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "tempMemory.h"
#include "tempMemory.cpp"
#include "linkedList.h"
#include "easing.h"
#include "charClasses.h"

// +--------------------------------------------------------------+
// |                  Platform Global Variables                   |
// +--------------------------------------------------------------+
const PlatformInfo_t* platform = nullptr;
const AppInput_t* input = nullptr;

#include "appHelpers.cpp"

// +--------------------------------------------------------------+
// |                     Application Includes                     |
// +--------------------------------------------------------------+
#include "appDefines.h"
#include "appStructs.h"
#include "appLineList.h"
#include "appRenderState.h"
#include "appMenuHandler.h"
#include "appUiHandler.h"
#include "appRegularExpressions.h"
#include "appConfiguration.h"
#include "appData.h"

// +--------------------------------------------------------------+
// |                 Application Global Variables                 |
// +--------------------------------------------------------------+
AppData_t* app = nullptr;
GlobalConfig_t* GC = nullptr;
v2 RenderScreenSize = {};
v2 RenderMousePos = {};

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
	ClearArray(app->statusMessage);
	va_list args;
	va_start(args, formatString);
	size_t length = vsnprintf(app->statusMessage, ArrayCount(app->statusMessage), formatString, args);
	app->statusMessage[ArrayCount(app->statusMessage)-1] = '\0';
	va_end(args);
	app->statusMessageType = messageType;
	app->statusMessageTime = platform->localTime;
	
	DEBUG_PrintLine("[%s]: %s", functionName, app->statusMessage);
}

#define StatusDebug(formatString, ...)   StatusMessage(__func__, StatusMessage_Debug,   formatString, ##__VA_ARGS__)
#define StatusInfo(formatString, ...)    StatusMessage(__func__, StatusMessage_Info,    formatString, ##__VA_ARGS__)
#define StatusSuccess(formatString, ...) StatusMessage(__func__, StatusMessage_Success, formatString, ##__VA_ARGS__)
#define StatusError(formatString, ...)   StatusMessage(__func__, StatusMessage_Error,   formatString, ##__VA_ARGS__)

// +--------------------------------------------------------------+
// |                   Application Source Files                   |
// +--------------------------------------------------------------+
#include "appConfiguration.cpp"
#include "appFontHelpers.cpp"
#include "appLineList.cpp"
#include "appLoadingFunctions.cpp"
#include "appRenderState.cpp"
#include "appMenuHandler.cpp"
#include "appRenderLine.cpp"
#include "appUiHandler.cpp"
#include "appRegularExpressions.cpp"

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
	CreateLineList(&app->lineList, &app->mainHeap, "");
	
	app->selectionStart = NewTextLocation(0, 0);
	app->selectionEnd = NewTextLocation(0, 0);
	app->uiElements.hoverLocation = NewTextLocation(0, 0);
	
	app->genericCounter = 0;
}

void RefreshComPortList()
{
	BoundedStrListDestroy(&app->availablePorts, &app->mainHeap);
	app->availablePorts = platform->GetComPortListPntr(&app->mainHeap);
	
	StatusDebug("Found %u COM ports", app->availablePorts.count);
	for (u32 cIndex = 0; cIndex < app->availablePorts.count; cIndex++)
	{
		DEBUG_PrintLine("\"%s\"Available!", app->availablePorts[cIndex]);
	}
}

void HideComMenu()
{
	Menu_t* comMenu = GetMenuByName(&app->menuHandler, "COM Menu");
	if (comMenu->show)
	{
		comMenu->show = false;
		if (app->comMenuOptions.name != nullptr) { ArenaPop(&app->mainHeap, app->comMenuOptions.name); }
	}
}

void ShowComMenu()
{
	Menu_t* comMenu = GetMenuByName(&app->menuHandler, "COM Menu");
	if (comMenu->show == false)
	{
		comMenu->show = true;
		app->comMenuOptions = app->comPort;
		if (app->comPort.name != nullptr)
		{
			app->comMenuOptions.name = DupStr(app->comPort.name, &app->mainHeap);
		}
	}
}

bool OpenComPort(const char* comPortName, ComSettings_t settings)
{
	if (app->comPort.isOpen && strcmp(app->comPort.name, comPortName) == 0)
	{
		//NOTE: If we want to open the same port again we have to close it first before opening it
		//      Otherwise we will try to open and only close the port if the open succeeds
		platform->CloseComPortPntr(&app->mainHeap, &app->comPort);
		StatusError("Closed %s", app->comPort.name);
	}
	
	ComPort_t newComPort = platform->OpenComPortPntr(&app->mainHeap, comPortName, settings);
	
	if (newComPort.isOpen)
	{
		if (app->comPort.isOpen)
		{
			platform->CloseComPortPntr(&app->mainHeap, &app->comPort);
			StatusError("Closed %s", app->comPort.name);
		}
		ClearConsole();
		app->comPort = newComPort;
		
		StatusSuccess("\"%s\" Opened Successfully", comPortName);
		return true;
	}
	else
	{
		StatusError("Couldn't open \"%s\"!", comPortName);
		return false;
	}
}

void ComMenuUpdate(MenuHandler_t* menuHandler, Menu_t* menuPntr)
{
	if (ButtonPressed(Button_Escape))
	{
		menuPntr->show = !menuPntr->show;
		if (menuPntr->show)
		{
			RefreshComPortList();
			app->comMenuOptions = app->comPort;
		}
	}
	
	if (menuPntr->show)
	{
		u32 numTabs = app->availablePorts.count + (app->comPort.isOpen ? 1 : 0);
		r32 tabWidth = menuPntr->usableRec.width / numTabs;
		{
			r32 tabMinimumWidth = MeasureString(&app->testFont, "1234567890").x + COM_MENU_TAB_PADDING*2;
			if (tabWidth < tabMinimumWidth) { tabWidth = tabMinimumWidth; }
		}
		rec baudRateRec = NewRectangle(
			menuPntr->usableRec.x + COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + app->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			78, app->testFont.lineHeight * NumBaudRates
		);
		rec numBitsRec = NewRectangle(
			baudRateRec.x + baudRateRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + app->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			55, app->testFont.lineHeight * 8
		);
		rec parityTypesRec = NewRectangle(
			numBitsRec.x + numBitsRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + app->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			60, app->testFont.lineHeight * NumParityTypes
		);
		rec stopBitsRec = NewRectangle(
			parityTypesRec.x + parityTypesRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + app->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			80, app->testFont.lineHeight * NumStopBitTypes
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
		for (u32 tabIndex = ComPort_1; tabIndex < numTabs; tabIndex++)
		{
			char* portName = nullptr;
			if (tabIndex < app->availablePorts.count) { portName = app->availablePorts[tabIndex]; }
			else { portName = app->comPort.name; }
			
			const char* portUserName = GetPortUserName(portName);
			
			v2 givenNameSize = MeasureFormattedString(&app->testFont, portUserName, tabWidth - COM_MENU_TAB_PADDING*2, true);
			if (givenNameSize.y + COM_MENU_TAB_PADDING*2 > tabHeight)
			{
				tabHeight = givenNameSize.y + COM_MENU_TAB_PADDING*2;
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
				baudRateRec.y + baudIndex*app->testFont.lineHeight,
				baudRateRec.width, app->testFont.lineHeight
			);
			if (IsInsideRectangle(RenderMousePos, currentRec) && IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, currentRec) &&
				ButtonReleased(MouseButton_Left))
			{
				app->comMenuOptions.settings.baudRate = (BaudRate_t)baudIndex;
			}
		}
		
		for (i32 bitIndex = 0; bitIndex < 8; bitIndex++)
		{
			rec currentRec = NewRectangle(numBitsRec.x,
				numBitsRec.y + bitIndex*app->testFont.lineHeight,
				numBitsRec.width, app->testFont.lineHeight
			);
			if (IsInsideRectangle(RenderMousePos, currentRec) && IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, currentRec) &&
				ButtonReleased(MouseButton_Left))
			{
				app->comMenuOptions.settings.numBits = (u8)(bitIndex+1);
			}
		}
		
		for (i32 parityIndex = 0; parityIndex < NumParityTypes; parityIndex++)
		{
			rec currentRec = NewRectangle(parityTypesRec.x,
				parityTypesRec.y + parityIndex*app->testFont.lineHeight,
				parityTypesRec.width, app->testFont.lineHeight
			);
			if (IsInsideRectangle(RenderMousePos, currentRec) && IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, currentRec) &&
				ButtonReleased(MouseButton_Left))
			{
				app->comMenuOptions.settings.parity = (Parity_t)parityIndex;
			}
		}
		
		for (i32 stopBitIndex = 0; stopBitIndex < NumStopBitTypes; stopBitIndex++)
		{
			rec currentRec = NewRectangle(stopBitsRec.x,
				stopBitsRec.y + stopBitIndex*app->testFont.lineHeight,
				stopBitsRec.width, app->testFont.lineHeight
			);
			if (IsInsideRectangle(RenderMousePos, currentRec) && IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, currentRec) &&
				ButtonReleased(MouseButton_Left))
			{
				app->comMenuOptions.settings.stopBits = (StopBits_t)stopBitIndex;
			}
		}
		
		//Check for tab Presses
		for (u32 tabIndex = 0; tabIndex < numTabs; tabIndex++)
		{
			char* tabName = nullptr;
			if (tabIndex < app->availablePorts.count) { tabName = app->availablePorts[tabIndex]; }
			else { tabName = app->comPort.name; }
			
			rec tabRec = NewRectangle(tabIndex * tabWidth, 0, tabWidth, tabHeight);
			tabRec.topLeft += menuPntr->usableRec.topLeft;
			
			if (ButtonReleased(MouseButton_Left) && IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, tabRec))
			{
				if (app->comMenuOptions.name != nullptr) { ArenaPop(&app->mainHeap, app->comMenuOptions.name); }
				app->comMenuOptions.name = DupStr(tabName, &app->mainHeap);
				app->comMenuOptions.isOpen = true;
			}
		}
		
		//Check for connect button press
		bool connectButtonPressed = (IsInsideRectangle(RenderMousePos, connectButtonRec) &&
			ButtonReleased(MouseButton_Left) && IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, connectButtonRec));
		if (app->comMenuOptions.isOpen &&
			(connectButtonPressed || ButtonReleased(Button_Enter)))
		{
			if (OpenComPort(app->comMenuOptions.name, app->comMenuOptions.settings))
			{
				menuPntr->show = false;
			}
		}
	}
}
void ComMenuRender(RenderState_t* renderState, MenuHandler_t* menuHandler, Menu_t* menuPntr)
{
	if (menuPntr->show)
	{
		u32 numTabs = app->availablePorts.count + (app->comPort.isOpen ? 1 : 0);
		r32 tabWidth = menuPntr->usableRec.width / numTabs;
		rec baudRateRec = NewRectangle(
			menuPntr->usableRec.x + COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + app->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			78, app->testFont.lineHeight * NumBaudRates
		);
		rec numBitsRec = NewRectangle(
			baudRateRec.x + baudRateRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + app->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			55, app->testFont.lineHeight * 8
		);
		rec parityTypesRec = NewRectangle(
			numBitsRec.x + numBitsRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + app->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			60, app->testFont.lineHeight * NumParityTypes
		);
		rec stopBitsRec = NewRectangle(
			parityTypesRec.x + parityTypesRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + app->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			80, app->testFont.lineHeight * NumStopBitTypes
		);
		
		// +==============================+
		// |   Calculate The Tab Height   |
		// +==============================+
		r32 tabHeight = COM_MENU_TAB_HEIGHT;
		for (u32 tabIndex = ComPort_1; tabIndex < numTabs; tabIndex++)
		{
			char* portName = nullptr;
			if (tabIndex < app->availablePorts.count) { portName = app->availablePorts[tabIndex]; }
			else { portName = app->comPort.name; }
			
			const char* portUserName = GetPortUserName(portName);
			
			v2 givenNameSize = MeasureFormattedString(&app->testFont, portUserName, tabWidth - COM_MENU_TAB_PADDING*2, true);
			if (givenNameSize.y + COM_MENU_TAB_PADDING*2 > tabHeight)
			{
				tabHeight = givenNameSize.y + COM_MENU_TAB_PADDING*2;
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
		renderState->DrawString("Baud Rate", NewVec2(baudRateRec.x, baudRateRec.y - app->testFont.maxExtendDown), GC->colors.foreground);
		renderState->DrawRectangle(baudRateRec, GC->colors.foreground);
		for (i32 baudIndex = 0; baudIndex < NumBaudRates; baudIndex++)
		{
			const char* baudString = GetBaudRateString((BaudRate_t)baudIndex);
			rec currentRec = NewRectangle(baudRateRec.x,
				baudRateRec.y + baudIndex*app->testFont.lineHeight,
				baudRateRec.width, app->testFont.lineHeight
			);
			v2 textPos = baudRateRec.topLeft + NewVec2(
				baudRateRec.width/2 - MeasureString(&app->testFont, baudString).x/2,
				app->testFont.maxExtendUp + baudIndex*app->testFont.lineHeight);
			
			Color_t backColor, textColor, borderColor;
			ButtonColorChoice(backColor, textColor, borderColor, currentRec,
				((BaudRate_t)baudIndex == app->comMenuOptions.settings.baudRate), false);
			
			renderState->DrawRectangle(currentRec, backColor);
			renderState->DrawString(baudString, textPos, textColor);
		}
		
		// +==============================+
		// |   Draw the # Bits Options    |
		// +==============================+
		renderState->DrawString("# Bits", NewVec2(numBitsRec.x, numBitsRec.y - app->testFont.maxExtendDown), {GC->colors.foreground});
		renderState->DrawRectangle(numBitsRec, GC->colors.foreground);
		for (i32 bitIndex = 0; bitIndex < 8; bitIndex++)
		{
			char numBitsString[4] = {};
			snprintf(numBitsString, ArrayCount(numBitsString)-1, "%d", bitIndex+1);
			rec currentRec = NewRectangle(numBitsRec.x,
				numBitsRec.y + bitIndex*app->testFont.lineHeight,
				numBitsRec.width, app->testFont.lineHeight
			);
			v2 textPos = numBitsRec.topLeft + NewVec2(
				numBitsRec.width/2 - MeasureString(&app->testFont, numBitsString).x/2,
				app->testFont.maxExtendUp + bitIndex*app->testFont.lineHeight);
			
			Color_t backColor, textColor, borderColor;
			ButtonColorChoice(backColor, textColor, borderColor, currentRec,
				((u8)(bitIndex+1) == app->comMenuOptions.settings.numBits), false);
			
			renderState->DrawRectangle(currentRec, backColor);
			renderState->DrawString(numBitsString, textPos, textColor);
		}
		
		// +==============================+
		// |   Draw the Parity Options    |
		// +==============================+
		renderState->DrawString("Parity", NewVec2(parityTypesRec.x, parityTypesRec.y - app->testFont.maxExtendDown), {GC->colors.foreground});
		renderState->DrawRectangle(parityTypesRec, GC->colors.foreground);
		for (i32 parityIndex = 0; parityIndex < NumParityTypes; parityIndex++)
		{
			const char* parityString = GetParityString((Parity_t)parityIndex);
			rec currentRec = NewRectangle(parityTypesRec.x,
				parityTypesRec.y + parityIndex*app->testFont.lineHeight,
				parityTypesRec.width, app->testFont.lineHeight
			);
			v2 textPos = parityTypesRec.topLeft + NewVec2(
				parityTypesRec.width/2 - MeasureString(&app->testFont, parityString).x/2,
				app->testFont.maxExtendUp + parityIndex*app->testFont.lineHeight);
			
			Color_t backColor, textColor, borderColor;
			ButtonColorChoice(backColor, textColor, borderColor, currentRec,
				((Parity_t)parityIndex == app->comMenuOptions.settings.parity), false);
			
			renderState->DrawRectangle(currentRec, backColor);
			renderState->DrawString(parityString, textPos, textColor);
		}
		
		// +==============================+
		// |  Draw the Stop Bits Options  |
		// +==============================+
		renderState->DrawString("Stop Bits", NewVec2(stopBitsRec.x, stopBitsRec.y - app->testFont.maxExtendDown), {GC->colors.foreground});
		renderState->DrawRectangle(stopBitsRec, GC->colors.foreground);
		for (i32 stopBitIndex = 0; stopBitIndex < NumStopBitTypes; stopBitIndex++)
		{
			const char* stopBitsString = GetStopBitsString((StopBits_t)stopBitIndex);
			rec currentRec = NewRectangle(stopBitsRec.x,
				stopBitsRec.y + stopBitIndex*app->testFont.lineHeight,
				stopBitsRec.width, app->testFont.lineHeight
			);
			v2 textPos = stopBitsRec.topLeft + NewVec2(
				stopBitsRec.width/2 - MeasureString(&app->testFont, stopBitsString).x/2,
				app->testFont.maxExtendUp + stopBitIndex*app->testFont.lineHeight);
			
			Color_t buttonColor, textColor, borderColor;
			ButtonColorChoice(buttonColor, textColor, borderColor, currentRec,
				((StopBits_t)stopBitIndex == app->comMenuOptions.settings.stopBits), false);
			
			renderState->DrawRectangle(currentRec, buttonColor);
			renderState->DrawString(stopBitsString, textPos, textColor);
		}
		
		// +==============================+
		// |      Draw the Port Tabs      |
		// +==============================+
		//Check for tab Presses
		for (u32 tabIndex = 0; tabIndex < numTabs; tabIndex++)
		{
			char* portName = nullptr;
			if (tabIndex < app->availablePorts.count) { portName = app->availablePorts[tabIndex]; }
			else { portName = app->comPort.name; }
			const char* portUserName = GetPortUserName(portName);
			rec tabRec = NewRectangle(tabIndex * tabWidth, 0, tabWidth, tabHeight);
			tabRec.topLeft += menuPntr->usableRec.topLeft;
			v2 stringSize = MeasureFormattedString(&app->testFont, portUserName, tabWidth - COM_MENU_TAB_PADDING*2, true);
			v2 stringPosition = tabRec.topLeft + NewVec2(tabRec.width/2, tabRec.height/2 - stringSize.y/2 + app->testFont.maxExtendUp);
			
			Color_t buttonColor, textColor, borderColor;
			ButtonColorChoice(buttonColor, textColor, borderColor, tabRec,
				app->comPort.isOpen && strcmp(app->comPort.name, portName) == 0,
				(app->comMenuOptions.isOpen == true && strcmp(app->comMenuOptions.name, portName) == 0));
			
			renderState->DrawButton(tabRec, buttonColor, borderColor);
			// renderState->DrawRectangle(NewRectangle(stringPosition.x - stringSize.x/2, stringPosition.y - app->testFont.maxExtendUp, stringSize.x, stringSize.y), {Color_Red});
			renderState->DrawFormattedString(portUserName, stringPosition, tabWidth - COM_MENU_TAB_PADDING*2, textColor, Alignment_Center, true);
		}
		
		//Draw the connect button
		{
			const char* connectButtonText = "Connect";
			v2 textSize = MeasureString(&app->testFont, connectButtonText);
			v2 stringPosition = NewVec2(
				connectButtonRec.x + connectButtonRec.width/2 - textSize.x/2,
				connectButtonRec.y + connectButtonRec.height/2 + app->testFont.lineHeight/2 - app->testFont.maxExtendDown
			);
			bool settingsHaveChanged = ((!app->comPort.isOpen && app->comMenuOptions.isOpen) || app->comMenuOptions.settings != app->comPort.settings);
			if (app->comMenuOptions.name == nullptr && app->comPort.name != nullptr) { settingsHaveChanged = true; }
			else if (app->comMenuOptions.name != nullptr && app->comPort.name == nullptr) { settingsHaveChanged = true; }
			else if (app->comMenuOptions.name != nullptr && app->comPort.name != nullptr)
			{
				if (strcmp(app->comMenuOptions.name, app->comPort.name) != 0) { settingsHaveChanged = true; }
			}
			
			Color_t buttonColor, borderColor, textColor;
			if (!app->comMenuOptions.isOpen)
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

void ContextMenuUpdate(MenuHandler_t* menuHandler, Menu_t* menu)
{
	UiElements_t* ui = &app->uiElements;
	
	v2 textSize = MeasureString(&app->testFont, ui->contextStringBuffer);
	
	menu->drawRec.size = textSize;
	menu->drawRec = RectangleInflate(menu->drawRec, CONTEXT_MENU_PADDING);
	menu->drawRec.topLeft = RenderMousePos + NewVec2(0, -3 - menu->drawRec.height);
}
void ContextMenuRender(RenderState_t* renderState, MenuHandler_t* menuHandler, Menu_t* menu)
{
	UiElements_t* ui = &app->uiElements;
	
	v2 textPos = menu->usableRec.topLeft + NewVec2(CONTEXT_MENU_PADDING, CONTEXT_MENU_PADDING + app->testFont.maxExtendUp);
	app->renderState.DrawString(ui->contextStringBuffer, textPos, GC->colors.foreground);
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
			app->genericCounter++;
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
	Line_t* lastLine = GetLastLine(&app->lineList);
	
	for (i32 cIndex = 0; cIndex < numBytes; cIndex++)
	{
		char newChar = dataBuffer[cIndex];
		if (newChar == '\n')
		{
			Line_t* finishedLine = lastLine;
			
			if (finishedLine->timestamp == 0)
			{
				finishedLine->timestamp = GetTimestamp(platform->localTime);
			}
			
			if (app->writeToFile)
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
				
				platform->AppendFilePntr(&app->outputFile, timestampBuffer, timestampLength);
				
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
				platform->AppendFilePntr(&app->outputFile, finishedLine->chars, finishedLine->numChars);
				platform->AppendFilePntr(&app->outputFile, "\r\n", 2);
				
				LineReset(&app->lineList, finishedLine);
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
						bool appliedToThisComPort = (trigger->numComPorts == 0 || app->comPort.isOpen == false);
						if (appliedToThisComPort == false)
						{
							for (u32 comListIndex = 0; comListIndex < trigger->numComPorts; comListIndex++)
							{
								const char* supportedName = trigger->comPorts[comListIndex];
								if (strcmp(supportedName, app->comPort.name) == 0 ||
									strcmp(supportedName, GetPortUserName(app->comPort.name)) == 0)
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
								regexStr = GetRegularExpression(&app->regexList, trigger->expressionName);
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
					lastLine = AddLineToList(&app->lineList, "");
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
			LineAppend(&app->lineList, lastLine, newChar);
		}
	}
	
	app->rxShiftRegister |= 0x80;
}

//NOTE: This function serves as a measuring function AS WELL AS
//		a buffer filling function if not passed nullptr for bufferOutput
u32 GetSelection(char* bufferOutput = nullptr)
{
	TextLocation_t minLocation = TextLocationMin(app->selectionStart, app->selectionEnd);
	TextLocation_t maxLocation = TextLocationMax(app->selectionStart, app->selectionEnd);
	
	if (minLocation.lineNum == maxLocation.lineNum &&
		minLocation.charIndex == maxLocation.charIndex)
	{
		//No selection made
		return 0;
	}
	
	u8 newLineSize = (platform->platformType == Platform_Windows) ? 2 : 1;
	char newLine[2] = {};
	if (platform->platformType == Platform_Windows)
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
		Line_t* linePntr = GetLineAt(&app->lineList, minLocation.lineNum);
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
			Line_t* minLinePntr = GetLineAt(&app->lineList, minLocation.lineNum);
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
		for (i32 lineIndex = minLocation.lineNum+1; lineIndex < maxLocation.lineNum && lineIndex < app->lineList.numLines; lineIndex++)
		{
			Line_t* linePntr = GetLineAt(&app->lineList, lineIndex);
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
			Line_t* maxLinePntr = GetLineAt(&app->lineList, maxLocation.lineNum);
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
EXPORT AppGetVersion_DEFINITION(App_GetVersion)
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
EXPORT AppInitialize_DEFINITION(App_Initialize)
{
	platform = PlatformInfo;
	app = (AppData_t*)AppMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	
	DEBUG_WriteLine("Initializing Game...");
	
	// +==================================+
	// |          Memory Arenas           |
	// +==================================+
	Assert(AppMemory->permanantSize > INPUT_ARENA_SIZE);
	ClearPointer(app);
	
	u8* extraSpaceStart = ((u8*)app) + sizeof(AppData_t);
	
	InitializeMemoryArenaLinear(&app->inputArena, extraSpaceStart, INPUT_ARENA_SIZE);
	
	u32 mainHeapSize = AppMemory->permanantSize - sizeof(AppData_t) - INPUT_ARENA_SIZE;
	InitializeMemoryArenaHeap(&app->mainHeap, extraSpaceStart + INPUT_ARENA_SIZE, mainHeapSize);
	
	InitializeMemoryArenaTemp(&app->tempArena, AppMemory->transientPntr, AppMemory->transientSize, TRANSIENT_MAX_NUMBER_MARKS);
	
	DEBUG_PrintLine("Input Arena: %u bytes", INPUT_ARENA_SIZE);
	DEBUG_PrintLine("Main Heap:   %u bytes", mainHeapSize);
	DEBUG_PrintLine("Temp Arena:  %u bytes", AppMemory->transientSize);
	
	TempPushMark();
	
	// +================================+
	// |    External Initializations    |
	// +================================+
	LoadGlobalConfiguration(platform, &app->globalConfig, &app->mainHeap);
	InitializeUiElements(&app->uiElements);
	InitializeRenderState(&app->renderState);
	InitializeMenuHandler(&app->menuHandler, &app->mainHeap);
	InitializeRegexList(&app->regexList, &app->mainHeap);
	LoadRegexFile(&app->regexList, "Resources/Configuration/RegularExpressions.rgx", &app->mainHeap);
	CreateLineList(&app->lineList, &app->mainHeap, "");
	
	DEBUG_WriteLine("Creating menus");
	
	Menu_t* comMenu = AddMenu(&app->menuHandler, "COM Menu", NewRectangle((r32)RenderScreenSize.x / 2 - 50, (r32)RenderScreenSize.y / 2 - 150, 400, 300),
		ComMenuUpdate, ComMenuRender);
	comMenu->show = false;
	Menu_t* contextMenu = AddMenu(&app->menuHandler, "Context Menu", NewRectangle(0, 0, 100, 100),
		ContextMenuUpdate, ContextMenuRender);
	contextMenu->titleBarSize = 0;
	contextMenu->show = true;
	
	// +================================+
	// |          Load Content          |
	// +================================+
	DEBUG_WriteLine("Compiling shaders...");
	app->simpleShader = LoadShader(
		"Resources/Shaders/simple-vertex.glsl",
		"Resources/Shaders/simple-fragment.glsl");
	app->outlineShader = LoadShader(
		"Resources/Shaders/outline-vertex.glsl",
		"Resources/Shaders/outline-fragment.glsl");
	
	app->testTexture = LoadTexture("Resources/Sprites/buttonIcon3.png");
	app->scrollBarEndcapTexture = LoadTexture("Resources/Sprites/scrollBarEndcap.png", false, false);
	
	app->testFont = LoadFont("Resources/Fonts/consola.ttf",
		(r32)GC->fontSize, 1024, 1024, ' ', 96);
	
	// +================================+
	// |          Frame Buffer          |
	// +================================+
	DEBUG_WriteLine("Creating post-processing Frame Buffer");
	app->frameTexture = CreateTexture(nullptr, 2048, 2048);
	app->frameBuffer = CreateFrameBuffer(&app->frameTexture);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		DEBUG_WriteLine("FrameBuffer incomplete!");
	}
	
	// +================================+
	// |      Other Initialization      |
	// +================================+
	DEBUG_WriteLine("Initializing COM ports");
	app->comPort.settings.baudRate = BaudRate_115200;
	app->comPort.settings.parity = Parity_None;
	app->comPort.settings.stopBits = StopBits_1;
	app->comPort.settings.numBits = 8;
	
	RefreshComPortList();
	
	TempPopMark();
	DEBUG_WriteLine("Initialization Done!");
	app->appInitTempHighWaterMark = ArenaGetHighWaterMark(TempArena);
	ArenaResetHighWaterMark(TempArena);
}

//+================================================================+
//|                        App Reloaded                            |
//+================================================================+
EXPORT AppReloaded_DEFINITION(App_Reloaded)
{
	platform = PlatformInfo;
	app = (AppData_t*)AppMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	
	StatusDebug("App Reloaded");
	
	//Make sure our callbacks still match the location of the functions in the new DLL
	Menu_t* menuPntr = GetMenuByName(&app->menuHandler, "COM Menu");
	menuPntr->specialPntr = nullptr;
	menuPntr->updateFunctionPntr = (void*)ComMenuUpdate;
	menuPntr->renderFunctionPntr = (void*)ComMenuRender;
	menuPntr = GetMenuByName(&app->menuHandler, "Context Menu");
	menuPntr->specialPntr = nullptr;
	menuPntr->updateFunctionPntr = (void*)ContextMenuUpdate;
	menuPntr->renderFunctionPntr = (void*)ContextMenuRender;
}

//+================================================================+
//|                         App Update                             |
//+================================================================+
EXPORT AppUpdate_DEFINITION(App_Update)
{
	platform = PlatformInfo;
	input = AppInput;
	app = (AppData_t*)AppMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	RenderMousePos = NewVec2(input->mousePos.x / GUI_SCALE, input->mousePos.y / GUI_SCALE);
	
	UiElements_t* ui = &app->uiElements;
	RenderState_t* rs = &app->renderState;
	Menu_t* comMenu = GetMenuByName(&app->menuHandler, "COM Menu");
	Menu_t* contextMenu = GetMenuByName(&app->menuHandler, "Context Menu");
	Menu_t* hoverMenu = GetMenuAtPoint(&app->menuHandler, RenderMousePos);
	Color_t color1 = ColorFromHSV((i32)(platform->programTime*180) % 360, 1.0f, 1.0f);
	Color_t color2 = ColorFromHSV((i32)(platform->programTime*180 + 125) % 360, 1.0f, 1.0f);
	Color_t selectionColor = ColorLerp(GC->colors.selection1, GC->colors.selection2, (Sin32((r32)platform->programTime*6.0f) + 1.0f) / 2.0f);
	Color_t hoverLocColor  = ColorLerp(GC->colors.foreground, GC->colors.background, (Sin32((r32)platform->programTime*8.0f) + 1.0f) / 2.0f);
	// Color_t selectionColor = ColorFromHSV(180, 1.0f, (r32)(Sin32((r32)platform->programTime*5) + 1.0f) / 2.0f);
	
	// +==============================+
	// |  Clear Button Handled Array  |
	// +==============================+
	for (u32 bIndex = 0; bIndex < ArrayCount(app->buttonHandled); bIndex++)
	{
		app->buttonHandled[bIndex] = false;
	}
	
	// +==============================+
	// | Temp Arena Update Loop Push  |
	// +==============================+
	TempPushMark();
	
	// +==============================+
	// |     Set the Window Title     |
	// +==============================+
	ClearArray(AppOutput->windowTitle);
	if (app->comPort.isOpen)
	{
		snprintf(AppOutput->windowTitle, sizeof(AppOutput->windowTitle)-1,
			"%s - Const Port", GetPortUserName(app->comPort.name));
	}
	else
	{
		snprintf(AppOutput->windowTitle, sizeof(AppOutput->windowTitle)-1,
			"Const Port [Disconnected]");
	}
	
	RecalculateUiElements(ui, true);
	ui->mouseInMenu = (hoverMenu != nullptr && hoverMenu != contextMenu);
	
	//+================================+
	//|  Context Menu Showing/Filling  |
	//+================================+
	contextMenu->show = false;
	if (ui->mouseInMenu == false &&
		ButtonDown(Button_Control) &&// && mousePos.x <= ui->gutterRec.width)
		(IsInsideRectangle(RenderMousePos, ui->viewRec) || IsInsideRectangle(RenderMousePos, ui->gutterRec)))
	{
		Line_t* linePntr = GetLineAt(&app->lineList, ui->hoverLocation.lineNum);
		
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
				i64 secondsDifference = SubtractTimes(platform->localTime, lineTime, TimeUnit_Seconds);
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
	if (app->comPort.isOpen)
	{
		i32 readResult = 1;
		while (readResult > 0)
		{
			char buffer[4096] = {};
			readResult = platform->ReadComPortPntr(&app->comPort, buffer, ArrayCount(buffer)-1);
			if (readResult > 0)
			{
				// DEBUG_PrintLine("Read %d bytes \"%.*s\"", readResult, readResult, buffer);
				
				if (app->programInstance.isOpen == false ||
					GC->sendComDataToPython == false ||
					GC->alsoShowComData == true)
				{
					DataReceived(buffer, readResult);
				}
				
				if (app->programInstance.isOpen && GC->sendComDataToPython)
				{
					DEBUG_PrintLine("Writing to program instance \"%.*s\"", readResult, buffer);
					
					u32 numBytesWritten = platform->WriteProgramInputPntr(&app->programInstance, &buffer[0], readResult);
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
	if (app->programInstance.isOpen)
	{
		char readBuffer[256] = {};
		
		u32 numBytesRead = 1;
		while (numBytesRead > 0)
		{
			numBytesRead = platform->ReadProgramOutputPntr(&app->programInstance, readBuffer, ArrayCount(readBuffer));
			
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
		
		ProgramStatus_t programStatus = platform->GetProgramStatusPntr(&app->programInstance);
		if (programStatus != ProgramStatus_Running)
		{
			DEBUG_WriteLine("Program instance finished!");
			platform->CloseProgramInstancePntr(&app->programInstance);
		}
	}
	
	// +============================================+
	// | Read Keyboard Input and Route Accordingly  |
	// +============================================+
	if (true)
	{
		bool writeToComPort = (app->comPort.isOpen && (app->programInstance.isOpen == false || GC->sendInputToPython == false || GC->alsoSendInputToCom));
		bool writeToProgram = (app->programInstance.isOpen && GC->sendInputToPython);
		bool echoInput = GC->autoEchoInput;
		
		if (input->textInputLength > 0)
		{
			// DEBUG_PrintLine("Writing \"%.*s\"", input->textInputLength, input->textInput);
			if (echoInput) { DataReceived(&input->textInput[0], input->textInputLength); }
			if (writeToComPort)
			{
				platform->WriteComPortPntr(&app->comPort, &input->textInput[0], input->textInputLength);
				app->txShiftRegister |= 0x80;
			}
			if (writeToProgram) { platform->WriteProgramInputPntr(&app->programInstance, &input->textInput[0], input->textInputLength); }
		}
		
		if (comMenu->show == false && ButtonPressed(Button_Enter))
		{
			DEBUG_WriteLine("Writing New Line");
			
			char newChar = '\n';
			if (echoInput) { DataReceived("\n", 1); }
			if (writeToComPort)
			{
				platform->WriteComPortPntr(&app->comPort, &newChar, 1);
				app->txShiftRegister |= 0x80;
			}
			if (writeToProgram) { platform->WriteProgramInputPntr(&app->programInstance, &newChar, 1); }
		}
		
		if (!comMenu->show && ButtonPressed(Button_Backspace))
		{
			DEBUG_WriteLine("Writing Backspace");
			
			char newChar = '\b';
			if (echoInput) { DataReceived("\b", 1); }
			if (writeToComPort)
			{
				platform->WriteComPortPntr(&app->comPort, &newChar, 1);
				app->txShiftRegister |= 0x80;
			}
			if (writeToProgram)
			{
				platform->WriteProgramInputPntr(&app->programInstance, &newChar, 1);
			}
		}
	}
	
	//+==================================+
	//|        Recenter COM menu         |
	//+==================================+
	if (platform->windowResized)
	{
		comMenu->drawRec.topLeft = NewVec2(
			RenderScreenSize.x / 2 - comMenu->drawRec.width/2,
			RenderScreenSize.y / 2 - comMenu->drawRec.height/2);
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
			HideComMenu();
		}
		else
		{
			RefreshComPortList();
			ShowComMenu();
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
				
				platform->CopyToClipboardPntr(selectionTempBuffer, selectionSize);
				
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
		DisposeGlobalConfig(&app->globalConfig);
		LoadGlobalConfiguration(platform, &app->globalConfig, &app->mainHeap);
		DisposeRegexFile(&app->regexList);
		LoadRegexFile(&app->regexList, "Resources/Configuration/RegularExpressions.rgx", &app->mainHeap);
	}
	
	//+==================================+
	//|       Select/Deselect all        |
	//+==================================+
	if (ButtonPressed(Button_A) && ButtonDown(Button_Control))
	{
		if (app->selectionStart.lineNum == app->selectionEnd.lineNum &&
			app->selectionStart.charIndex == app->selectionEnd.charIndex)
		{
			//Select all
			app->selectionStart = NewTextLocation(0, 0);
			Line_t* lastLinePntr = GetLastLine(&app->lineList);
			app->selectionEnd = NewTextLocation(app->lineList.numLines-1, lastLinePntr->numChars);
			
			StatusInfo("Selected Everything");
		}
		else
		{
			//Deselect all
			app->selectionStart = NewTextLocation(0, 0);
			app->selectionEnd = NewTextLocation(0, 0);
			
			StatusError("Unselected Everything");
		}
	}
	
	// +==================================+
	// |      Toggle Output To File       |
	// +==================================+
	if (ButtonPressed(Button_F) && ButtonDown(Button_Control))
	{
		char* outputFileName = TempCombine(GetPortUserName(app->comPort.name), "_Output.txt");
		
		DEBUG_PrintLine("Outputting to file: \"%s\"", outputFileName);
		
		if (app->writeToFile)
		{
			platform->CloseFilePntr(&app->outputFile);
			app->writeToFile = false;
			StatusSuccess("Stopped outputting to file");
		}
		else
		{
			if (platform->OpenFilePntr(outputFileName, &app->outputFile))
			{
				StatusSuccess("Opened file successfully");
				app->writeToFile = true;
				const char* newString = "\r\n\r\n[File Opened for Writing]\r\n";
				platform->AppendFilePntr(&app->outputFile, newString, (u32)strlen(newString));
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
		const char* quickComSelection = nullptr;
		
		if (ButtonReleased(Button_1)) quickComSelection = "COM1";
		if (ButtonReleased(Button_2)) quickComSelection = "COM2";
		if (ButtonReleased(Button_3)) quickComSelection = "COM3";
		if (ButtonReleased(Button_4)) quickComSelection = "COM4";
		if (ButtonReleased(Button_5)) quickComSelection = "COM5";
		if (ButtonReleased(Button_6)) quickComSelection = "COM6";
		if (ButtonReleased(Button_7)) quickComSelection = "COM7";
		if (ButtonReleased(Button_8)) quickComSelection = "COM8";
		if (ButtonReleased(Button_9)) quickComSelection = "COM9";
		
		if (quickComSelection != nullptr)
		{
			if (comMenu->show) HideComMenu();
			
			RefreshComPortList();
			
			if (IsComAvailable(quickComSelection))
			{
				StatusError("%s not Available!", quickComSelection);
			}
			else
			{
				OpenComPort(quickComSelection, app->comPort.settings);
			}
		}
	}
	
	// +================================+
	// |      Toggle Debug Overlay      |
	// +================================+
	if (ButtonPressed(Button_F11))
	{
		app->showDebugMenu = !app->showDebugMenu;
	}
	
	RecalculateUiElements(ui, false);
	
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
	if (input->scrollDelta.y != 0)
	{
		if (ButtonDown(Button_Shift))
		{
			ui->scrollOffsetGoto.x -= input->scrollDelta.y * (r32)GC->scrollMultiplier;
		}
		else
		{
			ui->scrollOffsetGoto.y -= input->scrollDelta.y * (r32)GC->scrollMultiplier;
			
			if (input->scrollDelta.y > 0)
			{
				ui->followingEndOfFile = false;
			}
		}
	}
	if (input->scrollDelta.x != 0)
	{
		ui->scrollOffsetGoto.x -= input->scrollDelta.x * (r32)GC->scrollMultiplier;
	}
	
	bool gotoEndButtonPressed = (IsInsideRectangle(RenderMousePos, ui->gotoEndButtonRec) &&
		ButtonReleased(MouseButton_Left) &&
		IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, ui->gotoEndButtonRec));
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
		if (app->programInstance.isOpen)
		{
			StatusInfo("Closing python instance");
			platform->CloseProgramInstancePntr(&app->programInstance);
		}
		else
		{
			if (GC->pythonScript != nullptr)
			{
				char* commandStr = TempPrint("python %s", GC->pythonScript);
				StatusInfo("Running System Command: \"%s\"", commandStr);
				app->programInstance = platform->StartProgramInstancePntr(commandStr);
				if (app->programInstance.isOpen == false)
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
			const char* countExpression = GetRegularExpression(&app->regexList, GC->genericCountRegexName);
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
		ButtonReleased(MouseButton_Left) && IsInsideRectangle(RenderMousePos, ui->mainMenuRec) &&
		IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, ui->mainMenuRec))
	{
		for (u32 bIndex = 0; bIndex < ArrayCount(ui->buttonRecs); bIndex++)
		{
			if (IsInsideRectangle(RenderMousePos, ui->buttonRecs[bIndex]) &&
				IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, ui->buttonRecs[bIndex]))
			{
				switch (bIndex)
				{
					case Button_ComPort:
					{
						if (comMenu->show)
						{
							HideComMenu();
						}
						else
						{
							RefreshComPortList();
							ShowComMenu();
						}
					} break;
					
					case Button_Settings:
					{
						//TODO: Create file if it doesn't exist
						if (ButtonDown(Button_Shift) && ButtonDown(Button_Control))
						{
							platform->LaunchFilePntr("Resources\\Configuration\\PlatformConfig.json");
							platform->LaunchFilePntr("Resources\\Configuration\\RegularExpressions.rgx");
							platform->LaunchFilePntr("Resources\\Configuration\\GlobalConfig.json");
						}
						else if (ButtonDown(Button_Shift))
						{
							platform->LaunchFilePntr("Resources\\Configuration\\PlatformConfig.json");
						}
						else if (ButtonDown(Button_Control))
						{
							platform->LaunchFilePntr("Resources\\Configuration\\RegularExpressions.rgx");
						}
						else
						{
							platform->LaunchFilePntr("Resources\\Configuration\\GlobalConfig.json");
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
	if (IsInsideRectangle(RenderMousePos, ui->clearButtonRec))
	{
		if (ButtonReleased(MouseButton_Left) &&
			IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, ui->clearButtonRec))
		{
			ClearConsole();
		}
	}
	
	//+==================================+
	//|    Save To File Button Press     |
	//+==================================+
	bool saveButtonPressed = (IsInsideRectangle(RenderMousePos, ui->saveButtonRec) &&
		ButtonReleased(MouseButton_Left) &&
		IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, ui->saveButtonRec));
	if (saveButtonPressed ||
		(ButtonDown(Button_Control) && ButtonPressed(Button_S)))
	{
		char fileNameBuffer[256] = {};
		u32 fileNameLength = snprintf(&fileNameBuffer[0], sizeof(fileNameBuffer),
			"ConstPortSave_%02u-%02u-%u_%u-%02u-%02u.txt",
			platform->localTime.year, platform->localTime.month, platform->localTime.day,
			platform->localTime.hour, platform->localTime.minute, platform->localTime.second);
		
		u32 selectionSize = GetSelection(nullptr);
		if (selectionSize > 0)
		{
			TempPushMark();
			
			char* fileBuffer = TempString(selectionSize);
			GetSelection(fileBuffer);
			
			//NOTE: GetSelection adds a \0 on the end so need to remove it
			DEBUG_PrintLine("Saving %u bytes to %s", selectionSize-1, fileNameBuffer);
			platform->WriteEntireFilePntr(fileNameBuffer, fileBuffer, selectionSize-1);
			DEBUG_WriteLine("Done!");
			
			TempPopMark();
			
			if (platform->LaunchFilePntr(fileNameBuffer))
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
		if (IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, ui->scrollBarGutterRec) &&
			ui->scrollBarRec.height < ui->scrollBarGutterRec.height)
		{
			if (input->buttons[MouseButton_Left].transCount > 0)//Pressed the button down
			{
				ui->mouseScrollbarOffset = RenderMousePos.y - ui->scrollBarRec.y;
				if (IsInsideRectangle(RenderMousePos, ui->scrollBarRec))
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
				r32 newPixelLocation = RenderMousePos.y - ui->mouseScrollbarOffset - ui->scrollBarGutterRec.y;
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
				
				if (ui->scrollOffsetGoto.y < ui->maxScrollOffset.y - app->testFont.lineHeight)
				{
					ui->followingEndOfFile = false;
				}
			}
		}
		else if (IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, ui->viewRec))
		{
			if (input->buttons[MouseButton_Left].transCount > 0)//Pressed the button down
			{
				app->selectionStart = ui->hoverLocation;
				app->selectionEnd = ui->hoverLocation;
			}
			else //if (IsInsideRectangle(RenderMousePos, ui->viewRec)) //Mouse Button Holding
			{
				app->selectionEnd = ui->hoverLocation;
			}
		}
	}
	
	//+================================+
	//|   Mark lines using the mouse   |
	//+================================+
	if (ui->mouseInMenu == false &&
		IsInsideRectangle(RenderMousePos, ui->gutterRec) && IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, ui->gutterRec))
	{
		if (ButtonReleased(MouseButton_Left) &&
			ui->markIndex >= 0 && ui->markIndex < app->lineList.numLines)
		{
			Line_t* linePntr = GetLineAt(&app->lineList, ui->markIndex);
			
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
		i32 lastIndex = app->lineList.numLines - 1;
		Line_t* linePntr = GetLineAt(&app->lineList, lastIndex);
		if (linePntr->numChars == 0 && lastIndex > 0)
		{
			linePntr = GetLineAt(&app->lineList, lastIndex - 1);
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
	MenuHandlerUpdate(&app->menuHandler);
	
	UpdateUiElements(ui);
	RecalculateUiElements(ui, false);
	if (ui->followingEndOfFile)
	{
		ui->scrollOffsetGoto.y = ui->maxScrollOffset.y;
	}
	
	//+==================================+
	//|           Cursor Type            |
	//+==================================+
	if (IsInsideRectangle(RenderMousePos, ui->viewRec) && !ui->mouseInMenu && GC->showTextCursor)
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
	rs->SetViewport(NewRectangle(0, 0, (r32)platform->screenSize.x, (r32)platform->screenSize.y));
	
	glClearColor((GC->colors.background.r/255.f), (GC->colors.background.g/255.f), (GC->colors.background.b/255.f), 1.0f);
	// glClearColor((200/255.f), (200/255.f), (200/255.f), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	rs->BindFrameBuffer(nullptr);
	rs->BindShader(&app->simpleShader);
	rs->BindFont(&app->testFont);
	rs->SetGradientEnabled(false);
	
	Matrix4_t worldMatrix, viewMatrix, projMatrix;
	viewMatrix = Matrix4_Identity;
	projMatrix = Matrix4Scale(NewVec3(2.0f/RenderScreenSize.x, -2.0f/RenderScreenSize.y, 1.0f));
	projMatrix = Mat4Mult(projMatrix, Matrix4Translate(NewVec3(-RenderScreenSize.x/2.0f, -RenderScreenSize.y/2.0f, 0.0f)));
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
			
			v2 currentPos = NewVec2((r32)GC->lineSpacing, ui->scrollOffset.y - ui->firstRenderLineOffset + app->testFont.maxExtendUp);
			for (i32 lineIndex = firstLine; lineIndex < app->lineList.numLines; lineIndex++)
			{
				Line_t* linePntr = GetLineAt(&app->lineList, lineIndex);
				
				r32 lineHeight = RenderLine(linePntr, currentPos, true);
				//Draw line highlight
				if (GC->highlightHoverLine &&
					lineIndex == ui->hoverLocation.lineNum &&
					IsInsideRectangle(RenderMousePos, ui->viewRec) &&
					!ui->mouseInMenu)
				{
					rec backRec = NewRectangle(
						currentPos.x + ui->scrollOffset.x,
						currentPos.y - app->testFont.maxExtendUp,
						ui->viewRec.width, lineHeight
					);
					rs->DrawRectangle(backRec, GC->colors.hoverLine);
				}
				
				RenderLine(linePntr, currentPos, false);
				
				if (GC->showHoverCursor &&
					lineIndex == ui->hoverLocation.lineNum &&
					IsInsideRectangle(RenderMousePos, ui->viewRec) &&
					!ui->mouseInMenu)
				{
					v2 skipSize = MeasureString(&app->testFont, linePntr->chars, ui->hoverLocation.charIndex);
					rec cursorRec = NewRectangle(
						currentPos.x + skipSize.x,
						currentPos.y - app->testFont.maxExtendUp,
						1, app->testFont.lineHeight
					);
					rs->DrawRectangle(cursorRec, hoverLocColor);
				}
				
				currentPos.y += lineHeight + GC->lineSpacing;
				if (currentPos.y - app->testFont.maxExtendUp >= ui->scrollOffset.y + ui->viewRec.height)
				{
					//We've reached the bottom of the view
					break;
				}
			}
			
		}
		rs->SetViewMatrix(Matrix4_Identity);
	}
	
	rs->BindFrameBuffer(&app->frameBuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//+--------------------------------------+
	//|          Render Selection            |
	//+--------------------------------------+
	if (app->selectionStart.lineNum != app->selectionEnd.lineNum ||
		app->selectionStart.charIndex != app->selectionEnd.charIndex)
	{
		TextLocation_t minLocation = TextLocationMin(app->selectionStart, app->selectionEnd);
		TextLocation_t maxLocation = TextLocationMax(app->selectionStart, app->selectionEnd);
		i32 firstLine = max(0, ui->firstRenderLine);
		
		rs->SetViewMatrix(Matrix4Translate(NewVec3(ui->viewRec.x - ui->scrollOffset.x, ui->viewRec.y - ui->scrollOffset.y, 0)));
		{//Items drawn relative to view
			
			v2 currentPos = NewVec2(0, ui->scrollOffset.y - ui->firstRenderLineOffset + app->testFont.maxExtendUp);
			for (i32 lineIndex = firstLine;
				lineIndex < app->lineList.numLines && lineIndex <= maxLocation.lineNum;
				lineIndex++)
			{
				Line_t* linePntr = GetLineAt(&app->lineList, lineIndex);
				
				if (lineIndex >= minLocation.lineNum && lineIndex <= maxLocation.lineNum)
				{
					v2 skipSize = Vec2_Zero;
					v2 selectionSize = Vec2_Zero;
					
					if (lineIndex == minLocation.lineNum &&
						lineIndex == maxLocation.lineNum)
					{
						skipSize = MeasureString(&app->testFont, linePntr->chars, minLocation.charIndex);
						selectionSize = MeasureString(&app->testFont, &linePntr->chars[minLocation.charIndex], maxLocation.charIndex - minLocation.charIndex);
					}
					else if (lineIndex == minLocation.lineNum)
					{
						skipSize = MeasureString(&app->testFont, linePntr->chars, minLocation.charIndex);
						selectionSize = MeasureString(&app->testFont, &linePntr->chars[minLocation.charIndex]);
						// selectionSize.x += MeasureString(&app->testFont, " ", 1).x;
					}
					else if (lineIndex == maxLocation.lineNum)
					{
						selectionSize = MeasureString(&app->testFont, linePntr->chars, maxLocation.charIndex);
					}
					else
					{
						selectionSize = MeasureString(&app->testFont, linePntr->chars);
						// selectionSize.x += MeasureString(&app->testFont, " ", 1).x;
					}
					
					rec backRec = NewRectangle(GC->lineSpacing + currentPos.x + skipSize.x, currentPos.y - app->testFont.maxExtendUp, selectionSize.x, app->testFont.lineHeight);//linePntr->lineHeight);
					backRec = RectangleInflate(backRec, (r32)GC->lineSpacing/2);
					rs->DrawRectangle(backRec, selectionColor);
					
					if (currentPos.y - app->testFont.maxExtendUp >= ui->scrollOffset.y + ui->viewRec.height)
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
	rs->BindShader(&app->outlineShader);
	rs->UpdateShader();
	
	rs->SetSecondaryColor(NewColor(0, 0, 0, 20)); //TODO: Add this as a configuration option
	rs->BindTexture(&app->frameTexture);
	rs->DrawTexturedRec(NewRectangle(0, RenderScreenSize.y, (r32)app->frameTexture.width, (r32)-app->frameTexture.height), {Color_White});
	
	rs->BindShader(&app->simpleShader);
	rs->UpdateShader();
	
	//+--------------------------------------+
	//|    Render Line Gutter Elements       |
	//+--------------------------------------+
	rs->DrawGradient(ui->gutterRec, GC->colors.uiGray1, GC->colors.uiGray3, Direction2D_Right);
	{
		i32 firstLine = max(0, ui->firstRenderLine);
		
		rs->SetViewMatrix(Matrix4Translate(NewVec3(ui->gutterRec.x, ui->gutterRec.y - ui->scrollOffset.y, 0)));
		{//Items drawn relative to view
			
			v2 currentPos = NewVec2(0, ui->scrollOffset.y - ui->firstRenderLineOffset + app->testFont.maxExtendUp);
			for (i32 lineIndex = firstLine; lineIndex < app->lineList.numLines; lineIndex++)
			{
				Line_t* linePntr = GetLineAt(&app->lineList, lineIndex);
				
				RenderLineGutter(linePntr, currentPos, lineIndex, linePntr->lineHeight);
				
				currentPos.y += linePntr->lineHeight + GC->lineSpacing;
				if (currentPos.y - app->testFont.maxExtendUp >= ui->scrollOffset.y + ui->viewRec.height)
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
		rs->BindAlphaTexture(&app->scrollBarEndcapTexture);
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
		// 	NewVec2(0, RenderScreenSize.y-app->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 	"Heap: %u/%u used", app->mainHeap.used, app->mainHeap.size);
		// rs->PrintString(
		// 	NewVec2(0, RenderScreenSize.y-app->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 	"Line %d Char %d", ui->hoverLocation.lineNum+1, ui->hoverLocation.charIndex);
		// rs->PrintString(
		// 	NewVec2(0, RenderScreenSize.y-app->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 	"Offset: %f", ui->firstRenderLineOffset);
		// rs->PrintString(
		// 	NewVec2(0, RenderScreenSize.y-app->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 	"%s %u:%02u%s (%s %s, %u) [%u]",
		// 	GetDayOfWeekStr(GetDayOfWeek(platform->localTime)),
		// 	Convert24HourTo12Hour(platform->localTime.hour), platform->localTime.minute,
		// 	IsPostMeridian(platform->localTime.hour) ? "pm" : "am",
		// 	GetMonthStr((Month_t)platform->localTime.month), GetDayOfMonthString(platform->localTime.day), platform->localTime.year,
		// 	GetTimestamp(platform->localTime));
		// rs->PrintString(
		// 	NewVec2(0, RenderScreenSize.y-app->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 	"First: %d Last: %d", firstLine, lastLine);
		// PrintString(app, app->testFont,
		// 	NewVec2(0, RenderScreenSize.y-app->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 	"%u Lines Offset: %f (%fpx long)", app->lineList.numLines, ui->scrollOffset, ui->fileHeight);
		// Line_t* linePntr = GetLineAt(&app->lineList, ui->hoverLocation.lineNum);
		// if (linePntr != nullptr)
		// {
		// 	Line_t* lineBefore = (Line_t*)linePntr->header.lastItem;
		// 	Line_t* lineAfter = (Line_t*)linePntr->header.nextItem;
		
		// 	rs->PrintString(
		// 		NewVec2(0, RenderScreenSize.y-app->testFont.maxExtendDown), GC->colors.foreground, 1.0f,
		// 		"%08X <- #%u %08X -> %08X", lineBefore, ui->hoverLocation.lineNum, linePntr, lineAfter);
		// }
		
		Line_t* lastLine = GetLastLine(&app->lineList);
		// DEBUG_PrintLine("Last Item: %p", lastLine->header.lastItem);
		if (lastLine->timestamp == 0 && app->lineList.numLines > 1)
		{
			lastLine = GetLineAt(&app->lineList, app->lineList.numLines-1 - 1);
		}
		
		//Print the status message
		if (GetTimestamp(app->statusMessageTime) != 0)
		{
			i64 secondsDifference = SubtractTimes(platform->localTime, app->statusMessageTime, TimeUnit_Seconds);
			if (secondsDifference >= 0 && secondsDifference < GC->statusMessageTime)
			{
				Color_t messageColor = GC->colors.foreground;
				
				if (app->statusMessageType == StatusMessage_Debug)
				{
					messageColor = GC->colors.uiLightGray1;
				}
				else if (app->statusMessageType == StatusMessage_Info)
				{
					messageColor = GC->colors.foreground;
				}
				else if (app->statusMessageType == StatusMessage_Success)
				{
					messageColor = GC->colors.highlight2;
				}
				else if (app->statusMessageType == StatusMessage_Error)
				{
					messageColor = GC->colors.highlight3;
				}
				
				rs->DrawString(app->statusMessage, NewVec2(5, RenderScreenSize.y-app->testFont.maxExtendDown), messageColor, 1.0f);
			}
		}
		
		//Draw Goto End Button
		{
			Color_t buttonColor = GC->colors.uiGray1;
			Color_t outlineColor = GC->colors.background;
			
			if (IsInsideRectangle(RenderMousePos, ui->gotoEndButtonRec))
			{
				if (ButtonDown(MouseButton_Left) &&
					IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, ui->gotoEndButtonRec))
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
		
		if (app->comPort.isOpen && GC->showComNameInStatusBar)
		{
			const char* comPortUserName = GetPortUserName(app->comPort.name);
			v2 comNameSize = MeasureString(&app->testFont, comPortUserName);
			rs->DrawString(comPortUserName,
				NewVec2(ui->gotoEndButtonRec.x - comNameSize.x - 5, RenderScreenSize.y-app->testFont.maxExtendDown),
				GC->colors.foreground, 1.0f);
		}
	}
	
	//+--------------------------------------+
	//|           Render Main Menu           |
	//+--------------------------------------+
	rs->DrawGradient(ui->mainMenuRec, GC->colors.uiGray1, GC->colors.uiGray3, Direction2D_Down);
	rs->DrawRectangle(NewRectangle(0, ui->mainMenuRec.height-1, ui->mainMenuRec.width, 1), GC->colors.uiGray4);
	
	r32 mainMenuButtonsRight = 0;
	for (u32 bIndex = 0; bIndex < NumMainMenuButtons; bIndex++)
	{
		rec buttonRec = ui->buttonRecs[bIndex];
		Color_t baseColor = {Color_White};
		Color_t highlightColor = {Color_Red};
		Color_t iconColor = {Color_White};
		
		if (IsInsideRectangle(RenderMousePos, buttonRec) && !ui->mouseInMenu)
		{
			if (ButtonDown(MouseButton_Left) && IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, buttonRec))
			{
				// iconColor = Color_Highlight2;
				highlightColor = {Color_Black};//Color_Highlight4;
			}
		}
		highlightColor.a = 200;
		
		rs->BindTexture(&ui->buttonBaseTexture);
		rs->DrawTexturedRec(buttonRec, baseColor);
		
		if (IsInsideRectangle(RenderMousePos, buttonRec) && !ui->mouseInMenu)
		{
			rs->BindTexture(&ui->buttonHighlightTexture);
			rs->DrawTexturedRec(buttonRec, highlightColor);
		}
		
		rs->BindTexture(&ui->buttonTextures[bIndex]);
		rs->DrawTexturedRec(buttonRec, iconColor);
		mainMenuButtonsRight = buttonRec.x + buttonRec.width;
	}
	
	// +================================+
	// |         Rx and Tx LEDs         |
	// +================================+
	{
		if (app->rxTxShiftCountdown == 0)
		{
			app->rxTxShiftCountdown = GC->rxTxLedDelay;
			
			app->rxShiftRegister = (u8)(app->rxShiftRegister >> 1);
			app->txShiftRegister = (u8)(app->txShiftRegister >> 1);
		}
		else
		{
			app->rxTxShiftCountdown--;
		}
		
		Color_t centerColor = GC->colors.uiGray4;
		if ((app->rxShiftRegister&0x80) > 0 ||
			(app->rxShiftRegister&0x40) > 0)
		{
			centerColor = GC->colors.receiveLed;
		}
		rs->DrawRectangle(ui->rxLedRec, centerColor);
		centerColor = GC->colors.uiGray4;
		if ((app->txShiftRegister&0x80) > 0 ||
			(app->txShiftRegister&0x40) > 0)
		{
			centerColor = GC->colors.transmitLed;
		}
		rs->DrawRectangle(ui->txLedRec, centerColor);
		for (u32 shift = 0; shift < sizeof(u8)*8; shift++)
		{
			if (IsFlagSet(app->rxShiftRegister, (1<<shift)))
			{
				rec deflatedRec = RectangleInflate(ui->rxLedRec, (r32)(8-shift) * 1);
				rs->DrawButton(deflatedRec, {Color_TransparentBlack}, GC->colors.receiveLed, 1);
			}
			
			if (IsFlagSet(app->txShiftRegister, (1<<shift)))
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
		v2 textSize = MeasureString(&app->testFont, clearStr);
		v2 textPos = NewVec2(
			ui->clearButtonRec.x + ui->clearButtonRec.width/2 - textSize.x/2,
			ui->clearButtonRec.y + ui->clearButtonRec.height/2 + app->testFont.lineHeight/2 - app->testFont.maxExtendDown
		);
		
		Color_t buttonColor, textColor, borderColor;
		ButtonColorChoice(buttonColor, textColor, borderColor, ui->clearButtonRec, false, false);
		
		rs->DrawButton(ui->clearButtonRec, buttonColor, borderColor);
		rs->DrawString(clearStr, textPos, textColor);
	}
	
	// +==============================+
	// |     Python Running Label     |
	// +==============================+
	if (app->programInstance.isOpen)
	{
		const char* runningStr = "Py";
		v2 strSize = MeasureString(&app->testFont, runningStr);
		v2 textPos = NewVec2(
			ui->clearButtonRec.x - strSize.x - 5,
			ui->mainMenuRec.y + app->testFont.maxExtendUp
		);
		
		rs->DrawString(runningStr, textPos, GC->colors.foreground);
	}
	
	// +==================================+
	// |         Generic Counter          |
	// +==================================+
	if (GetRegularExpression(&app->regexList, GC->genericCountRegexName) != nullptr)
	{
		// rs->PrintString(NewVec2(mainMenuButtonsRight + 10, app->testFont.maxExtendUp + 10), GC->colors.foreground, 1.0f,
		// 				"Counter: %u", app->genericCounter);
	}
	
	// +==================================+
	// |       Save To File Button        |
	// +==================================+
	if (app->selectionStart.lineNum != app->selectionEnd.lineNum ||
		app->selectionStart.charIndex != app->selectionEnd.charIndex)
	{
		const char* clearStr = "Save To File";
		v2 textSize = MeasureString(&app->testFont, clearStr);
		v2 textPos = NewVec2(
			ui->saveButtonRec.x + ui->saveButtonRec.width/2 - textSize.x/2,
			ui->saveButtonRec.y + ui->saveButtonRec.height/2 + app->testFont.lineHeight/2 - app->testFont.maxExtendDown
		);
		
		Color_t buttonColor, textColor, borderColor;
		ButtonColorChoice(buttonColor, textColor, borderColor, ui->saveButtonRec, false, false);
		
		rs->DrawButton(ui->saveButtonRec, buttonColor, borderColor);
		rs->DrawString(clearStr, textPos, textColor);
	}
	
	MenuHandlerDrawMenus(&app->renderState, &app->menuHandler);
	
	// +================================+
	// |       Draw Debug Overlay       |
	// +================================+
	#if DEBUG
	if (app->showDebugMenu)
	{
		rec overlayRec = NewRectangle(10, 10, (r32)RenderScreenSize.x - 10*2, (r32)RenderScreenSize.y - 10*2);
		
		rs->DrawButton(overlayRec, ColorTransparent({Color_Black}, 0.5f), ColorTransparent({Color_White}, 0.5f));
		
		v2 textPos = NewVec2(overlayRec.x + 5, overlayRec.y + 5 + app->testFont.maxExtendUp);
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "AppData Size: %u/%u (%.3f%%)",
			sizeof(AppData_t), AppMemory->permanantSize,
			(r32)sizeof(AppData_t) / (r32)AppMemory->permanantSize * 100.0f);
		textPos.y += app->testFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Input Arena: %u/%u (%.3f%%)",
			app->inputArena.used, app->inputArena.size,
			(r32)app->inputArena.used / (r32)app->inputArena.size * 100.0f);
		textPos.y += app->testFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Main Heap: %u/%u (%.3f%%)",
			app->mainHeap.used, app->mainHeap.size,
			(r32)app->mainHeap.used / (r32)app->mainHeap.size * 100.0f);
		textPos.y += app->testFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Temp Init: %u/%u (%.3f%%)",
			app->appInitTempHighWaterMark, TempArena->size,
			(r32)app->appInitTempHighWaterMark / (r32)TempArena->size * 100.0f);
		textPos.y += app->testFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Temp Update: %u/%u (%.3f%%)",
			ArenaGetHighWaterMark(TempArena), TempArena->size,
			(r32)ArenaGetHighWaterMark(TempArena) / (r32)TempArena->size * 100.0f);
		textPos.y += app->testFont.lineHeight;
	}
	#endif
	
	// rs->DrawCircle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, input->mouseMaxDist[MouseButton_Left]/GUI_SCALE, {Color_Red});
	// rs->DrawCircle(input->mouseStartPos[MouseButton_Right]/GUI_SCALE, input->mouseMaxDist[MouseButton_Right]/GUI_SCALE, {Color_Blue});
	// rs->DrawCircle(input->mouseStartPos[MouseButton_Middle]/GUI_SCALE, input->mouseMaxDist[MouseButton_Middle]/GUI_SCALE, {Color_Green});
	
	// rs->DrawRectangle(ui->statusBarRec, {Color_Yellow});
	// rs->DrawRectangle(ui->scrollBarGutterRec, {Color_Red});
	// rs->DrawRectangle(ui->scrollBarRec, {Color_Blue});
	// rs->DrawRectangle(NewRectangle(0, 0, ui->gutterRec.width, screenSize.y - ui->statusBarRec.height), {Color_Orange});
	// rs->DrawRectangle(ui->statusBarRec, {Color_Yellow});
	
	// +==============================+
	// |  Temp Arena Update Loop Pop  |
	// +==============================+
	TempPopMark();
	if (ButtonPressed(Button_T) && ButtonDown(Button_Control)) { ArenaResetHighWaterMark(TempArena); }
}

//+================================================================+
//|                   App Get Sound Samples                        |
//+================================================================+
EXPORT AppGetSoundSamples_DEFINITION(App_GetSoundSamples)
{
	platform = PlatformInfo;
	input = AppInput;
	app = (AppData_t*)AppMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	RenderMousePos = NewVec2(input->mousePos.x / GUI_SCALE, input->mousePos.y / GUI_SCALE);
}

//+================================================================+
//|                        App Closing                             |
//+================================================================+
EXPORT AppClosing_DEFINITION(App_Closing)
{
	platform = PlatformInfo;
	app = (AppData_t*)AppMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	
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