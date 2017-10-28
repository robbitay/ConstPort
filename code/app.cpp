/*
File:   app.cpp
Author: Taylor Robbins
Date:   06\06\2017
Description:
	** Contains all the exported functions and #includes
	** the rest of the source code files.
*/


#define USE_ASSERT_FAILURE_FUNCTION true
#if WINDOWS_COMPILATION
#include "win32_assert.h"
#elif OSX_COMPILATION
#include "osx_assert.h"
#else
#error No supported platform defined. No Assert macro used.
#endif

#include "platformInterface.h"
#include "appVersion.h"
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
v2 RenderMouseStartPos = {};

char* GetElapsedString(u64 timespan)
{
	char* result = nullptr;
	
	u32 numDays = (u32)(timespan/(60*60*24));
	u32 numHours = (u32)(timespan/(60*60)) - (numDays*24);
	u32 numMinutes = (u32)(timespan/60) - (numDays*60*24) - (numHours*60);
	u32 numSeconds = (u32)(timespan) - (numDays*60*60*24) - (numHours*60*60) - (numMinutes*60);
	if (numDays > 0)
	{
		result = TempPrint("%ud %uh %um %us", numDays, numHours, numMinutes, numSeconds);
	}
	else if (numHours > 0)
	{
		result = TempPrint("%uh %um %us", numHours, numMinutes, numSeconds);
	}
	else if (numMinutes > 0)
	{
		result = TempPrint("%um %us", numMinutes, numSeconds);
	}
	else
	{
		result = TempPrint("%us", numSeconds);
	}
	
	return result;
}

#define StatusMessage(functionName, messageColor, duration, formatString, ...) do \
{                                                                                 \
	ClearArray(app->statusMessage);                                               \
	BufferPrint(app->statusMessage, formatString, ##__VA_ARGS__);                 \
	app->statusColor = messageColor;                                              \
	app->statusDuration = duration;                                               \
	app->statusTime = platform->programTime;                                      \
	DEBUG_PrintLine("[%s] Status: %s", functionName, app->statusMessage);         \
} while(0)

#define PopupMessage(functionName, messageColor, duration, formatString, ...) do \
{                                                                                \
	ClearArray(app->popupMessage);                                               \
	BufferPrint(app->popupMessage, formatString, ##__VA_ARGS__);                 \
	app->popupColor = messageColor;                                              \
	app->popupDuration = duration;                                               \
	app->popupTime = platform->programTime;                                      \
	app->popupExcused = false;                                                   \
	DEBUG_PrintLine("[%s] Popup: %s", functionName, app->popupMessage);          \
} while(0)

#define StatusDebug(formatString, ...)   StatusMessage(__func__, GC->colors.debugMessage,   GC->statusDefaultDuration, formatString, ##__VA_ARGS__)
#define StatusInfo(formatString, ...)    StatusMessage(__func__, GC->colors.infoMessage,    GC->statusDefaultDuration, formatString, ##__VA_ARGS__)
#define StatusSuccess(formatString, ...) StatusMessage(__func__, GC->colors.successMessage, GC->statusDefaultDuration, formatString, ##__VA_ARGS__)
#define StatusError(formatString, ...)   StatusMessage(__func__, GC->colors.errorMessage,   GC->statusDefaultDuration, formatString, ##__VA_ARGS__)

#define StatusDebugTimed(duration, formatString, ...)   StatusMessage(__func__, GC->colors.debugMessage,   duration, formatString, ##__VA_ARGS__)
#define StatusInfoTimed(duration, formatString, ...)    StatusMessage(__func__, GC->colors.infoMessage,    duration, formatString, ##__VA_ARGS__)
#define StatusSuccessTimed(duration, formatString, ...) StatusMessage(__func__, GC->colors.successMessage, duration, formatString, ##__VA_ARGS__)
#define StatusErrorTimed(duration, formatString, ...)   StatusMessage(__func__, GC->colors.errorMessage,   duration, formatString, ##__VA_ARGS__)

#define PopupDebug(formatString, ...)   PopupMessage(__func__, GC->colors.debugMessage,   GC->popupDefaultDuration, formatString, ##__VA_ARGS__)
#define PopupInfo(formatString, ...)    PopupMessage(__func__, GC->colors.infoMessage,    GC->popupDefaultDuration, formatString, ##__VA_ARGS__)
#define PopupSuccess(formatString, ...) PopupMessage(__func__, GC->colors.successMessage, GC->popupDefaultDuration, formatString, ##__VA_ARGS__)
#define PopupError(formatString, ...)   PopupMessage(__func__, GC->colors.errorMessage,   GC->popupDefaultDuration, formatString, ##__VA_ARGS__)

#define PopupDebugTimed(duration, formatString, ...)   PopupMessage(__func__, GC->colors.debugMessage,   duration, formatString, ##__VA_ARGS__)
#define PopupInfoTimed(duration, formatString, ...)    PopupMessage(__func__, GC->colors.infoMessage,    duration, formatString, ##__VA_ARGS__)
#define PopupSuccessTimed(duration, formatString, ...) PopupMessage(__func__, GC->colors.successMessage, duration, formatString, ##__VA_ARGS__)
#define PopupErrorTimed(duration, formatString, ...)   PopupMessage(__func__, GC->colors.errorMessage,   duration, formatString, ##__VA_ARGS__)

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

void ComMenuUpdate(MenuHandler_t* menuHandler, Menu_t* menuPntr)
{
	if (menuPntr->show)
	{
		if (ButtonPressedUnhandled(Button_Escape))
		{
			HandleButton(Button_Escape);
			menuPntr->show = false;
		}
		
		u32 numTabs = app->availablePorts.count + ((app->comPort.isOpen && !IsComAvailable(app->comPort.name)) ? 1 : 0);
		r32 tabWidth = menuPntr->usableRec.width / numTabs;
		r32 tabMinimumWidth = MeasureString(&app->uiFont, "1234567890").x + COM_MENU_TAB_PADDING*2;
		if (tabWidth < tabMinimumWidth) { tabWidth = tabMinimumWidth; }
		rec baudRateRec = NewRectangle(
			menuPntr->usableRec.x + COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + app->uiFont.lineHeight + COM_MENU_OUTER_PADDING,
			78, app->uiFont.lineHeight * NumBaudRates
		);
		rec numBitsRec = NewRectangle(
			baudRateRec.x + baudRateRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + app->uiFont.lineHeight + COM_MENU_OUTER_PADDING,
			55, app->uiFont.lineHeight * 8
		);
		rec parityTypesRec = NewRectangle(
			numBitsRec.x + numBitsRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + app->uiFont.lineHeight + COM_MENU_OUTER_PADDING,
			60, app->uiFont.lineHeight * NumParityTypes
		);
		rec stopBitsRec = NewRectangle(
			parityTypesRec.x + parityTypesRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + app->uiFont.lineHeight + COM_MENU_OUTER_PADDING,
			80, app->uiFont.lineHeight * NumStopBitTypes
		);
		
		// +==============================+
		// |   Calculate the Menu Width   |
		// +==============================+
		r32 menuWidth = (stopBitsRec.x + stopBitsRec.width + COM_MENU_OUTER_PADDING) - menuPntr->drawRec.x;
		if (menuWidth < tabMinimumWidth * numTabs)
		{
			menuWidth = tabMinimumWidth * numTabs;
		}
		
		// +==============================+
		// |   Calculate The Tab Height   |
		// +==============================+
		r32 tabHeight = COM_MENU_TAB_HEIGHT;
		for (u32 tabIndex = 0; tabIndex < numTabs; tabIndex++)
		{
			char* portName = nullptr;
			if (tabIndex < app->availablePorts.count) { portName = app->availablePorts[tabIndex]; }
			else { portName = app->comPort.name; }
			
			const char* portUserName = GetPortUserName(portName);
			
			v2 givenNameSize = MeasureFormattedString(&app->uiFont, portUserName, tabWidth - COM_MENU_TAB_PADDING*2, true);
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
		
		const char* connectStr = "Connect";
		v2 connectStrSize = MeasureString(&app->uiFont, connectStr);
		rec connectButtonRec = NewRectangle(
			menuPntr->usableRec.x + menuPntr->usableRec.width - (connectStrSize.x + 10) - COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + menuPntr->usableRec.height - (connectStrSize.y + 10) - COM_MENU_OUTER_PADDING,
			(connectStrSize.x + 10), (connectStrSize.y + 10)
		);
		const char* disconnectStr = "Disconnect";
		v2 disconnectStrSize = MeasureString(&app->uiFont, disconnectStr);
		rec disconnectButtonRec = NewRectangle(
			connectButtonRec.x - (disconnectStrSize.x + 10) - COM_MENU_OUTER_PADDING,
			connectButtonRec.y,
			(disconnectStrSize.x + 10), (connectStrSize.y + 10)
		);
		
		for (i32 baudIndex = 0; baudIndex < NumBaudRates; baudIndex++)
		{
			rec currentRec = NewRectangle(baudRateRec.x,
				baudRateRec.y + baudIndex*app->uiFont.lineHeight,
				baudRateRec.width, app->uiFont.lineHeight
			);
			if (IsInsideRectangle(RenderMousePos, currentRec) && IsInsideRectangle(RenderMouseStartPos, currentRec) &&
				ButtonReleased(MouseButton_Left))
			{
				app->comMenuOptions.settings.baudRate = (BaudRate_t)baudIndex;
			}
		}
		
		for (i32 bitIndex = 0; bitIndex < 8; bitIndex++)
		{
			rec currentRec = NewRectangle(numBitsRec.x,
				numBitsRec.y + bitIndex*app->uiFont.lineHeight,
				numBitsRec.width, app->uiFont.lineHeight
			);
			if (IsInsideRectangle(RenderMousePos, currentRec) && IsInsideRectangle(RenderMouseStartPos, currentRec) &&
				ButtonReleased(MouseButton_Left))
			{
				app->comMenuOptions.settings.numBits = (u8)(bitIndex+1);
			}
		}
		
		for (i32 parityIndex = 0; parityIndex < NumParityTypes; parityIndex++)
		{
			rec currentRec = NewRectangle(parityTypesRec.x,
				parityTypesRec.y + parityIndex*app->uiFont.lineHeight,
				parityTypesRec.width, app->uiFont.lineHeight
			);
			if (IsInsideRectangle(RenderMousePos, currentRec) && IsInsideRectangle(RenderMouseStartPos, currentRec) &&
				ButtonReleased(MouseButton_Left))
			{
				app->comMenuOptions.settings.parity = (Parity_t)parityIndex;
			}
		}
		
		for (i32 stopBitIndex = 0; stopBitIndex < NumStopBitTypes; stopBitIndex++)
		{
			rec currentRec = NewRectangle(stopBitsRec.x,
				stopBitsRec.y + stopBitIndex*app->uiFont.lineHeight,
				stopBitsRec.width, app->uiFont.lineHeight
			);
			if (IsInsideRectangle(RenderMousePos, currentRec) && IsInsideRectangle(RenderMouseStartPos, currentRec) &&
				ButtonReleased(MouseButton_Left))
			{
				app->comMenuOptions.settings.stopBits = (StopBits_t)stopBitIndex;
			}
		}
		
		// +==============================+
		// |    Check for tab Presses     |
		// +==============================+
		for (u32 tabIndex = 0; tabIndex < numTabs; tabIndex++)
		{
			char* tabName = nullptr;
			if (tabIndex < app->availablePorts.count) { tabName = app->availablePorts[tabIndex]; }
			else { tabName = app->comPort.name; }
			
			rec tabRec = NewRectangle(tabIndex * tabWidth, 0, tabWidth, tabHeight);
			tabRec.topLeft += menuPntr->usableRec.topLeft;
			
			if (ButtonReleased(MouseButton_Left) && IsInsideRectangle(RenderMousePos, tabRec) &&
				IsInsideRectangle(RenderMouseStartPos, tabRec))
			{
				if (app->comMenuOptions.name != nullptr) { ArenaPop(&app->mainHeap, app->comMenuOptions.name); }
				app->comMenuOptions.name = DupStr(tabName, &app->mainHeap);
				app->comMenuOptions.isOpen = true;
			}
		}
		
		// +================================+
		// | Check for Connect Button Press |
		// +================================+
		bool connectButtonPressed = (IsInsideRectangle(RenderMousePos, connectButtonRec) &&
			ButtonReleased(MouseButton_Left) && IsInsideRectangle(RenderMouseStartPos, connectButtonRec));
		if (app->comMenuOptions.isOpen &&
			(connectButtonPressed || ButtonReleased(Button_Enter)))
		{
			if (OpenComPort(app->comMenuOptions.name, app->comMenuOptions.settings))
			{
				menuPntr->show = false;
			}
		}
		
		// +====================================+
		// | Check for Disconnect Button Press  |
		// +====================================+
		if (app->comPort.isOpen)
		{
			bool currentPortIsSelected = true;
			if (app->comMenuOptions.name == nullptr && app->comPort.name != nullptr) { currentPortIsSelected = false; }
			else if (app->comMenuOptions.name != nullptr && app->comPort.name == nullptr) { currentPortIsSelected = false; }
			else if (app->comMenuOptions.name != nullptr && app->comPort.name != nullptr)
			{
				if (strcmp(app->comMenuOptions.name, app->comPort.name) != 0) { currentPortIsSelected = false; }
			}
			
			if (currentPortIsSelected)
			{
				if (app->comMenuOptions.isOpen && IsInsideRectangle(RenderMousePos, disconnectButtonRec) &&
					ButtonReleased(MouseButton_Left) && IsInsideRectangle(RenderMouseStartPos, disconnectButtonRec))
				{
					PopupError("Closed \"%s\"", app->comPort.name);
					platform->CloseComPort(&app->mainHeap, &app->comPort);
					menuPntr->show = false;
					ClearConsole();
				}
			}
		}
	}
}
void ComMenuRender(RenderState_t* renderState, MenuHandler_t* menuHandler, Menu_t* menuPntr)
{
	if (menuPntr->show)
	{
		u32 numTabs = app->availablePorts.count + ((app->comPort.isOpen && !IsComAvailable(app->comPort.name)) ? 1 : 0);
		r32 tabWidth = menuPntr->usableRec.width / numTabs;
		rec baudRateRec = NewRectangle(
			menuPntr->usableRec.x + COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + app->uiFont.lineHeight + COM_MENU_OUTER_PADDING,
			78, app->uiFont.lineHeight * NumBaudRates
		);
		rec numBitsRec = NewRectangle(
			baudRateRec.x + baudRateRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + app->uiFont.lineHeight + COM_MENU_OUTER_PADDING,
			55, app->uiFont.lineHeight * 8
		);
		rec parityTypesRec = NewRectangle(
			numBitsRec.x + numBitsRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + app->uiFont.lineHeight + COM_MENU_OUTER_PADDING,
			60, app->uiFont.lineHeight * NumParityTypes
		);
		rec stopBitsRec = NewRectangle(
			parityTypesRec.x + parityTypesRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + app->uiFont.lineHeight + COM_MENU_OUTER_PADDING,
			80, app->uiFont.lineHeight * NumStopBitTypes
		);
		
		// +==============================+
		// |   Calculate The Tab Height   |
		// +==============================+
		r32 tabHeight = COM_MENU_TAB_HEIGHT;
		for (u32 tabIndex = 0; tabIndex < numTabs; tabIndex++)
		{
			char* portName = nullptr;
			if (tabIndex < app->availablePorts.count) { portName = app->availablePorts[tabIndex]; }
			else { portName = app->comPort.name; }
			
			const char* portUserName = GetPortUserName(portName);
			
			v2 givenNameSize = MeasureFormattedString(&app->uiFont, portUserName, tabWidth - COM_MENU_TAB_PADDING*2, true);
			if (givenNameSize.y + COM_MENU_TAB_PADDING*2 > tabHeight)
			{
				tabHeight = givenNameSize.y + COM_MENU_TAB_PADDING*2;
			}
		}
		baudRateRec.y    += tabHeight;
		numBitsRec.y     += tabHeight;
		parityTypesRec.y += tabHeight;
		stopBitsRec.y    += tabHeight;
		
		const char* connectStr = "Connect";
		v2 connectStrSize = MeasureString(&app->uiFont, connectStr);
		rec connectButtonRec = NewRectangle(
			menuPntr->usableRec.x + menuPntr->usableRec.width - (connectStrSize.x + 10) - COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + menuPntr->usableRec.height - (connectStrSize.y + 10) - COM_MENU_OUTER_PADDING,
			(connectStrSize.x + 10), (connectStrSize.y + 10)
		);
		const char* disconnectStr = "Disconnect";
		v2 disconnectStrSize = MeasureString(&app->uiFont, disconnectStr);
		rec disconnectButtonRec = NewRectangle(
			connectButtonRec.x - (disconnectStrSize.x + 10) - COM_MENU_OUTER_PADDING,
			connectButtonRec.y,
			(disconnectStrSize.x + 10), (connectStrSize.y + 10)
		);
		
		renderState->BindFont(&app->uiFont);
		// +==============================+
		// |  Draw the Baud Rate Options  |
		// +==============================+
		renderState->DrawString("Baud Rate", NewVec2(baudRateRec.x, baudRateRec.y - app->uiFont.maxExtendDown), GC->colors.uiText);
		renderState->DrawRectangle(baudRateRec, GC->colors.button);
		for (i32 baudIndex = 0; baudIndex < NumBaudRates; baudIndex++)
		{
			const char* baudString = GetBaudRateString((BaudRate_t)baudIndex);
			rec currentRec = NewRectangle(baudRateRec.x,
				baudRateRec.y + baudIndex*app->uiFont.lineHeight,
				baudRateRec.width, app->uiFont.lineHeight
			);
			v2 textPos = baudRateRec.topLeft + NewVec2(
				baudRateRec.width/2 - MeasureString(&app->uiFont, baudString).x/2,
				app->uiFont.maxExtendUp + baudIndex*app->uiFont.lineHeight);
			
			Color_t backColor   = GC->colors.button;
			Color_t textColor   = GC->colors.buttonText;
			Color_t borderColor = GC->colors.buttonBorder;
			ButtonColorChoice(backColor, textColor, borderColor, currentRec,
				((BaudRate_t)baudIndex == app->comMenuOptions.settings.baudRate), false);
			
			renderState->DrawRectangle(currentRec, backColor);
			renderState->DrawString(baudString, textPos, textColor);
		}
		
		// +==============================+
		// |   Draw the # Bits Options    |
		// +==============================+
		renderState->DrawString("# Bits", NewVec2(numBitsRec.x, numBitsRec.y - app->uiFont.maxExtendDown), GC->colors.uiText);
		renderState->DrawRectangle(numBitsRec, GC->colors.button);
		for (i32 bitIndex = 0; bitIndex < 8; bitIndex++)
		{
			char numBitsString[4] = {};
			snprintf(numBitsString, ArrayCount(numBitsString)-1, "%d", bitIndex+1);
			rec currentRec = NewRectangle(numBitsRec.x,
				numBitsRec.y + bitIndex*app->uiFont.lineHeight,
				numBitsRec.width, app->uiFont.lineHeight
			);
			v2 textPos = numBitsRec.topLeft + NewVec2(
				numBitsRec.width/2 - MeasureString(&app->uiFont, numBitsString).x/2,
				app->uiFont.maxExtendUp + bitIndex*app->uiFont.lineHeight);
			
			Color_t backColor   = GC->colors.button;
			Color_t textColor   = GC->colors.buttonText;
			Color_t borderColor = GC->colors.buttonBorder;
			ButtonColorChoice(backColor, textColor, borderColor, currentRec,
				((u8)(bitIndex+1) == app->comMenuOptions.settings.numBits), false);
			
			renderState->DrawRectangle(currentRec, backColor);
			renderState->DrawString(numBitsString, textPos, textColor);
		}
		
		// +==============================+
		// |   Draw the Parity Options    |
		// +==============================+
		renderState->DrawString("Parity", NewVec2(parityTypesRec.x, parityTypesRec.y - app->uiFont.maxExtendDown), GC->colors.uiText);
		renderState->DrawRectangle(parityTypesRec, GC->colors.button);
		for (i32 parityIndex = 0; parityIndex < NumParityTypes; parityIndex++)
		{
			const char* parityString = GetParityString((Parity_t)parityIndex);
			rec currentRec = NewRectangle(parityTypesRec.x,
				parityTypesRec.y + parityIndex*app->uiFont.lineHeight,
				parityTypesRec.width, app->uiFont.lineHeight
			);
			v2 textPos = parityTypesRec.topLeft + NewVec2(
				parityTypesRec.width/2 - MeasureString(&app->uiFont, parityString).x/2,
				app->uiFont.maxExtendUp + parityIndex*app->uiFont.lineHeight);
			
			Color_t backColor   = GC->colors.button;
			Color_t textColor   = GC->colors.buttonText;
			Color_t borderColor = GC->colors.buttonBorder;
			ButtonColorChoice(backColor, textColor, borderColor, currentRec,
				((Parity_t)parityIndex == app->comMenuOptions.settings.parity), false);
			
			renderState->DrawRectangle(currentRec, backColor);
			renderState->DrawString(parityString, textPos, textColor);
		}
		
		// +==============================+
		// |  Draw the Stop Bits Options  |
		// +==============================+
		renderState->DrawString("Stop Bits", NewVec2(stopBitsRec.x, stopBitsRec.y - app->uiFont.maxExtendDown), GC->colors.uiText);
		renderState->DrawRectangle(stopBitsRec, GC->colors.button);
		for (i32 stopBitIndex = 0; stopBitIndex < NumStopBitTypes; stopBitIndex++)
		{
			const char* stopBitsString = GetStopBitsString((StopBits_t)stopBitIndex);
			rec currentRec = NewRectangle(stopBitsRec.x,
				stopBitsRec.y + stopBitIndex*app->uiFont.lineHeight,
				stopBitsRec.width, app->uiFont.lineHeight
			);
			v2 textPos = stopBitsRec.topLeft + NewVec2(
				stopBitsRec.width/2 - MeasureString(&app->uiFont, stopBitsString).x/2,
				app->uiFont.maxExtendUp + stopBitIndex*app->uiFont.lineHeight);
			
			Color_t buttonColor = GC->colors.button;
			Color_t textColor   = GC->colors.buttonText;
			Color_t borderColor = GC->colors.buttonBorder;
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
			v2 stringSize = MeasureFormattedString(&app->uiFont, portUserName, tabWidth - COM_MENU_TAB_PADDING*2, true);
			v2 stringPosition = tabRec.topLeft + NewVec2(tabRec.width/2, tabRec.height/2 - stringSize.y/2 + app->uiFont.maxExtendUp);
			
			Color_t buttonColor = GC->colors.button;
			Color_t textColor   = GC->colors.buttonText;
			Color_t borderColor = GC->colors.buttonBorder;
			ButtonColorChoice(buttonColor, textColor, borderColor, tabRec,
				app->comPort.isOpen && strcmp(app->comPort.name, portName) == 0,
				(app->comMenuOptions.isOpen == true && strcmp(app->comMenuOptions.name, portName) == 0));
			
			renderState->DrawButton(tabRec, buttonColor, borderColor);
			// renderState->DrawRectangle(NewRectangle(stringPosition.x - stringSize.x/2, stringPosition.y - app->uiFont.maxExtendUp, stringSize.x, stringSize.y), {Color_Red});
			renderState->DrawFormattedString(portUserName, stringPosition, tabWidth - COM_MENU_TAB_PADDING*2, textColor, Alignment_Center, true);
		}
		
		// +==============================+
		// |   Draw the Connect Button    |
		// +==============================+
		{
			v2 stringPosition = NewVec2(
				connectButtonRec.x + connectButtonRec.width/2 - connectStrSize.x/2,
				connectButtonRec.y + connectButtonRec.height/2 + app->uiFont.lineHeight/2 - app->uiFont.maxExtendDown
			);
			bool settingsHaveChanged = ((!app->comPort.isOpen && app->comMenuOptions.isOpen) || app->comMenuOptions.settings != app->comPort.settings);
			if (app->comMenuOptions.name == nullptr && app->comPort.name != nullptr) { settingsHaveChanged = true; }
			else if (app->comMenuOptions.name != nullptr && app->comPort.name == nullptr) { settingsHaveChanged = true; }
			else if (app->comMenuOptions.name != nullptr && app->comPort.name != nullptr)
			{
				if (strcmp(app->comMenuOptions.name, app->comPort.name) != 0) { settingsHaveChanged = true; }
			}
			
			Color_t buttonColor = GC->colors.button;
			Color_t textColor   = GC->colors.buttonText;
			Color_t borderColor = GC->colors.buttonBorder;
			if (!app->comMenuOptions.isOpen)
			{
				buttonColor = GC->colors.buttonDisabled;
				textColor   = GC->colors.buttonDisabledText;
				borderColor = GC->colors.buttonDisabledBorder;
			}
			else
			{
				ButtonColorChoice(buttonColor, textColor, borderColor, connectButtonRec, false, settingsHaveChanged);
			}
			
			renderState->DrawButton(connectButtonRec, buttonColor, borderColor);
			renderState->DrawString(connectStr, stringPosition, textColor);
		}
		
		// +==============================+
		// |  Draw the Disconnect Button  |
		// +==============================+
		if (app->comPort.isOpen)
		{
			v2 stringPosition = NewVec2(
				disconnectButtonRec.x + disconnectButtonRec.width/2 - disconnectStrSize.x/2,
				disconnectButtonRec.y + disconnectButtonRec.height/2 + app->uiFont.lineHeight/2 - app->uiFont.maxExtendDown
			);
			bool currentPortIsSelected = true;
			if (app->comMenuOptions.name == nullptr && app->comPort.name != nullptr) { currentPortIsSelected = false; }
			else if (app->comMenuOptions.name != nullptr && app->comPort.name == nullptr) { currentPortIsSelected = false; }
			else if (app->comMenuOptions.name != nullptr && app->comPort.name != nullptr)
			{
				if (strcmp(app->comMenuOptions.name, app->comPort.name) != 0) { currentPortIsSelected = false; }
			}
			
			if (currentPortIsSelected)
			{
				Color_t buttonColor = GC->colors.button;
				Color_t textColor   = GC->colors.buttonText;
				Color_t borderColor = GC->colors.buttonBorder;
				ButtonColorChoice(buttonColor, textColor, borderColor, disconnectButtonRec, true, false);
				
				renderState->DrawButton(disconnectButtonRec, buttonColor, borderColor);
				renderState->DrawString(disconnectStr, stringPosition, textColor);
			}
		}
		
		renderState->BindFont(&app->mainFont);
	}
}

void ContextMenuUpdate(MenuHandler_t* menuHandler, Menu_t* menu)
{
	UiElements_t* ui = &app->uiElements;
	
	if (ui->contextString != nullptr)
	{
		v2 textSize = MeasureString(&app->uiFont, ui->contextString);
		menu->drawRec.size = textSize;
		menu->drawRec = RectangleInflate(menu->drawRec, CONTEXT_MENU_PADDING);
		menu->drawRec.topLeft = RenderMousePos + NewVec2(0, -3 - menu->drawRec.height);
	}
}
void ContextMenuRender(RenderState_t* renderState, MenuHandler_t* menuHandler, Menu_t* menu)
{
	UiElements_t* ui = &app->uiElements;
	
	if (ui->contextString != nullptr)
	{
		v2 textPos = menu->usableRec.topLeft + NewVec2(CONTEXT_MENU_PADDING, CONTEXT_MENU_PADDING + app->uiFont.maxExtendUp);
		app->renderState.BindFont(&app->uiFont);
		app->renderState.DrawString(ui->contextString, textPos, GC->colors.contextMenuText);
		app->renderState.BindFont(&app->mainFont);
	}
}

const char testMessage[] = "Hello world!\n"
	"This message is intended to be used to test the capabilities of the application without having a COM port to connect to\n"
	"It will be a really long message with not a lot of\n"
	"actual\n"
	"content\n"
	"One thing that this program can do is process a lot of data at once\n"
	"without missing or dropping any bytes\n"
	"This is important so that the user can be confident that what they see is what was actually sent\n"
	"We need to add a few more lines to make this message even taller\n"
	"We will\n"
	"do that\n"
	"by typing\n"
	"two words\n"
	"at a\n"
	"time into\n"
	"each line\n"
	"in this\n"
	"lame sentence.\n"
	"AAAAAAAA";

#define REAL_LOGO_HEIGHT 75
#define ABOUT_INFO_TEXT_PADDING 20
#define ABOUT_INFO_FORMAT_STRING "Const Port is an application written by me, Taylor Robbins. "                      \
	"It's primarily been developed for my own purposes at work while developing embedded firmware applications.\n\n" \
	"I release it under an open source license in the hope that others might also find it useful.\n\n"               \
	"If you find any bugs or have any suggestions you can email me at robbitay@gmail.com.\n\n"                       \
	"Latest releases can be found on the development blog:\nhttps://www.SilTutorials.com/blog\n\n"                   \
	"More info on the HMN project page:\nhttps://ConstPort.Handmade.Network/\n\n"                                    \
	"Application Version %u.%u (build %u)\nPlatform Version %u.%u (build %u)\n"

void AboutMenuUpdate(MenuHandler_t* menuHandler, Menu_t* menu)
{
	menu->drawRec.width = 350;
	if (menu->drawRec.width > RenderScreenSize.x)
	{
		menu->drawRec.width = RenderScreenSize.x;
	}
	
	UpdateMenuRecs(menu);
	
	const char* aboutInfoString = TempPrint(ABOUT_INFO_FORMAT_STRING,
		APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_BUILD,
		platform->version.major, platform->version.minor, platform->version.build);
	v2 textSize = MeasureFormattedString(&app->uiFont, aboutInfoString, menu->usableRec.width - ABOUT_INFO_TEXT_PADDING*2, true);
	
	r32 mustContainY = (menu->usableRec.y + REAL_LOGO_HEIGHT + ABOUT_INFO_TEXT_PADDING*2 + textSize.y) - menu->drawRec.y;
	
	menu->drawRec.height = mustContainY;
}
void AboutMenuRender(RenderState_t* renderState, MenuHandler_t* menuHandler, Menu_t* menu)
{
	const char* aboutInfoString = TempPrint(ABOUT_INFO_FORMAT_STRING,
		APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_BUILD,
		platform->version.major, platform->version.minor, platform->version.build);
	r32 logoScale = MinReal32(
		(r32)menu->usableRec.width / app->crappyLogo.width,
		(r32)menu->usableRec.height / REAL_LOGO_HEIGHT);
	rec logoRec = NewRectangle(
		menu->usableRec.x, menu->usableRec.y,
		app->crappyLogo.width * logoScale,
		REAL_LOGO_HEIGHT * logoScale
	);
	renderState->BindTexture(&app->crappyLogo);
	renderState->DrawTexturedRec(logoRec, {Color_White}, NewRectangle(0, 0, (r32)app->crappyLogo.width, REAL_LOGO_HEIGHT));
	
	
	renderState->BindFont(&app->uiFont);
	renderState->DrawFormattedString(aboutInfoString,
		NewVec2(menu->usableRec.width/2, REAL_LOGO_HEIGHT + ABOUT_INFO_TEXT_PADDING + app->uiFont.maxExtendUp) + menu->usableRec.topLeft,
		menu->usableRec.width - ABOUT_INFO_TEXT_PADDING*2,
		GC->colors.uiText, Alignment_Center, true
	);
	renderState->BindFont(&app->mainFont);
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

char* SanatizeStringAdvanced(const char* strPntr, u32 numChars, MemoryArena_t* arenaPntr,
	bool keepNewLines = true, bool keepBackspaces = false, bool convertInvalidToHex = false)
{
	char* result = nullptr;
	u32 resultLength = 0;
	
	for (u8 round = 0; round < 2; round++)
	{
		Assert(round == 0 || result != nullptr);
		
		//Run this code twice, once to measure and once to actually copy into buffer
		u32 fromIndex = 0;
		u32 toIndex = 0;
		while (fromIndex < numChars)
		{
			char c = strPntr[fromIndex];
			char lastChar = '\0';
			if (fromIndex > 0) { lastChar = strPntr[fromIndex-1]; }
			char nextChar = strPntr[fromIndex+1];
			
			bool charHandled = false;
			if (c == '\n')
			{
				if (keepNewLines)
				{
					if (result != nullptr) { result[toIndex] = '\n'; }
					toIndex++;
					charHandled = true;
				}
			}
			else if (c == '\r')
			{
				if (nextChar == '\n' || lastChar == '\n')
				{
					//Drop the \r character
					charHandled = true;
				}
				else if (keepNewLines)
				{
					if (result != nullptr) { result[toIndex] = '\n'; }
					toIndex++;
					charHandled = true;
				}
			}
			else if (c == '\b')
			{
				if (keepBackspaces)
				{
					if (result != nullptr) { result[toIndex] = '\b'; }
					toIndex++;
					charHandled = true;
				}
			}
			else if (IsCharClassPrintable(c) || c == '\t')
			{
				if (result != nullptr) { result[toIndex] = c; }
				toIndex++;
				charHandled = true;
			}
			
			if (charHandled == false && convertInvalidToHex)
			{
				if (result != nullptr)
				{
					result[toIndex+0] = '0';
					result[toIndex+1] = 'x';
					result[toIndex+2] = UpperHexChar(c);
					result[toIndex+3] = LowerHexChar(c);
				}
				toIndex += 4;
			}
			
			fromIndex++;
		}
		
		resultLength = toIndex;
		if (resultLength == 0) { break; }
		if (round == 0)
		{
			result = (char*)ArenaPush_(arenaPntr, resultLength+1);
		}
		result[resultLength] = '\0';
	}
	
	return result;
}

struct TriggerResults_t
{
	bool addLineToBuffer;
	bool createNewLine;
	bool clearScreen;
};

TriggerResults_t ApplyTriggerEffects(Line_t* newLine, RegexTrigger_t* trigger)
{
	TriggerResults_t result = {};
	result.addLineToBuffer = true;
	
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
		else if (strcmp(effectStr, "mark_above") == 0)
		{
			if (newLine->previous != nullptr)
			{
				FlagSet(newLine->previous->flags, LineFlag_MarkBelow);
			}
		}
		else if (strcmp(effectStr, "thick_mark_above") == 0)
		{
			if (newLine->previous != nullptr)
			{
				FlagSet(newLine->previous->flags, LineFlag_MarkBelow | LineFlag_ThickMark);
			}
		}
		else if (strcmp(effectStr, "clear_screen") == 0)
		{
			result.clearScreen = true;
		}
		else if (strcmp(effectStr, "new_line") == 0)
		{
			result.createNewLine = true;
		}
		else if (strcmp(effectStr, "clear_line") == 0)
		{
			result.addLineToBuffer = false;
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
				else if (strcmp(nameStr, "status_debug") == 0)
				{
					StatusDebug(valueStr);
				}
				else if (strcmp(nameStr, "status_info") == 0)
				{
					StatusInfo(valueStr);
				}
				else if (strcmp(nameStr, "status_success") == 0)
				{
					StatusSuccess(valueStr);
				}
				else if (strcmp(nameStr, "status_error") == 0)
				{
					StatusError(valueStr);
				}
				else if (strcmp(nameStr, "popup") == 0)
				{
					PopupInfo(valueStr);
				}
				else if (strcmp(nameStr, "popup_debug") == 0)
				{
					PopupDebug(valueStr);
				}
				else if (strcmp(nameStr, "popup_info") == 0)
				{
					PopupInfo(valueStr);
				}
				else if (strcmp(nameStr, "popup_success") == 0)
				{
					PopupSuccess(valueStr);
				}
				else if (strcmp(nameStr, "popup_error") == 0)
				{
					PopupError(valueStr);
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
	
	return result;
}

void ReplaceLineWithCapture(Line_t* linePntr, const char* regexStr)
{
	TempPushMark();
	
	char* tempString = GetRegexCaptureString(regexStr, linePntr->chars, linePntr->numChars, TempArena);
	DEBUG_PrintLine("Captured replacement: \"%s\"", tempString);
	LineListReplaceLine(&app->lineList, tempString, (u32)strlen(tempString));
	
	TempPopMark();
}

void ReplaceLineWithCaptureFormatString(Line_t* linePntr, const char* regexStr, const char* formatStr)
{
	TempPushMark();
	
	char* tempString = GetRegexCaptureFormatString(regexStr, linePntr->chars, linePntr->numChars, formatStr, TempArena);
	DEBUG_PrintLine("Captured formatted replacement: \"%s\"", tempString);
	LineListReplaceLine(&app->lineList, tempString, (u32)strlen(tempString));
	
	TempPopMark();
}

#define CheckRegularExpression(perCharacter, addLineToBuffer, createNewLine, clearScreen, linePntr) do             \
{                                                                                                                  \
	for (u32 rIndex = 0; rIndex < GC->numTriggers; rIndex++)                                                       \
	{                                                                                                              \
		RegexTrigger_t* trigger = &GC->triggers[rIndex];                                                           \
		if (perCharacter == false || trigger->runPerCharacter)                                                     \
		{                                                                                                          \
			bool appliedToThisComPort = (trigger->numComPorts == 0 || app->comPort.isOpen == false);               \
			if (appliedToThisComPort == false)                                                                     \
			{                                                                                                      \
				for (u32 comListIndex = 0; comListIndex < trigger->numComPorts; comListIndex++)                    \
				{                                                                                                  \
					const char* supportedName = trigger->comPorts[comListIndex];                                   \
					if (strcmp(supportedName, app->comPort.name) == 0 ||                                           \
						strcmp(supportedName, GetPortUserName(app->comPort.name)) == 0)                            \
					{                                                                                              \
						appliedToThisComPort = true;                                                               \
						break;                                                                                     \
					}                                                                                              \
				}                                                                                                  \
			}                                                                                                      \
			if (appliedToThisComPort)                                                                              \
			{                                                                                                      \
				const char* regexStr = nullptr;                                                                    \
				if (trigger->expression != nullptr)                                                                \
				{                                                                                                  \
					regexStr = trigger->expression;                                                                \
				}                                                                                                  \
				else if (trigger->expressionName != nullptr)                                                       \
				{                                                                                                  \
					regexStr = GetRegularExpression(&app->regexList, trigger->expressionName);                     \
				}                                                                                                  \
				if (regexStr != nullptr)                                                                           \
				{                                                                                                  \
					bool expressionMatched = TestRegularExpression(regexStr, linePntr->chars, linePntr->numChars); \
					if (expressionMatched)                                                                         \
					{                                                                                              \
						if (trigger->showOnlyCaptured)                                                             \
						{                                                                                          \
							ReplaceLineWithCapture(linePntr, regexStr);                                            \
						}                                                                                          \
						if (trigger->replaceStr != nullptr)                                                        \
						{                                                                                          \
							ReplaceLineWithCaptureFormatString(linePntr, regexStr, trigger->replaceStr);           \
						}                                                                                          \
						TriggerResults_t results = ApplyTriggerEffects(linePntr, trigger);                         \
						if (!results.addLineToBuffer) { createNewLine = true; }                                    \
						if (results.createNewLine) { createNewLine = true; }                                       \
						if (results.clearScreen) { clearScreen = true; }                                           \
					}                                                                                              \
				}                                                                                                  \
			}                                                                                                      \
		}                                                                                                          \
	}                                                                                                              \
} while(0)

void DropCharData(u32 targetSize)
{
	StatusInfo("Resizing %u bytes down to %u bytes", app->lineList.charDataSize, targetSize);
	u32 numLinesRemoved = 0;
	r32 heightRemoved = LineListDownsize(&app->lineList, targetSize, &numLinesRemoved);
	app->uiElements.scrollOffset.y = max(0, app->uiElements.scrollOffset.y - heightRemoved);
	app->uiElements.scrollOffsetGoto.y = max(0, app->uiElements.scrollOffsetGoto.y - heightRemoved);
	app->selectionStart.lineIndex -= numLinesRemoved;
	if (app->selectionStart.lineIndex < 0)
	{
		app->selectionStart.lineIndex = 0;
		app->selectionStart.charIndex = 0;
	}
	app->selectionEnd.lineIndex -= numLinesRemoved;
	if (app->selectionEnd.lineIndex < 0)
	{
		app->selectionEnd.lineIndex = 0;
		app->selectionEnd.charIndex = 0;
	}
	
	PopupError("Dropped %u line(s) due to space requirements", numLinesRemoved);
}

void DataReceived(const char* dataBuffer, i32 numBytes)
{
	Line_t* lastLine = app->lineList.lastLine;
	
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
				
				platform->AppendFile(&app->outputFile, timestampBuffer, timestampLength);
				
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
				platform->AppendFile(&app->outputFile, finishedLine->chars, finishedLine->numChars);
				platform->AppendFile(&app->outputFile, "\r\n", 2);
				
				LineListClearLine(&app->lineList);
			}
			else
			{
				//Throw the line on the end of the list
				
				// +====================================+
				// | Check Line Against Regex Triggers  |
				// +====================================+
				bool addLineToBuffer = true;
				bool createNewLine = false;
				bool clearScreen = false;
				CheckRegularExpression(false, addLineToBuffer, createNewLine, clearScreen, lastLine);
				
				if (clearScreen)
				{
					ClearConsole();
					lastLine = app->lineList.lastLine;
				}
				else
				{
					if (addLineToBuffer)
					{
						lastLine = LineListPushLine(&app->lineList);
					}
					else
					{
						LineListClearLine(&app->lineList);
					}
				}
			}
		}
		else if (newChar == '\b')
		{
			bool charRemoved = LineListPopCharacter(&app->lineList);
		}
		else
		{
			LineListAppendData(&app->lineList, &newChar, 1);
			
			// +====================================+
			// | Check Per Character Regex Triggers |
			// +====================================+
			bool addLineToBuffer = true;
			bool createNewLine = false;
			bool clearScreen = false;
			CheckRegularExpression(true, addLineToBuffer, createNewLine, clearScreen, lastLine);
			
			if (createNewLine)
			{
				CheckRegularExpression(false, addLineToBuffer, createNewLine, clearScreen, lastLine);
				
				if (clearScreen)
				{
					ClearConsole();
					lastLine = app->lineList.lastLine;
				}
				else
				{
					if (addLineToBuffer)
					{
						lastLine = LineListPushLine(&app->lineList);
					}
					else
					{
						LineListClearLine(&app->lineList);
					}
				}
			}
		}
		
		// +==============================+
		// |        Drop Char Data        |
		// +==============================+
		u32 inputArenaMax        = (u32)(app->inputArenaSize/100.f * 98);
		u32 targetInputArenaSize = (u32)(app->inputArenaSize/100.f * 90);
		if (app->lineList.charDataSize >= inputArenaMax)
		{
			DropCharData(targetInputArenaSize);
		}
	}
	
	app->rxShiftRegister |= 0x80;
}

//NOTE: This function serves as a measuring function AS WELL AS
//		a buffer filling function if not passed nullptr for bufferOutput
u32 GetSelection(bool insertTimestamps, char* bufferOutput = nullptr)
{
	TextLocation_t minLocation = TextLocationMin(app->selectionStart, app->selectionEnd);
	TextLocation_t maxLocation = TextLocationMax(app->selectionStart, app->selectionEnd);
	
	if (minLocation.lineIndex == maxLocation.lineIndex &&
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
	
	if (minLocation.lineIndex == maxLocation.lineIndex)
	{
		Line_t* linePntr = LineListGetItemAt(&app->lineList, minLocation.lineIndex);
		
		if (insertTimestamps)
		{
			TempPushMark();
			u64 lineTimestamp = linePntr->timestamp;
			char* timeString = FormattedTimeStr(lineTimestamp);
			u32 timeStringLength = (u32)strlen(timeString);
			if (bufferOutput != nullptr)
			{
				*outputPntr = '['; outputPntr++;
				memcpy(outputPntr, timeString, timeStringLength);
				outputPntr += timeStringLength;
				*outputPntr = ']'; outputPntr++;
				*outputPntr = ' '; outputPntr++;
			}
			bufferLength += timeStringLength + 3;
			TempPopMark();
		}
		
		bufferLength += maxLocation.charIndex - minLocation.charIndex;
		if (bufferOutput != nullptr)
		{
			memcpy(outputPntr, &linePntr->chars[minLocation.charIndex], bufferLength);
			outputPntr += bufferLength;
		}
	}
	else
	{
		{ //First Line
			Line_t* minLinePntr = LineListGetItemAt(&app->lineList, minLocation.lineIndex);
			
			if (insertTimestamps)
			{
				TempPushMark();
				u64 lineTimestamp = minLinePntr->timestamp;
				char* timeString = FormattedTimeStr(lineTimestamp);
				u32 timeStringLength = (u32)strlen(timeString);
				if (bufferOutput != nullptr)
				{
					*outputPntr = '['; outputPntr++;
					memcpy(outputPntr, timeString, timeStringLength);
					outputPntr += timeStringLength;
					*outputPntr = ']'; outputPntr++;
					*outputPntr = ' '; outputPntr++;
				}
				bufferLength += timeStringLength + 3;
				TempPopMark();
			}
			
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
		for (i32 lineIndex = minLocation.lineIndex+1; lineIndex < maxLocation.lineIndex && lineIndex < app->lineList.numLines; lineIndex++)
		{
			Line_t* linePntr = LineListGetItemAt(&app->lineList, lineIndex);
			
			if (insertTimestamps)
			{
				TempPushMark();
				u64 lineTimestamp = linePntr->timestamp;
				char* timeString = FormattedTimeStr(lineTimestamp);
				u32 timeStringLength = (u32)strlen(timeString);
				if (bufferOutput != nullptr)
				{
					*outputPntr = '['; outputPntr++;
					memcpy(outputPntr, timeString, timeStringLength);
					outputPntr += timeStringLength;
					*outputPntr = ']'; outputPntr++;
					*outputPntr = ' '; outputPntr++;
				}
				bufferLength += timeStringLength + 3;
				TempPopMark();
			}
			
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
			Line_t* maxLinePntr = LineListGetItemAt(&app->lineList, maxLocation.lineIndex);
			
			if (insertTimestamps)
			{
				TempPushMark();
				u64 lineTimestamp = maxLinePntr->timestamp;
				char* timeString = FormattedTimeStr(lineTimestamp);
				u32 timeStringLength = (u32)strlen(timeString);
				if (bufferOutput != nullptr)
				{
					*outputPntr = '['; outputPntr++;
					memcpy(outputPntr, timeString, timeStringLength);
					outputPntr += timeStringLength;
					*outputPntr = ']'; outputPntr++;
					*outputPntr = ' '; outputPntr++;
				}
				bufferLength += timeStringLength + 3;
				TempPopMark();
			}
			
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

void LoadApplicationFonts()
{
	if (app->mainFont.numChars != 0) { DestroyTexture(&app->mainFont.bitmap); }
	if (app->uiFont.numChars != 0)   { DestroyTexture(&app->uiFont.bitmap);   }
	
	app->mainFont = LoadFont("Resources/Fonts/consola.ttf",
		(r32)GC->mainFontSize, 1024, 1024, ' ', 96);
	app->uiFont = LoadFont("Resources/Fonts/consola.ttf",
		(r32)GC->uiFontSize, 1024, 1024, ' ', 96);
}

void DrawPopupOverlay(RenderState_t* rs)
{
	if (app->popupMessage[0] != '\0' && platform->programTime - app->popupTime < app->popupDuration)
	{
		rec overlayRec = NewRectangle(
			RenderScreenSize.x,
			app->uiElements.mainMenuRec.y + app->uiElements.mainMenuRec.height + 5,
			POPUP_MAX_WIDTH, 0
		);
		
		if (overlayRec.width > (RenderScreenSize.x))
		{
			overlayRec.width = (RenderScreenSize.x);
		}
		
		r32 messagePadding = POPUP_MESSAGE_PADDING + MaxReal32((r32)GC->popupCornerRadius/2, (r32)GC->popupOutlineThickness);
		r32 textAreaWidth = overlayRec.width - messagePadding*2;
		v2 textSize = MeasureFormattedString(&app->uiFont, app->popupMessage, textAreaWidth, true);
		if (textSize.x < textAreaWidth)
		{
			overlayRec.width = textSize.x + messagePadding*2;
		}
		
		u64 timeSinceMessage = platform->programTime - app->popupTime;
		u64 timeTillEnd = app->popupDuration - timeSinceMessage;
		
		if (ButtonPressedUnhandled(Button_Escape))
		{
			if (timeTillEnd > GC->popupOutAnimTime)
			{
				HandleButton(Button_Escape);
				
				u64 timeToSubtract = timeTillEnd - (GC->popupOutAnimTime/2);
				if (app->popupTime >= timeToSubtract) { app->popupTime -= timeToSubtract; }
				else { app->popupTime = 0; }
				app->popupExcused = true;
				
				timeSinceMessage = platform->programTime - app->popupTime;
				timeTillEnd = app->popupDuration - timeSinceMessage;
			}
		}
		
		r32 tMessage = (r32)timeSinceMessage / (r32)app->popupDuration;
		
		r32 animAmount = 1.0f;
		if (tMessage < 0.5f)
		{
			if (timeSinceMessage < GC->popupInAnimTime)
			{
				animAmount = Ease(EasingStyle_QuinticOut, (r32)timeSinceMessage / GC->popupInAnimTime);
			}
		}
		else
		{
			i32 outTime = (app->popupExcused ? GC->popupOutAnimTime/2 : GC->popupOutAnimTime);
			if (timeTillEnd < outTime)
			{
				animAmount = Ease(EasingStyle_CubicOut, (r32)timeTillEnd / outTime);
			}
		}
		
		overlayRec.x -= overlayRec.width * animAmount;
		
		overlayRec.height = textSize.y + messagePadding*2;
		if (overlayRec.height < (r32)GC->popupCornerRadius*2)
		{
			overlayRec.height = (r32)GC->popupCornerRadius*2;
		}
		
		// rs->DrawRectangle(RectangleInflate(overlayRec, (r32)GC->menuBorderThickness), GC->colors.windowOutline);
		// rs->DrawGradient(overlayRec, GC->colors.statusBar1, GC->colors.statusBar2, Direction2D_Right);
		
		r32 cornerRadius = (r32)GC->popupCornerRadius;
		r32 innerRingThickness = (r32)GC->popupOutlineThickness;
		Color_t outerBorderColor = GC->colors.popupOutlineOuter;
		Color_t innerRingColor   = GC->colors.popupOutlineCenter;
		Color_t innerBorderColor = GC->colors.popupOutlineInner;
		Color_t innerColor = GC->colors.popupBackground;
		
		rec innerRec = NewRectangle(
			overlayRec.x,
			overlayRec.y + cornerRadius,
			overlayRec.width,
			overlayRec.height - cornerRadius*2
		);
		rec outerRec = NewRectangle(
			overlayRec.x + cornerRadius,
			overlayRec.y,
			overlayRec.width,
			overlayRec.height
		);
		v2 ulCircleCenter = NewVec2(
			overlayRec.x + cornerRadius,
			overlayRec.y + cornerRadius
		);
		r32 ulCIrcleRadius = cornerRadius;
		v2 blCircleCenter = NewVec2(
			overlayRec.x + cornerRadius,
			overlayRec.y + overlayRec.height - cornerRadius
		);
		r32 blCIrcleRadius = cornerRadius;
		
		rs->DrawRectangle(RectangleInflateX(innerRec, 1.0f), outerBorderColor);
		rs->DrawRectangle(RectangleInflateY(outerRec, 1.0f), outerBorderColor);
		rs->DrawCircle(ulCircleCenter, ulCIrcleRadius + 1.0f, outerBorderColor);
		rs->DrawCircle(blCircleCenter, blCIrcleRadius + 1.0f, outerBorderColor);
		
		rs->DrawRectangle(innerRec, innerRingColor);
		rs->DrawRectangle(outerRec, innerRingColor);
		rs->DrawCircle(ulCircleCenter, ulCIrcleRadius, innerRingColor);
		rs->DrawCircle(blCircleCenter, blCIrcleRadius, innerRingColor);
		
		innerRec = RectangleInflateX(innerRec, -innerRingThickness);
		outerRec = RectangleInflateY(outerRec, -innerRingThickness);
		ulCIrcleRadius -= innerRingThickness;
		blCIrcleRadius -= innerRingThickness;
		
		rs->DrawRectangle(innerRec, innerBorderColor);
		rs->DrawRectangle(outerRec, innerBorderColor);
		rs->DrawCircle(ulCircleCenter, ulCIrcleRadius, innerBorderColor);
		rs->DrawCircle(blCircleCenter, blCIrcleRadius, innerBorderColor);
		
		rs->DrawRectangle(RectangleInflate(innerRec, -1.0f), innerColor);
		rs->DrawRectangle(RectangleInflate(outerRec, -1.0f), innerColor);
		rs->DrawCircle(ulCircleCenter, ulCIrcleRadius - 1.0f, innerColor);
		rs->DrawCircle(blCircleCenter, blCIrcleRadius - 1.0f, innerColor);
		
		v2 textPos = overlayRec.topLeft + NewVec2(overlayRec.width/2, overlayRec.height/2 - textSize.y/2 + app->uiFont.maxExtendUp);
		
		rs->BindFont(&app->uiFont);
		rs->DrawFormattedString(app->popupMessage, textPos, textAreaWidth, app->popupColor, Alignment_Center, true);
		rs->BindFont(&app->mainFont);
	}
}

void DrawSelectionOnFormattedLine(RenderState_t* rs, Line_t* linePntr, v2 position, u32 startIndex, u32 endIndex, Color_t selectionColor)
{
	Assert(rs != nullptr);
	Assert(linePntr != nullptr);
	Assert(startIndex >= 0 && (u32)startIndex <= linePntr->numChars);
	Assert(endIndex >= 0 && (u32)endIndex <= linePntr->numChars);
	Assert(startIndex <= endIndex);
	
	// if (startIndex == endIndex) { return; }
	
	if (linePntr->numChars == 0)
	{
		rec selectionRec = NewRectangle(
			position.x, position.y,
			1, app->mainFont.lineHeight
		);
		selectionRec = RectangleInflate(selectionRec, (r32)GC->lineSpacing/2);
		rs->DrawRectangle(selectionRec, selectionColor);
		return;
	}
	
	u32 cIndex = 0;
	
	u32 numLines = 0;
	while (cIndex < linePntr->numChars)
	{
		u32 numChars = FindNextFormatChunk(&app->mainFont, &linePntr->chars[cIndex], linePntr->numChars - cIndex, linePntr->lineWrapWidth, GC->lineWrapPreserveWords);
		if (numChars == 0) { numChars = 1; }
		
		while (numChars > 1 && IsCharClassWhitespace(linePntr->chars[cIndex + numChars-1]))
		{
			numChars--;
		}
		
		if (cIndex < endIndex && cIndex+numChars > startIndex)
		{
			u32 measureStart = cIndex;
			u32 measureEnd = cIndex + numChars;
			if (measureStart < startIndex) { measureStart = startIndex; }
			if (measureEnd > endIndex) { measureEnd = endIndex; }
			r32 skipSize = 0;
			if (measureStart > cIndex) { skipSize = MeasureString(&app->mainFont, &linePntr->chars[cIndex], measureStart - cIndex).x; }
			r32 selectionWidth = MeasureString(&app->mainFont, &linePntr->chars[measureStart], measureEnd - measureStart).x;
			if (selectionWidth == 0) { selectionWidth = 1; }
			
			rec selectionRec = NewRectangle(
				position.x + skipSize,
				position.y + (app->mainFont.lineHeight*numLines),
				selectionWidth,
				app->mainFont.lineHeight
			);
			selectionRec = RectangleInflate(selectionRec, (r32)GC->lineSpacing/2);
			rs->DrawRectangle(selectionRec, selectionColor);
		}
		
		
		if (cIndex+numChars < linePntr->numChars && linePntr->chars[cIndex+numChars] == '\r')
		{
			numChars++;
		}
		if (cIndex+numChars < linePntr->numChars && linePntr->chars[cIndex+numChars] == '\n')
		{
			numChars++;
		}
		while (cIndex+numChars < linePntr->numChars && linePntr->chars[cIndex+numChars] == ' ')
		{
			numChars++;
		}
		
		numLines++;
		cIndex += numChars;
	}
}

//+================================================================+
//|                       App Get Version                          |
//+================================================================+
// Version_t App_GetVersion(bool* resetApplication)
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
// void App_Initialize(const PlatformInfo_t* PlatformInfo, const AppMemory_t* AppMemory)
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
	
	app->inputArenaBase = (char*)(app + 1);
	app->inputArenaSize = INPUT_ARENA_SIZE;
	
	u32 mainHeapSize = AppMemory->permanantSize - sizeof(AppData_t) - INPUT_ARENA_SIZE;
	InitializeMemoryArenaHeap(&app->mainHeap, app->inputArenaBase + app->inputArenaSize, mainHeapSize);
	
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
	// void InitializeLineList(LineList_t* lineList, char* charStorageBase, u32 charStorageSize)
	InitializeLineList(&app->lineList, app->inputArenaBase, app->inputArenaSize);
	
	DEBUG_WriteLine("Creating menus");
	
	Menu_t* comMenu = AddMenu(&app->menuHandler, "COM Menu", NewRectangle((r32)RenderScreenSize.x / 2 - 50, (r32)RenderScreenSize.y / 2 - 150, 400, 300),
		ComMenuUpdate, ComMenuRender);
	comMenu->show = false;
	Menu_t* contextMenu = AddMenu(&app->menuHandler, "Context Menu", NewRectangle(0, 0, 100, 100),
		ContextMenuUpdate, ContextMenuRender);
	contextMenu->titleBarSize = 0;
	contextMenu->show = false;
	Menu_t* aboutMenu = AddMenu(&app->menuHandler, "About Menu", NewRectangle(0, 0, 400, 400),
		AboutMenuUpdate, AboutMenuRender);
	aboutMenu->show = false;
	
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
	app->crappyLogo = LoadTexture("Resources/Sprites/crappyLogo.png", false, false);
	app->pythonIcon = LoadTexture("Resources/Sprites/python.png", false, false);
	
	LoadApplicationFonts();
	
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
	
	// +==============================+
	// |   Auto-Start Python Script   |
	// +==============================+
	if (GC->autoRunPython && GC->pythonScriptEnabled && GC->pythonScript != nullptr)
	{
		char* commandStr = TempPrint("python %s", GC->pythonScript);
		StatusInfo("Running System Command: \"%s\"", commandStr);
		app->programInstance = platform->StartProgramInstance(commandStr);
		if (app->programInstance.isOpen == false)
		{
			StatusError("Python exec failed: \"%s\"", commandStr);
		}
	}
	
	TempPopMark();
	DEBUG_WriteLine("Initialization Done!");
	app->appInitTempHighWaterMark = ArenaGetHighWaterMark(TempArena);
	ArenaResetHighWaterMark(TempArena);
}

//+================================================================+
//|                        App Reloaded                            |
//+================================================================+
// void App_Reloaded(const PlatformInfo_t* PlatformInfo, const AppMemory_t* AppMemory)
EXPORT AppReloaded_DEFINITION(App_Reloaded)
{
	platform = PlatformInfo;
	app = (AppData_t*)AppMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	
	PopupSuccess("App Reloaded");
	
	//Make sure our callbacks still match the location of the functions in the new DLL
	Menu_t* menuPntr = GetMenuByName(&app->menuHandler, "COM Menu");
	menuPntr->specialPntr = nullptr;
	menuPntr->updateFunctionPntr = (void*)ComMenuUpdate;
	menuPntr->renderFunctionPntr = (void*)ComMenuRender;
	menuPntr = GetMenuByName(&app->menuHandler, "Context Menu");
	menuPntr->specialPntr = nullptr;
	menuPntr->updateFunctionPntr = (void*)ContextMenuUpdate;
	menuPntr->renderFunctionPntr = (void*)ContextMenuRender;
	menuPntr = GetMenuByName(&app->menuHandler, "About Menu");
	menuPntr->specialPntr = nullptr;
	menuPntr->updateFunctionPntr = (void*)AboutMenuUpdate;
	menuPntr->renderFunctionPntr = (void*)AboutMenuRender;
}

//+================================================================+
//|                         App Update                             |
//+================================================================+
// void App_Update(const PlatformInfo_t* PlatformInfo, const AppMemory_t* AppMemory, const AppInput_t* AppInput, AppOutput_t* AppOutput)
EXPORT AppUpdate_DEFINITION(App_Update)
{
	platform = PlatformInfo;
	input = AppInput;
	app = (AppData_t*)AppMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	RenderMousePos = NewVec2(input->mousePos.x, input->mousePos.y) / (r32)GUI_SCALE;
	RenderMouseStartPos = NewVec2(input->mouseStartPos[MouseButton_Left].x, input->mouseStartPos[MouseButton_Left].y) / (r32)GUI_SCALE;
	AppOutput->cursorType = Cursor_Default;
	
	UiElements_t* ui = &app->uiElements;
	RenderState_t* rs = &app->renderState;
	Menu_t* comMenu = GetMenuByName(&app->menuHandler, "COM Menu");
	Menu_t* contextMenu = GetMenuByName(&app->menuHandler, "Context Menu");
	Menu_t* aboutMenu = GetMenuByName(&app->menuHandler, "About Menu");
	Menu_t* hoverMenu = GetMenuAtPoint(&app->menuHandler, RenderMousePos);
	Color_t color1 = ColorFromHSV((i32)(platform->programTime*180) % 360, 1.0f, 1.0f);
	Color_t color2 = ColorFromHSV((i32)(platform->programTime*180 + 125) % 360, 1.0f, 1.0f);
	Color_t selectionColor = ColorLerp(GC->colors.selection1, GC->colors.selection2, (Sin32((platform->programTime/1000.0f)*6.0f) + 1.0f) / 2.0f);
	
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
		Line_t* linePntr = LineListGetItemAt(&app->lineList, ui->mouseTextLocation.lineIndex);
		
		if (linePntr != nullptr && linePntr->timestamp != 0)
		{
			RealTime_t lineTime = RealTimeAt(linePntr->timestamp);
			
			if (ButtonDown(Button_Shift))
			{
				ui->contextString = TempPrint("%s %u:%02u:%02u%s (%s %s, %u)",
					GetDayOfWeekStr(GetDayOfWeek(lineTime)),
					Convert24HourTo12Hour(lineTime.hour), lineTime.minute, lineTime.second,
					IsPostMeridian(lineTime.hour) ? "pm" : "am",
					GetMonthStr((Month_t)lineTime.month), GetDayOfMonthString(lineTime.day), lineTime.year
				);
			}
			else
			{
				i64 secondsDifference = SubtractTimes(platform->localTime, lineTime, TimeUnit_Seconds);
				i64 absDifference = Abs64i(secondsDifference);
				ui->contextString = TempPrint("%s Ago", GetElapsedString((u64)absDifference));
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
		u32 readCount = 0;
		while (readResult > 0)
		{
			char buffer[4096] = {};
			readResult = platform->ReadComPort(&app->comPort, buffer, ArrayCount(buffer)-1);
			if (readResult > 0)
			{
				DEBUG_PrintLine("Read %d bytes \"%.*s\"", readResult, readResult, buffer);
				
				if (app->programInstance.isOpen == false ||
					GC->sendComDataToPython == false ||
					GC->alsoShowComData == true)
				{
					DataReceived(buffer, readResult);
				}
				
				if (app->programInstance.isOpen && GC->sendComDataToPython)
				{
					DEBUG_PrintLine("Writing to program instance \"%.*s\"", readResult, buffer);
					
					u32 numBytesWritten = platform->WriteProgramInput(&app->programInstance, &buffer[0], readResult);
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
			
			readCount++;
			if (readCount >= MAX_COM_READ_LOOPS)
			{
				StatusError("Too much information recieved in a single frame");
				break;
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
			numBytesRead = platform->ReadProgramOutput(&app->programInstance, readBuffer, ArrayCount(readBuffer));
			
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
		
		ProgramStatus_t programStatus = platform->GetProgramStatus(&app->programInstance);
		if (programStatus != ProgramStatus_Running)
		{
			DEBUG_WriteLine("Program instance finished!");
			platform->CloseProgramInstance(&app->programInstance);
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
			if (GC->showInputTextBox) //put the new text in the inputTextBox
			{
				for (u32 cIndex = 0; cIndex < input->textInputLength; cIndex++)
				{
					char newChar = input->textInput[cIndex];
					if (app->inputTextLength+1 < ArrayCount(app->inputText))
					{
						app->inputText[app->inputTextLength] = newChar;
						app->inputTextLength++;
						app->inputTextCursor++;
						app->inputText[app->inputTextLength] = '\0';
						DEBUG_PrintLine("Added 0x%02X \'%c\'", newChar, newChar);
					}
					else
					{
						StatusError("Input text box is full");
					}
				}
			}
			else //route the input immediately
			{
				DEBUG_PrintLine("Writing \"%.*s\"", input->textInputLength, input->textInput);
				if (echoInput) { DataReceived(&input->textInput[0], input->textInputLength); }
				if (writeToComPort)
				{
					platform->WriteComPort(&app->comPort, &input->textInput[0], input->textInputLength);
					app->txShiftRegister |= 0x80;
				}
				if (writeToProgram) { platform->WriteProgramInput(&app->programInstance, &input->textInput[0], input->textInputLength); }
			}
		}
		
		bool sendButtonPressed = false;
		if (IsInsideRectangle(RenderMousePos, ui->sendButtonRec) && input->mouseInsideWindow && !ui->mouseInMenu)
		{
			AppOutput->cursorType = Cursor_Pointer;
			if (ButtonReleased(MouseButton_Left) && IsInsideRectangle(RenderMouseStartPos, ui->sendButtonRec))
			{
				sendButtonPressed = true;
			}
		}
		
		if (comMenu->show == false && (ButtonPressed(Button_Enter) || sendButtonPressed))
		{
			if (GC->showInputTextBox)
			{
				if (app->inputTextLength > 0)
				{
					DEBUG_PrintLine("Writing %u byte input: \"%.*s\"", app->inputTextLength, app->inputTextLength, app->inputText);
					if (echoInput) { DataReceived(app->inputText, app->inputTextLength); }
					if (writeToComPort)
					{
						platform->WriteComPort(&app->comPort, app->inputText, app->inputTextLength);
						app->txShiftRegister |= 0x80;
					}
					if (writeToProgram) { platform->WriteProgramInput(&app->programInstance, app->inputText, app->inputTextLength); }
					
					memcpy(app->lastInputText, app->inputText, ArrayCount(app->inputText));
					app->lastInputTextLength = app->inputTextLength;
					
					app->inputTextLength = 0;
					app->inputTextCursor = 0;
					app->inputText[app->inputTextLength] = '\0';
				}
				else
				{
					StatusInfo("Writing blank line");
				}
				
				if (true) //Write new line after text input
				{
					const char* newLineStr = GC->newLineString;
					if (newLineStr == nullptr) { newLineStr = "\n"; }
					u32 newLineStrLength = (u32)strlen(newLineStr);
					if (echoInput) { DataReceived(newLineStr, newLineStrLength); }
					if (writeToComPort)
					{
						platform->WriteComPort(&app->comPort, newLineStr, newLineStrLength);
						app->txShiftRegister |= 0x80;
					}
					if (writeToProgram) { platform->WriteProgramInput(&app->programInstance, newLineStr, newLineStrLength); }
				}
			}
			else
			{
				DEBUG_WriteLine("Writing New Line");
				
				const char* newLineStr = GC->newLineString;
				if (newLineStr == nullptr) { newLineStr = "\n"; }
				u32 newLineStrLength = (u32)strlen(newLineStr);
				if (echoInput) { DataReceived(newLineStr, newLineStrLength); }
				if (writeToComPort)
				{
					platform->WriteComPort(&app->comPort, newLineStr, newLineStrLength);
					app->txShiftRegister |= 0x80;
				}
				if (writeToProgram) { platform->WriteProgramInput(&app->programInstance, newLineStr, newLineStrLength); }
			}
		}
		
		if (!comMenu->show && ButtonPressed(Button_Backspace))
		{
			if (GC->showInputTextBox)
			{
				if (app->inputTextLength > 0)
				{
					app->inputTextLength--;
					app->inputTextCursor--;
					app->inputText[app->inputTextLength] = '\0';
				}
				else
				{
					// StatusInfo("No characters to delete");
				}
			}
			else
			{
				DEBUG_WriteLine("Writing Backspace");
				
				char newChar = '\b';
				if (echoInput) { DataReceived("\b", 1); }
				if (writeToComPort)
				{
					platform->WriteComPort(&app->comPort, &newChar, 1);
					app->txShiftRegister |= 0x80;
				}
				if (writeToProgram)
				{
					platform->WriteProgramInput(&app->programInstance, &newChar, 1);
				}
			}
		}
	
		// +==============================+
		// |     Paste from Clipboard     |
		// +==============================+
		if (ButtonDown(Button_Control) && ButtonPressed(Button_V))
		{
			TempPushMark();
			
			u32 clipboardDataSize = 0;
			char* clipbardData = (char*)platform->CopyFromClipboard(TempArena, &clipboardDataSize);
			
			if (clipbardData != nullptr)
			{
				StatusSuccess("Pasted %u bytes from clipboard", clipboardDataSize);
				
				char* sanatized = SanatizeStringAdvanced(clipbardData, clipboardDataSize, TempArena, true, false, true);
				u32 sanatizedLength = (u32)strlen(sanatized);
				
				if (echoInput) { DataReceived(sanatized, sanatizedLength); }
				if (writeToComPort)
				{
					platform->WriteComPort(&app->comPort, sanatized, sanatizedLength);
					app->txShiftRegister |= 0x80;
				}
				if (writeToProgram) { platform->WriteProgramInput(&app->programInstance, sanatized, sanatizedLength); }
			}
			
			TempPopMark();
		}
	}
	
	// +==============================+
	// |      Recall Last Input       |
	// +==============================+
	if (GC->showInputTextBox && ButtonPressed(Button_Up))
	{
		memcpy(app->inputText, app->lastInputText, ArrayCount(app->inputText));
		app->inputTextLength = app->lastInputTextLength;
		app->inputTextCursor = app->lastInputTextLength;
	}
	if (GC->showInputTextBox && ButtonPressed(Button_Down))
	{
		app->inputTextLength = 0;
		app->inputText[0] = '\0';
		app->inputTextCursor = 0;
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
	if (ButtonDown(Button_Control) && ButtonPressed(Button_C))
	{
		if (ButtonDown(Button_Shift))
		{
			ClearConsole();
		}
		else
		{
			u32 selectionSize = GetSelection(false);
			if (selectionSize != 0)
			{
				TempPushMark();
				
				char* selectionTempBuffer = TempString(selectionSize+1);
				GetSelection(false, selectionTempBuffer);
				
				platform->CopyToClipboard(selectionTempBuffer, selectionSize);
				StatusSuccess("Copied %u bytes to clipboard", selectionSize);
				
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
		if (ButtonDown(Button_Shift))
		{
			//Reload shaders
			app->simpleShader = LoadShader("Resources/Shaders/simple-vertex.glsl", "Resources/Shaders/simple-fragment.glsl");
			PopupSuccess("Reloaded shaders");
		}
		else
		{
			DisposeGlobalConfig(&app->globalConfig);
			LoadGlobalConfiguration(platform, &app->globalConfig, &app->mainHeap);
			DisposeRegexFile(&app->regexList);
			LoadRegexFile(&app->regexList, "Resources/Configuration/RegularExpressions.rgx", &app->mainHeap);
			LoadApplicationFonts();
			
			PopupSuccess("Reloaded application configuration options");
		}
	}
	
	//+==================================+
	//|       Select/Deselect all        |
	//+==================================+
	if (ButtonPressed(Button_A) && ButtonDown(Button_Control))
	{
		if (app->selectionStart.lineIndex == app->selectionEnd.lineIndex &&
			app->selectionStart.charIndex == app->selectionEnd.charIndex)
		{
			//Select all
			app->selectionStart = NewTextLocation(0, 0);
			app->selectionEnd = NewTextLocation(app->lineList.numLines-1, app->lineList.lastLine->numChars);
			
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
			platform->CloseFile(&app->outputFile);
			app->writeToFile = false;
			StatusSuccess("Stopped outputting to file");
		}
		else
		{
			if (platform->OpenFile(outputFileName, &app->outputFile))
			{
				StatusSuccess("Opened file successfully");
				app->writeToFile = true;
				const char* newString = "\r\n\r\n[File Opened for Writing]\r\n";
				platform->AppendFile(&app->outputFile, newString, (u32)strlen(newString));
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
			
			if (IsComAvailable(quickComSelection) == false)
			{
				PopupError("%s not Available!", quickComSelection);
			}
			else
			{
				OpenComPort(quickComSelection, app->comMenuOptions.settings);
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
	
	// +==============================+
	// |         Debug Popups         |
	// +==============================+
	#if 0
	#if DEBUG
	if (ButtonPressed(Button_1))
	{
		PopupDebug("Something interesting is happening behind the scenes");
		StatusDebug("Something interesting is happening behind the scenes");
	}
	if (ButtonPressed(Button_2))
	{
		PopupInfo("We are working :)");
		StatusInfo("We are working :)");
	}
	if (ButtonPressed(Button_3))
	{
		PopupSuccess("That was awesome! Do it again.");
		StatusSuccess("That was awesome! Do it again.");
	}
	if (ButtonPressed(Button_4))
	{
		PopupError("An error has occurred. The world might be ending...");
		StatusError("An error has occurred. The world might be ending...");
	}
	if (ButtonPressed(Button_5))
	{
		PopupErrorTimed(10000, "This is a really long message for the user to read. Because it is so long we are going to give you 10 seconds to read it.");
		StatusErrorTimed(10000, "This is a really long message for the user to read. Because it is so long we are going to give you 10 seconds to read it.");
	}
	
	// +==============================+
	// |        Debug Message         |
	// +==============================+
	if (ButtonPressed(Button_F2))
	{
		u32 testMessageLength = (u32)strlen(testMessage);
		if (app->programInstance.isOpen && ButtonDown(Button_Shift))
		{
			DEBUG_PrintLine("Writing to program instance \"%.*s\"", testMessageLength, testMessage);
			
			u32 numBytesWritten = platform->WriteProgramInput(&app->programInstance, &testMessage[0], testMessageLength);
			if ((i32)numBytesWritten != testMessageLength)
			{
				DEBUG_PrintLine("Only wrote %u/%u bytes to program instance", numBytesWritten, testMessageLength);
			}
		}
		else
		{
			DataReceived(testMessage, testMessageLength);
		}
	}
	
	if (ButtonPressed(Button_F1) && app->lineList.charDataSize >= 1)
	{
		DropCharData(app->lineList.charDataSize - 1);
	}
	#endif
	#endif
	
	
	RecalculateUiElements(ui, false);
	
	if (ButtonDown(Button_Right))
	{
		ui->scrollOffsetGoto.x += ButtonDown(Button_Shift) ? 16 : 5;
	}
	if (ButtonDown(Button_Left))
	{
		ui->scrollOffsetGoto.x -= ButtonDown(Button_Shift) ? 16 : 5;
	}
	if (GC->showInputTextBox == false && ButtonDown(Button_Down))
	{
		ui->scrollOffsetGoto.y += ButtonDown(Button_Shift) ? 16 : 5;
	}
	if (GC->showInputTextBox == false && ButtonDown(Button_Up))
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
		IsInsideRectangle(RenderMouseStartPos, ui->gotoEndButtonRec));
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
	
	// +==============================+
	// |    Start Python Shortcut     |
	// +==============================+
	if (GC->pythonScriptEnabled && ButtonPressed(Button_P) && ButtonDown(Button_Control))
	{
		if (app->programInstance.isOpen)
		{
			StatusInfo("Closing python instance");
			platform->CloseProgramInstance(&app->programInstance);
		}
		else
		{
			if (GC->pythonScript != nullptr)
			{
				char* commandStr = TempPrint("python %s", GC->pythonScript);
				StatusInfo("Running System Command: \"%s\"", commandStr);
				app->programInstance = platform->StartProgramInstance(commandStr);
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
		u32 selectionLength = GetSelection(false, nullptr);
		if (selectionLength > 0)
		{
			const char* countExpression = GetRegularExpression(&app->regexList, GC->genericCountRegexName);
			if (countExpression != nullptr)
			{
				TempPushMark();
				
				char* selectionBuffer = TempString(selectionLength);
				GetSelection(false, selectionBuffer);
				
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
	if (input->mouseInsideWindow && !ui->mouseInMenu)
	{
		for (u32 bIndex = 0; bIndex < ArrayCount(ui->buttonRecs); bIndex++)
		{
			v2 buttonCenter = ui->buttonRecs[bIndex].topLeft + ui->buttonRecs[bIndex].size/2.0f;
			r32 buttonRadius = ui->buttonRecs[bIndex].width/2.0f;
			if (Vec2Length(RenderMousePos - buttonCenter) <= buttonRadius)
			{
				AppOutput->cursorType = Cursor_Pointer;
				
				if (ButtonReleased(MouseButton_Left) && Vec2Length(RenderMouseStartPos - buttonCenter) <= buttonRadius)
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
								platform->LaunchFile("Resources/Configuration/PlatformConfig.json");
								platform->LaunchFile("Resources/Configuration/RegularExpressions.rgx");
								platform->LaunchFile("Resources/Configuration/GlobalConfig.json");
							}
							else if (ButtonDown(Button_Shift))
							{
								platform->LaunchFile("Resources/Configuration/PlatformConfig.json");
							}
							else if (ButtonDown(Button_Control))
							{
								platform->LaunchFile("Resources/Configuration/RegularExpressions.rgx");
							}
							else
							{
								platform->LaunchFile("Resources/Configuration/GlobalConfig.json");
							}
						} break;
						
						case Button_About:
						{
							aboutMenu->show = !aboutMenu->show;
							if (aboutMenu->show)
							{
								aboutMenu->drawRec.x = RenderScreenSize.x / 2 - aboutMenu->drawRec.width/2;
								aboutMenu->drawRec.y = RenderScreenSize.y / 2 - aboutMenu->drawRec.height/2;
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
	}
	
	//+================================+
	//|       Clear Button Press       |
	//+================================+
	if (IsInsideRectangle(RenderMousePos, ui->clearButtonRec) && input->mouseInsideWindow && !ui->mouseInMenu)
	{
		AppOutput->cursorType = Cursor_Pointer;
		if (ButtonReleased(MouseButton_Left) &&
			IsInsideRectangle(RenderMouseStartPos, ui->clearButtonRec))
		{
			ClearConsole();
		}
	}
	
	//+==================================+
	//|    Save To File Button Press     |
	//+==================================+
	bool saveButtonPressed = false;
	if (IsInsideRectangle(RenderMousePos, ui->saveButtonRec) && input->mouseInsideWindow && !ui->mouseInMenu)
	{
		AppOutput->cursorType = Cursor_Pointer;
		if (ButtonReleased(MouseButton_Left) && IsInsideRectangle(RenderMouseStartPos, ui->saveButtonRec))
		{
			saveButtonPressed = true;
		}
	}
	if (saveButtonPressed || (ButtonDown(Button_Control) && ButtonPressed(Button_S)))
	{
		char* fileName = TempPrint("ConstPortSave_%02u-%02u-%u_%u-%02u-%02u.txt",
			platform->localTime.year, platform->localTime.month, platform->localTime.day,
			platform->localTime.hour, platform->localTime.minute, platform->localTime.second
		);
		
		u32 selectionSize = GetSelection(GC->saveTimesToFile, nullptr);
		if (selectionSize > 0)
		{
			TempPushMark();
			
			char* fileBuffer = TempString(selectionSize);
			GetSelection(GC->saveTimesToFile, fileBuffer);
			
			//NOTE: GetSelection adds a \0 on the end so need to remove it
			DEBUG_PrintLine("Saving %u bytes to %s", selectionSize-1, fileName);
			platform->WriteEntireFile(fileName, fileBuffer, selectionSize-1);
			DEBUG_WriteLine("Done!");
			
			TempPopMark();
			
			if (GC->showFileAfterSaving)
			{
				if (platform->LaunchFile(fileName))
				{
					DEBUG_WriteLine("Opened output file for viewing");
				}
				else
				{
					DEBUG_WriteLine("Could not open output file");
				}
			}
			
			StatusSuccess("Saved to %s", fileName);
		}
	}
	
	//+==========================================+
	//| Scrollbar Interaction and Text Selection |
	//+==========================================+
	if (ButtonDown(MouseButton_Left) && !ui->mouseInMenu)
	{
		//Handle scrollbar interaction with mouse
		if (IsInsideRectangle(RenderMouseStartPos, ui->scrollBarGutterRec) &&
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
				
				if (ui->scrollOffsetGoto.y < ui->maxScrollOffset.y - app->mainFont.lineHeight)
				{
					ui->followingEndOfFile = false;
				}
			}
		}
		else if (IsInsideRectangle(RenderMouseStartPos, ui->viewRec))
		{
			if (input->buttons[MouseButton_Left].transCount > 0)//Pressed the button down
			{
				app->selectionStart = ui->mouseTextLocation;
				app->selectionEnd = ui->mouseTextLocation;
			}
			else //if (IsInsideRectangle(RenderMousePos, ui->viewRec)) //Mouse Button Holding
			{
				app->selectionEnd = ui->mouseTextLocation;
			}
		}
	}
	
	//+================================+
	//|   Mark lines using the mouse   |
	//+================================+
	if (ui->mouseInMenu == false &&
		IsInsideRectangle(RenderMousePos, ui->gutterRec) && IsInsideRectangle(RenderMouseStartPos, ui->gutterRec))
	{
		if (ButtonReleased(MouseButton_Left) &&
			ui->markIndex >= 0 && ui->markIndex < app->lineList.numLines)
		{
			Line_t* linePntr = LineListGetItemAt(&app->lineList, ui->markIndex);
			
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
		Line_t* linePntr = LineListGetItemAt(&app->lineList, lastIndex);
		if (linePntr->numChars == 0 && lastIndex > 0)
		{
			linePntr = LineListGetItemAt(&app->lineList, lastIndex - 1);
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
	
	// +==============================+
	// |       Show Text Cursor       |
	// +==============================+
	if (IsInsideRectangle(RenderMousePos, ui->viewRec) && !ui->mouseInMenu && GC->showTextCursor)
	{
		AppOutput->cursorType = Cursor_Text;
	}
	
	// +==============================+
	// |       Close Shortcuts        |
	// +==============================+
	if (ButtonPressed(Button_W) && ButtonDown(Button_Control))
	{
		if (ButtonDown(Button_Shift))
		{
			AppOutput->closeWindow = true;
		}
		else
		{
			if (app->comPort.isOpen)
			{
				PopupError("Closed \"%s\"", app->comPort.name);
				platform->CloseComPort(&app->mainHeap, &app->comPort);
				comMenu->show = false;
				ClearConsole();
			}
		}
	}
	
	// +--------------------------------------------------------------+
	// |                       Rendering Setup                        |
	// +--------------------------------------------------------------+
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	rs->SetViewport(NewRectangle(0, 0, (r32)platform->screenSize.x, (r32)platform->screenSize.y));
	
	v4 backgroundColorVec = ColorToVec4(GC->colors.textBackground);
	glClearColor(backgroundColorVec.x, backgroundColorVec.y, backgroundColorVec.z, backgroundColorVec.w);
	// glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	rs->BindFrameBuffer(nullptr);
	rs->BindShader(&app->simpleShader);
	rs->BindFont(&app->mainFont);
	rs->SetGradientEnabled(false);
	rs->SetCircleRadius(0.0f, 0.0f);
	
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
		r32 lineWrapWidth = ui->viewRec.width - GC->lineSpacing;
		i32 firstLine = max(0, ui->firstRenderLine);
		
		rs->SetViewMatrix(Matrix4Translate(NewVec3(ui->viewRec.x - ui->scrollOffset.x, ui->viewRec.y - ui->scrollOffset.y, 0)));
		{//Items drawn relative to view
			
			v2 currentPos = NewVec2((r32)GC->lineSpacing, ui->scrollOffset.y - ui->firstRenderLineOffset + app->mainFont.maxExtendUp);
			for (i32 lineIndex = firstLine; lineIndex < app->lineList.numLines; lineIndex++)
			{
				Line_t* linePntr = LineListGetItemAt(&app->lineList, lineIndex);
				
				v2 lineSize = RenderLine(linePntr, currentPos, lineWrapWidth, true);
				
				RenderLine(linePntr, currentPos, lineWrapWidth, false);
			
				// +==============================+
				// |     Draw the File Cursor     |
				// +==============================+
				if (GC->showFileCursor && lineIndex == app->lineList.numLines-1 && platform->windowHasFocus)
				{
					Color_t fileCursorColor  = ColorLerp(GC->colors.fileCursor1, GC->colors.fileCursor2, (Sin32((platform->programTime/1000.0f)*8.0f) + 1.0f) / 2.0f);
					v2 skipSize = MeasureString(&app->mainFont, linePntr->chars, linePntr->numChars);
					rec cursorRec = NewRectangle(
						currentPos.x + skipSize.x,
						currentPos.y - app->mainFont.maxExtendUp,
						1, app->mainFont.lineHeight
					);
					rs->DrawRectangle(cursorRec, fileCursorColor);
				}
				
				// +==============================+
				// |    Draw the Hover Cursor     |
				// +==============================+
				if (GC->showHoverCursor && input->mouseInsideWindow &&
					lineIndex == ui->mouseTextLocation.lineIndex &&
					IsInsideRectangle(RenderMousePos, ui->viewRec) &&
					!ui->mouseInMenu)
				{
					Color_t hoverCursorColor  = ColorLerp(GC->colors.hoverCursor1, GC->colors.hoverCursor2, (Sin32((platform->programTime/1000.0f)*8.0f) + 1.0f) / 2.0f);
					const char* skipStrPntr = &linePntr->chars[ui->mouseTextLocation.charIndex - ui->mouseTextLineLocation.charIndex];
					v2 skipSize = MeasureString(&app->mainFont, skipStrPntr, ui->mouseTextLineLocation.charIndex);
					rec cursorRec = NewRectangle(
						currentPos.x + skipSize.x,
						currentPos.y - app->mainFont.maxExtendUp + (ui->mouseTextLineLocation.lineIndex * app->mainFont.lineHeight),
						1, app->mainFont.lineHeight
					);
					rs->DrawRectangle(cursorRec, hoverCursorColor);
				}
				
				currentPos.y += lineSize.y + GC->lineSpacing;
				if (currentPos.y - app->mainFont.maxExtendUp >= ui->scrollOffset.y + ui->viewRec.height)
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
	if (app->selectionStart.lineIndex != app->selectionEnd.lineIndex ||
		app->selectionStart.charIndex != app->selectionEnd.charIndex)
	{
		TextLocation_t minLocation = TextLocationMin(app->selectionStart, app->selectionEnd);
		TextLocation_t maxLocation = TextLocationMax(app->selectionStart, app->selectionEnd);
		i32 firstLine = max(0, ui->firstRenderLine);
		
		rs->SetViewMatrix(Matrix4Translate(NewVec3(ui->viewRec.x - ui->scrollOffset.x, ui->viewRec.y - ui->scrollOffset.y, 0)));
		{//Items drawn relative to view
			
			v2 currentPos = NewVec2(0, ui->scrollOffset.y - ui->firstRenderLineOffset + app->mainFont.maxExtendUp);
			for (i32 lineIndex = firstLine;
				lineIndex < app->lineList.numLines && lineIndex <= maxLocation.lineIndex;
				lineIndex++)
			{
				Line_t* linePntr = LineListGetItemAt(&app->lineList, lineIndex);
				
				if (lineIndex >= minLocation.lineIndex && lineIndex <= maxLocation.lineIndex)
				{
					if (GC->lineWrapEnabled)
					{
						i32 startIndex = 0;
						i32 endIndex = linePntr->numChars;
						
						if (lineIndex == minLocation.lineIndex)
						{
							startIndex = minLocation.charIndex;
						}
						if (lineIndex == maxLocation.lineIndex)
						{
							endIndex = maxLocation.charIndex;
						}
						
						DrawSelectionOnFormattedLine(rs, linePntr,
							currentPos + NewVec2((r32)GC->lineSpacing, -app->mainFont.maxExtendUp),
							startIndex, endIndex, selectionColor);
					}
					else
					{
						v2 skipSize = Vec2_Zero;
						v2 selectionSize = Vec2_Zero;
						
						if (lineIndex == minLocation.lineIndex &&
							lineIndex == maxLocation.lineIndex)
						{
							skipSize = MeasureString(&app->mainFont, linePntr->chars, minLocation.charIndex);
							selectionSize = MeasureString(&app->mainFont, &linePntr->chars[minLocation.charIndex], maxLocation.charIndex - minLocation.charIndex);
						}
						else if (lineIndex == minLocation.lineIndex)
						{
							skipSize = MeasureString(&app->mainFont, linePntr->chars, minLocation.charIndex);
							selectionSize = MeasureString(&app->mainFont, &linePntr->chars[minLocation.charIndex]);
							// selectionSize.x += MeasureString(&app->mainFont, " ", 1).x;
						}
						else if (lineIndex == maxLocation.lineIndex)
						{
							selectionSize = MeasureString(&app->mainFont, linePntr->chars, maxLocation.charIndex);
						}
						else
						{
							selectionSize = MeasureString(&app->mainFont, linePntr->chars);
							// selectionSize.x += MeasureString(&app->mainFont, " ", 1).x;
						}
						
						rec backRec = NewRectangle(GC->lineSpacing + currentPos.x + skipSize.x, currentPos.y - app->mainFont.maxExtendUp, selectionSize.x, app->mainFont.lineHeight);//linePntr->lineHeight);
						backRec = RectangleInflate(backRec, (r32)GC->lineSpacing/2);
						rs->DrawRectangle(backRec, selectionColor);
						
						if (currentPos.y - app->mainFont.maxExtendUp >= ui->scrollOffset.y + ui->viewRec.height)
						{
							//We've reached the bottom of the view
							break;
						}
					}
				}
				
				currentPos.y += linePntr->size.y + GC->lineSpacing;
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
	rs->DrawGradient(ui->gutterRec, GC->colors.gutter1, GC->colors.gutter2, Direction2D_Right);
	{
		i32 firstLine = max(0, ui->firstRenderLine);
		
		rs->SetViewMatrix(Matrix4Translate(NewVec3(ui->gutterRec.x, ui->gutterRec.y - ui->scrollOffset.y, 0)));
		{//Items drawn relative to view
			
			v2 currentPos = NewVec2(0, ui->scrollOffset.y - ui->firstRenderLineOffset + app->mainFont.maxExtendUp);
			for (i32 lineIndex = firstLine; lineIndex < app->lineList.numLines; lineIndex++)
			{
				Line_t* linePntr = LineListGetItemAt(&app->lineList, lineIndex);
				
				RenderLineGutter(linePntr, lineIndex, currentPos);
				
				currentPos.y += linePntr->size.y + GC->lineSpacing;
				if (currentPos.y - app->mainFont.maxExtendUp >= ui->scrollOffset.y + ui->viewRec.height)
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
		rs->DrawGradient(ui->scrollBarGutterRec, GC->colors.scrollbarBack1, GC->colors.scrollbarBack2, Direction2D_Right);
		
		rec centerScrollBarRec = ui->scrollBarRec;
		centerScrollBarRec.y += ui->scrollBarRec.width;
		centerScrollBarRec.height -= 2 * ui->scrollBarRec.width;
		rec startCapRec = NewRectangle(ui->scrollBarRec.x, ui->scrollBarRec.y, ui->scrollBarRec.width, ui->scrollBarRec.width);
		rec endCapRec = NewRectangle(ui->scrollBarRec.x, ui->scrollBarRec.y + ui->scrollBarRec.height - ui->scrollBarRec.width, ui->scrollBarRec.width, ui->scrollBarRec.width);
		endCapRec.y += endCapRec.height;
		endCapRec.height = -endCapRec.height;
		rs->DrawRectangle(RectangleInflate(centerScrollBarRec, 1), GC->colors.scrollbarOutline);
		rs->BindAlphaTexture(&app->scrollBarEndcapTexture);
		rs->DrawRectangle(RectangleInflate(startCapRec, 1), GC->colors.scrollbarOutline);
		rs->DrawRectangle(RectangleInflate(endCapRec, 1), GC->colors.scrollbarOutline);
		
		rs->DrawGradient(startCapRec, GC->colors.scrollbar1, GC->colors.scrollbar2, Direction2D_Right);
		rs->DrawGradient(endCapRec, GC->colors.scrollbar1, GC->colors.scrollbar2, Direction2D_Right);
		rs->DisableAlphaTexture();
		rs->DrawGradient(centerScrollBarRec, GC->colors.scrollbar1, GC->colors.scrollbar2, Direction2D_Right);
	}
	
	// +==============================+
	// |   Render Text Input Field    |
	// +==============================+
	if (GC->showInputTextBox)
	{
		rs->DrawGradient(ui->textInputRec, GC->colors.textInputBox1, GC->colors.textInputBox2, Direction2D_Down);
		rs->DrawButton(ui->textInputRec, {Color_TransparentBlack}, GC->colors.textInputText, 1);
		
		v2 wholeInputSize = MeasureString(&app->mainFont, app->inputText, app->inputTextLength);
		r32 cursorOffset  = MeasureString(&app->mainFont, app->inputText, (u32)app->inputTextCursor).x;
		
		v2 textPosition = NewVec2(
			ui->textInputRec.x + 5,
			ui->textInputRec.y + ui->textInputRec.height/2 - app->mainFont.lineHeight/2 + app->mainFont.maxExtendUp
		);
		
		rs->DrawString(app->inputText, app->inputTextLength, textPosition, GC->colors.textInputText);
		
		if (platform->windowHasFocus)
		{
			Rectangle_t cursorRec = NewRectangle(
				textPosition.x + cursorOffset,
				textPosition.y - app->mainFont.maxExtendUp,
				1, app->mainFont.lineHeight
			);
			Color_t cursorColor = ColorLerp(GC->colors.textInputCursor1, GC->colors.textInputCursor2, (Sin32((platform->programTime/1000.0f)*6.0f) + 1.0f) / 2.0f); //TODO: Sin?
			rs->DrawRectangle(cursorRec, cursorColor);
		}
	}
	
	// +==============================+
	// |      Render Send Button      |
	// +==============================+
	if (GC->showInputTextBox)
	{
		v2 textSize = MeasureString(&app->uiFont, "Send");
		v2 textPos = ui->sendButtonRec.topLeft + ui->sendButtonRec.size/2.f - textSize/2.f + NewVec2(0, app->uiFont.maxExtendUp);
		
		Color_t buttonColor = GC->colors.button;
		Color_t textColor = GC->colors.buttonText;
		Color_t borderColor = GC->colors.buttonBorder;
		ButtonColorChoice(buttonColor, textColor, borderColor, ui->sendButtonRec, ButtonDown(Button_Enter), app->inputTextLength > 0);
		
		rs->DrawButton(ui->sendButtonRec, buttonColor, borderColor, 1);
		rs->BindFont(&app->uiFont);
		rs->DrawString("Send", textPos, textColor);
		rs->BindFont(&app->mainFont);
	}
	
	//+--------------------------------------+
	//|          Render Status Bar           |
	//+--------------------------------------+
	{
		rs->DrawGradient(ui->statusBarRec, GC->colors.statusBar1, GC->colors.statusBar2, Direction2D_Right);
		
		#if 0
		rs->BindFont(&app->uiFont);
		rs->PrintString(NewVec2(ui->statusBarRec.x+5, ui->statusBarRec.y + app->uiFont.maxExtendUp), GC->colors.uiText, 1.0f,
			"Mouse Line Location: (%d, %d)", ui->mouseTextLineLocation.lineIndex, ui->mouseTextLineLocation.charIndex);
		rs->BindFont(&app->mainFont);
		#endif
		
		// +==============================+
		// |  Render the Status Message   |
		// +==============================+
		{
			u32 stringLength = (u32)strlen(app->statusMessage);
			u32 animInTime = stringLength * GC->statusInAnimCharTime;
			u32 actualDuration = animInTime + app->statusDuration;
			v2 stringPos = NewVec2(5, RenderScreenSize.y-app->uiFont.maxExtendDown);
			r32 availableWidth = ui->statusBarRec.width - stringPos.x - 5;
			
			// +==============================+
			// |    Com Name in Status Bar    |
			// +==============================+
			if (app->comPort.isOpen && GC->showComNameInStatusBar)
			{
				const char* comPortUserName = GetPortUserName(app->comPort.name);
				v2 comNameSize = MeasureString(&app->uiFont, comPortUserName);
				v2 renderPos = NewVec2(ui->gotoEndButtonRec.x - comNameSize.x - 5, RenderScreenSize.y-app->uiFont.maxExtendDown);
				rs->BindFont(&app->uiFont);
				rs->DrawString(comPortUserName, renderPos, GC->colors.uiText, 1.0f);
				rs->BindFont(&app->mainFont);
				
				availableWidth = renderPos.x - stringPos.x - 5;
			}
			
			if (app->statusMessage[0] != '\0' && platform->programTime - app->statusTime < actualDuration)
			{
				u64 timeSinceMessage = platform->programTime - app->statusTime;
				u64 timeTillEnd = actualDuration - timeSinceMessage;
				
				u32 numCharacters = stringLength;
				if (timeSinceMessage < animInTime)
				{
					numCharacters = (u32)(stringLength * ((r32)timeSinceMessage / (r32)animInTime));
				}
				
				r32 alphaAmount = 1.0f;
				if (timeTillEnd < GC->statusOutAnimTime)
				{
					alphaAmount = Ease(EasingStyle_CubicOut, (r32)timeTillEnd / GC->statusOutAnimTime);
				}
				
				v2 stringSize = MeasureString(&app->uiFont, app->statusMessage, numCharacters);
				Color_t textColor = ColorTransparent(app->statusColor, alphaAmount);
				
				rs->BindFont(&app->uiFont);
				rs->DrawFormattedString(app->statusMessage, numCharacters, stringPos, availableWidth, textColor, Alignment_Left, false);
				if (numCharacters < stringLength)
				{
					v2 charPos = NewVec2(stringSize.x, 0);
					if (charPos.x < availableWidth)
					{
						rs->DrawString("|", 1, stringPos + charPos, textColor, 1.0f, Alignment_Left);
					}
				}
				rs->BindFont(&app->mainFont);
			}
		}
		
		// +==============================+
		// |     Draw Goto End Button     |
		// +==============================+
		{
			Color_t buttonColor = GC->colors.buttonDisabled;
			Color_t borderColor = GC->colors.buttonDisabledBorder;
			Color_t textColor   = GC->colors.buttonDisabledText;
			
			ButtonColorChoice(buttonColor, textColor, borderColor, ui->gotoEndButtonRec, false, false);
			
			rs->DrawButton(ui->gotoEndButtonRec, buttonColor, borderColor);
		}
		
		// +==============================+
		// |   Render Update Indicator    |
		// +==============================+
		if (GC->showUpdateIndicator)
		{
			static u32 indicatorStep = 0;
			
			rec indicatorRec = NewRectangle(
				ui->statusBarRec.x,
				ui->statusBarRec.y,
				3,
				ui->statusBarRec.height
			);
			indicatorStep++;
			if (indicatorStep >= (u32)indicatorRec.height)
			{
				indicatorStep = 0;
			}
			
			u32 pMin = indicatorStep;
			u32 pMax = (indicatorStep + (u32)indicatorRec.height*3/4) % (u32)indicatorRec.height;
			
			rs->DrawRectangle(indicatorRec, GC->colors.updateIndicatorColor2);
			
			if (pMin <= pMax)
			{
				rec rec1 = indicatorRec;
				rec1.height = (r32)pMax-(r32)pMin;
				rec1.y += indicatorRec.height - rec1.height;
				rec1.y -= pMin;
				
				rs->DrawRectangle(rec1, GC->colors.updateIndicatorColor1);
			}
			else
			{
				rec rec1 = indicatorRec;
				rec1.height = indicatorRec.height - pMin;
				
				rec rec2 = indicatorRec;
				rec2.height = (r32)pMax;
				rec2.y += indicatorRec.height - rec2.height;
				
				rs->DrawRectangle(rec1, GC->colors.updateIndicatorColor1);
				rs->DrawRectangle(rec2, GC->colors.updateIndicatorColor1);
			}
		}
	}
	
	//+--------------------------------------+
	//|           Render Main Menu           |
	//+--------------------------------------+
	rs->DrawGradient(ui->mainMenuRec, GC->colors.mainMenu1, GC->colors.mainMenu2, Direction2D_Down);
	// rs->DrawRectangle(NewRectangle(0, ui->mainMenuRec.height-1, ui->mainMenuRec.width, 1), GC->colors.uiGray4); //TODO: Reimplement line at bottom of menu?
	
	r32 mainMenuButtonsRight = 0;
	for (u32 bIndex = 0; bIndex < NumMainMenuButtons; bIndex++)
	{
		rec buttonRec = ui->buttonRecs[bIndex];
		// Color_t baseColor = {Color_White};
		// Color_t highlightColor = {Color_Red};
		// Color_t iconColor = {Color_White};
		
		// if (IsInsideRectangle(RenderMousePos, buttonRec) && !ui->mouseInMenu)
		// {
		// 	if (ButtonDown(MouseButton_Left) && IsInsideRectangle(RenderMouseStartPos, buttonRec))
		// 	{
		// 		// iconColor = Color_Highlight2;
		// 		highlightColor = {Color_Black};//Color_Highlight4;
		// 	}
		// }
		// highlightColor.a = 200;
		
		// rs->BindTexture(&ui->buttonBaseTexture);
		// rs->DrawTexturedRec(buttonRec, baseColor);
		
		// if (IsInsideRectangle(RenderMousePos, buttonRec) && !ui->mouseInMenu)
		// if (Vec2Length(RenderMousePos - buttonCenter) < buttonRadius)
		// {
		// 	rs->BindTexture(&ui->buttonHighlightTexture);
		// 	rs->DrawTexturedRec(buttonRec, highlightColor);
		// }
		
		v2 buttonCenter = buttonRec.topLeft + buttonRec.size/2;
		r32 buttonRadius = buttonRec.width/2;
		Color_t centerColor = GC->colors.mainMenuButton;
		Color_t borderColor = GC->colors.mainMenuButtonBorder;
		Color_t iconColor   = GC->colors.mainMenuButtonIcon;
		
		if (input->mouseInsideWindow && Vec2Length(RenderMousePos - buttonCenter) < buttonRadius)
		{
			centerColor = GC->colors.mainMenuButtonHover;
			borderColor = GC->colors.mainMenuButtonHoverBorder;
			iconColor   = GC->colors.mainMenuButtonHoverIcon;
			
			v2 mouseStartPos = RenderMouseStartPos;
			if (ButtonDown(MouseButton_Left) && Vec2Length(mouseStartPos - buttonCenter) < buttonRadius)
			{
				centerColor = GC->colors.mainMenuButtonPress;
				borderColor = GC->colors.mainMenuButtonPressBorder;
				iconColor   = GC->colors.mainMenuButtonPressIcon;
			}
		}
		
		rs->DrawCircle(buttonCenter, buttonRadius, borderColor);
		rs->DrawCircle(buttonCenter, buttonRadius-1, centerColor);
		
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
		
		v2 rxLedCenter = ui->rxLedRec.topLeft + ui->rxLedRec.size/2;
		r32 rxLedRadius = ui->rxLedRec.width/4 * 3;
		Color_t centerColor = GC->colors.receiveLed;
		if ((app->rxShiftRegister&0x80) > 0 ||
			(app->rxShiftRegister&0x40) > 0)
		{
			centerColor = GC->colors.receiveLedActive;
		}
		if (!GC->circularRxLed) { rs->DrawRectangle(ui->rxLedRec, centerColor); }
		else                    { rs->DrawCircle(rxLedCenter, rxLedRadius, centerColor); }
		
		v2 txLedCenter = ui->txLedRec.topLeft + ui->txLedRec.size/2;
		r32 txLedRadius = ui->txLedRec.width/4 * 3;
		centerColor = GC->colors.transmitLed;
		if ((app->txShiftRegister&0x80) > 0 ||
			(app->txShiftRegister&0x40) > 0)
		{
			centerColor = GC->colors.transmitLedActive;
		}
		if (!GC->circularTxLed) { rs->DrawRectangle(ui->txLedRec, centerColor); }
		else                    { rs->DrawCircle(txLedCenter, txLedRadius, centerColor); }
		
		for (u32 shift = 0; shift < sizeof(u8)*8; shift++)
		{
			u32 ringNumber = (7 - shift);
			
			if (IsFlagSet(app->rxShiftRegister, (1<<shift)))
			{
				if (GC->circularRxLed == false)
				{
					rec deflatedRec = RectangleInflate(ui->rxLedRec, (r32)(ringNumber+1) * GC->rxTxLedRingSize);
					rs->DrawButton(deflatedRec, {Color_TransparentBlack}, GC->colors.receiveLedRing, (r32)GC->rxTxLedRingSize);
				}
				else
				{
					r32 radius = rxLedRadius -0.2f + (ringNumber+1) * GC->rxTxLedRingSize;
					r32 innerRadius = rxLedRadius -0.2f + ringNumber * GC->rxTxLedRingSize - 1.8f;
					rs->DrawDonut(rxLedCenter, radius, innerRadius, GC->colors.receiveLedRing);
				}
			}
			
			if (IsFlagSet(app->txShiftRegister, (1<<shift)))
			{
				if (GC->circularTxLed == false)
				{
					rec deflatedRec = RectangleInflate(ui->txLedRec, (r32)(ringNumber+1) * GC->rxTxLedRingSize);
					rs->DrawButton(deflatedRec, {Color_TransparentBlack}, GC->colors.transmitLedRing, (r32)GC->rxTxLedRingSize);
				}
				else
				{
					r32 radius = txLedRadius -0.2f + (ringNumber+1) * GC->rxTxLedRingSize;
					r32 innerRadius = txLedRadius -0.2f + ringNumber * GC->rxTxLedRingSize - 1.8f;
					rs->DrawDonut(txLedCenter, radius, innerRadius, GC->colors.transmitLedRing);
				}
			}
		}
	}
	
	// +================================+
	// |          Clear Button          |
	// +================================+
	{
		const char* clearStr = "Clear";
		v2 textSize = MeasureString(&app->uiFont, clearStr);
		v2 textPos = NewVec2(
			ui->clearButtonRec.x + ui->clearButtonRec.width/2 - textSize.x/2,
			ui->clearButtonRec.y + ui->clearButtonRec.height/2 + app->uiFont.lineHeight/2 - app->uiFont.maxExtendDown
		);
		
		Color_t buttonColor = GC->colors.button;
		Color_t textColor   = GC->colors.buttonText;
		Color_t borderColor = GC->colors.buttonBorder;
		ButtonColorChoice(buttonColor, textColor, borderColor, ui->clearButtonRec, false, false);
		
		rs->DrawButton(ui->clearButtonRec, buttonColor, borderColor);
		rs->BindFont(&app->uiFont);
		rs->DrawString(clearStr, textPos, textColor);
		rs->BindFont(&app->mainFont);
	}
	
	// +==============================+
	// |     Python Running Label     |
	// +==============================+
	if (app->programInstance.isOpen)
	{
		rec pythonIconRec = NewRectangle(
			ui->txLedRec.x - GC->rxTxLedRingSize*8 - PYTHON_ICON_SIZE - 5,
			ui->txLedRec.y,
			PYTHON_ICON_SIZE,
			PYTHON_ICON_SIZE
		);
		rs->BindTexture(&app->pythonIcon);
		rs->DrawTexturedRec(pythonIconRec, {Color_White});
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
	if (app->selectionStart.lineIndex != app->selectionEnd.lineIndex ||
		app->selectionStart.charIndex != app->selectionEnd.charIndex)
	{
		const char* clearStr = "Save To File";
		v2 textSize = MeasureString(&app->uiFont, clearStr);
		v2 textPos = NewVec2(
			ui->saveButtonRec.x + ui->saveButtonRec.width/2 - textSize.x/2,
			ui->saveButtonRec.y + ui->saveButtonRec.height/2 + app->uiFont.lineHeight/2 - app->uiFont.maxExtendDown
		);
		
		Color_t buttonColor = GC->colors.button;
		Color_t textColor   = GC->colors.buttonText;
		Color_t borderColor = GC->colors.buttonBorder;
		ButtonColorChoice(buttonColor, textColor, borderColor, ui->saveButtonRec, false, false);
		
		rs->DrawButton(ui->saveButtonRec, buttonColor, borderColor);
		rs->BindFont(&app->uiFont);
		rs->DrawString(clearStr, textPos, textColor);
		rs->BindFont(&app->mainFont);
	}
	
	MenuHandlerDrawMenus(&app->renderState, &app->menuHandler);
	
	DrawPopupOverlay(&app->renderState);
	
	// +================================+
	// |       Draw Debug Overlay       |
	// +================================+
	#if DEBUG
	if (app->showDebugMenu)
	{
		rec overlayRec = NewRectangle(10, 10, (r32)RenderScreenSize.x - 10*2, (r32)RenderScreenSize.y - 10*2);
		
		rs->DrawButton(overlayRec, ColorTransparent({Color_Black}, 0.5f), ColorTransparent({Color_White}, 0.5f));
		
		v2 textPos = NewVec2(overlayRec.x + 5, overlayRec.y + 5 + app->uiFont.maxExtendUp);
		rs->BindFont(&app->uiFont);
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "AppData Size: %u/%u (%.3f%%)",
			sizeof(AppData_t), AppMemory->permanantSize,
			(r32)sizeof(AppData_t) / (r32)AppMemory->permanantSize * 100.0f);
		textPos.y += app->uiFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Input Arena: %u/%u (%.3f%%)",
			app->lineList.charDataSize, app->lineList.charDataMaxSize,
			(r32)app->lineList.charDataSize / (r32)app->lineList.charDataMaxSize * 100.0f);
		textPos.y += app->uiFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Main Heap: %u/%u (%.3f%%)",
			app->mainHeap.used, app->mainHeap.size,
			(r32)app->mainHeap.used / (r32)app->mainHeap.size * 100.0f);
		textPos.y += app->uiFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Temp Init: %u/%u (%.3f%%)",
			app->appInitTempHighWaterMark, TempArena->size,
			(r32)app->appInitTempHighWaterMark / (r32)TempArena->size * 100.0f);
		textPos.y += app->uiFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Temp Update: %u/%u (%.3f%%)",
			ArenaGetHighWaterMark(TempArena), TempArena->size,
			(r32)ArenaGetHighWaterMark(TempArena) / (r32)TempArena->size * 100.0f);
		textPos.y += app->uiFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Program Time: %lu", platform->programTime);
		textPos.y += app->uiFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Time Delta: %.2f", platform->timeDelta);
		textPos.y += app->uiFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "System Timestamp: %ld", GetTimestamp(platform->systemTime));
		textPos.y += app->uiFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "System Time: %s", FormattedTimeStr(platform->systemTime));
		textPos.y += app->uiFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Local Timestamp: %ld", GetTimestamp(platform->localTime));
		textPos.y += app->uiFont.lineHeight;
		
		rs->PrintString(textPos, {Color_White}, 1.0f, "Local Time: %s", FormattedTimeStr(platform->localTime));
		textPos.y += app->uiFont.lineHeight;
		
		rs->BindFont(&app->mainFont);
	}
	#endif
	
	// rs->DrawCircle(RenderMouseStartPos, input->mouseMaxDist[MouseButton_Left]/GUI_SCALE, {Color_Red});
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
// void App_GetSoundSamples(const PlatformInfo_t* PlatformInfo, const AppMemory_t* AppMemory, const AppInput_t* AppInput)
EXPORT AppGetSoundSamples_DEFINITION(App_GetSoundSamples)
{
	platform = PlatformInfo;
	input = AppInput;
	app = (AppData_t*)AppMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	RenderMousePos = NewVec2(input->mousePos.x, input->mousePos.y) / (r32)GUI_SCALE;
	RenderMouseStartPos = NewVec2(input->mouseStartPos[MouseButton_Left].x, input->mouseStartPos[MouseButton_Left].y) / (r32)GUI_SCALE;
}

//+================================================================+
//|                        App Closing                             |
//+================================================================+
// void App_Closing(const PlatformInfo_t* PlatformInfo, const AppMemory_t* AppMemory)
EXPORT AppClosing_DEFINITION(App_Closing)
{
	platform = PlatformInfo;
	app = (AppData_t*)AppMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	
	DEBUG_WriteLine("Application closing!");
	
	if (app->programInstance.isOpen)
	{
		platform->CloseProgramInstance(&app->programInstance);
	}
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