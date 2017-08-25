/*
File:   app.cpp
Author: Taylor Robbins
Date:   06\06\2017
Description:
	** Contains all the exported functions and #includes
	** the rest of the source code files.
*/

#include <stdarg.h>
#include "platformInterface.h"
#include "app_version.h"
#include "Colors.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "memoryArena.h"
#include "linkedList.h"
#include "easing.h"

const PlatformInfo_t* Gl_PlatformInfo = nullptr;
const AppMemory_t*    Gl_AppMemory    = nullptr;

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
	CreateLineList(&appData->lineList, &appData->memArena, "");

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
			DEBUG_PrintLine("\"%s\"Available!", GetComPortName((ComPortIndex_t)cIndex));
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
		StatusError("Closed %s", GetComPortName(appData->comPort.index));
	}

	ClearConsole();

	appData->comPort = PlatformInfo->OpenComPortPntr(comPortIndex, settings);

	if (appData->comPort.isOpen)
	{
		StatusSuccess("%s Opened Successfully", GetComPortName(comPortIndex));
	}
	else
	{
		StatusError("Couldn't open %s port.", GetComPortName(comPortIndex));
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
		v2 tabSize = NewVec2(menuPntr->usableRec.width / numTabs, COM_MENU_TAB_HEIGHT);
		rec baudRateRec = NewRectangle(
			menuPntr->usableRec.x + COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + tabSize.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			78, appData->testFont.lineHeight * NumBaudRates
		);
		rec numBitsRec = NewRectangle(
			baudRateRec.x + baudRateRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + tabSize.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			55, appData->testFont.lineHeight * 8
		);
		rec parityTypesRec = NewRectangle(
			numBitsRec.x + numBitsRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + tabSize.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			60, appData->testFont.lineHeight * NumParityTypes
		);
		rec stopBitsRec = NewRectangle(
			parityTypesRec.x + parityTypesRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + tabSize.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			80, appData->testFont.lineHeight * NumStopBitTypes
		);

		tabSize.x = menuPntr->usableRec.width / numTabs;

		//Update the menu size
		{
			v2 comNameSize = MeasureString(&appData->testFont, GetComPortName(ComPort_24));
			r32 tabMinimumWidth = comNameSize.x + COM_MENU_TAB_PADDING*2;
			v2 menuSize = NewVec2(
				stopBitsRec.x + stopBitsRec.width + COM_MENU_OUTER_PADDING - menuPntr->drawRec.x,
				baudRateRec.y + baudRateRec.height + COM_MENU_OUTER_PADDING - menuPntr->drawRec.y);

			// if (menuSize.x / (r32)numTabs < tabMinimumWidth)
			// {
			// 	menuSize.x = tabMinimumWidth * numTabs;
			// }

			menuPntr->drawRec.size = menuSize;
			UpdateMenuRecs(menuPntr);
		}

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
				rec tabRec = NewRectangle(tabIndex * tabSize.x, 0, tabSize.x, tabSize.y);
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
		v2 tabSize = NewVec2(menuPntr->usableRec.width / numTabs, COM_MENU_TAB_HEIGHT);
		rec baudRateRec = NewRectangle(
			menuPntr->usableRec.x + COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + tabSize.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			78, appData->testFont.lineHeight * NumBaudRates
		);
		rec numBitsRec = NewRectangle(
			baudRateRec.x + baudRateRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + tabSize.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			55, appData->testFont.lineHeight * 8
		);
		rec parityTypesRec = NewRectangle(
			numBitsRec.x + numBitsRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + tabSize.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			60, appData->testFont.lineHeight * NumParityTypes
		);
		rec stopBitsRec = NewRectangle(
			parityTypesRec.x + parityTypesRec.width + COM_MENU_INNER_PADDING,
			menuPntr->usableRec.y + tabSize.y + appData->testFont.lineHeight + COM_MENU_OUTER_PADDING,
			80, appData->testFont.lineHeight * NumStopBitTypes
		);
		rec connectButtonRec = NewRectangle(
			menuPntr->usableRec.x + menuPntr->usableRec.width - CONNECT_BUTTON_WIDTH - COM_MENU_OUTER_PADDING,
			menuPntr->usableRec.y + menuPntr->usableRec.height - CONNECT_BUTTON_HEIGHT - COM_MENU_OUTER_PADDING,
			CONNECT_BUTTON_WIDTH, CONNECT_BUTTON_HEIGHT
		);
		
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
		
		//Draw the tabs
		u32 tabIndex = 0;
		for (u32 comIndex = 0; comIndex < ArrayCount(appData->availableComPorts); comIndex++)
		{
			if (appData->availableComPorts[comIndex] == true ||
				(appData->comPort.isOpen && (ComPortIndex_t)comIndex == appData->comPort.index))
			{
				v2 stringSize = MeasureString(&appData->testFont, GetComPortName((ComPortIndex_t)comIndex));
				rec tabRec = NewRectangle(tabIndex * tabSize.x, 0, tabSize.x, tabSize.y);
				tabRec.topLeft += menuPntr->usableRec.topLeft;
				v2 stringPosition = tabRec.topLeft + NewVec2(tabRec.width/2 - stringSize.x/2, tabRec.height/2);
				
				Color_t buttonColor, textColor, borderColor;
				ButtonColorChoice(buttonColor, textColor, borderColor, tabRec,
					(appData->comMenuOptions.isOpen == true && (ComPortIndex_t)comIndex == appData->comMenuOptions.index), false);
				
				renderState->DrawButton(tabRec, buttonColor, borderColor);
				renderState->DrawString(GetComPortName((ComPortIndex_t)comIndex), stringPosition, textColor);
				
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
		if (charPntr[cIndex] == 0x01 || charPntr[cIndex] == 0x02 || charPntr[cIndex] == 0x03 || charPntr[cIndex] == 0x04)
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

	void* arenaBase = (void*)(appData+1);
	u32 arenaSize = AppMemory->permanantSize - sizeof(AppData_t);
	InitializeMemoryArenaHeap(&appData->memArena, arenaBase, arenaSize);

	LoadGlobalConfiguration(PlatformInfo, &appData->globalConfig, &appData->memArena);
	InitializeUiElements(&appData->uiElements);
	InitializeRenderState(PlatformInfo, &appData->renderState);
	InitializeMenuHandler(&appData->menuHandler, &appData->memArena);
	InitializeRegexList(&appData->regexList, &appData->memArena);
	LoadRegexFile(&appData->regexList, "Resources/Configuration/RegularExpressions.rgx", &appData->memArena);

	v2i screenSize = PlatformInfo->screenSize;
	Menu_t* comMenu = AddMenu(&appData->menuHandler, "COM Menu", NewRectangle((r32)screenSize.x / 2 - 50, (r32)screenSize.y / 2 - 150, 400, 300),
		ComMenuUpdate, ComMenuRender);
	comMenu->show = false;
	Menu_t* contextMenu = AddMenu(&appData->menuHandler, "Context Menu", NewRectangle(0, 0, 100, 100),
		ContextMenuUpdate, ContextMenuRender);
	contextMenu->titleBarSize = 0;
	contextMenu->show = true;

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

	appData->frameTexture = CreateTexture(nullptr, 2048, 2048);
	appData->frameBuffer = CreateFrameBuffer(&appData->frameTexture);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		DEBUG_WriteLine("FrameBuffer incomplete!");
	}

	FileInfo_t testFile = PlatformInfo->ReadEntireFilePntr("test.txt");
	CreateLineList(&appData->lineList, &appData->memArena, "");//(const char*)testFile.content);
	PlatformInfo->FreeFileMemoryPntr(&testFile);

	appData->comPort.settings.baudRate = BaudRate_115200;
	appData->comPort.settings.parity = Parity_None;
	appData->comPort.settings.stopBits = StopBits_1;
	appData->comPort.settings.numBits = 8;

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
	GC = &appData->globalConfig;

	StatusDebug("App Reloaded");

	//Make sure our callbacks still match the location of the functions in the new DLL
	Menu_t* menuPntr = GetMenuByName(&appData->menuHandler, "COM Menu");
	menuPntr->specialPntr = nullptr;
	menuPntr->updateFunctionPntr = ComMenuUpdate;
	menuPntr->renderFunctionPntr = ComMenuRender;
	menuPntr = GetMenuByName(&appData->menuHandler, "Context Menu");
	menuPntr->specialPntr = nullptr;
	menuPntr->updateFunctionPntr = ContextMenuUpdate;
	menuPntr->renderFunctionPntr = ContextMenuRender;
}

void DrawThings(const AppInput_t* AppInput)
{
	AppData_t* appData = GL_AppData;
	RenderState_t* rs = &appData->renderState;

	rs->BindTexture(&appData->testTexture);
	rs->DrawTexturedRec(NewRectangle(10, 10, 500, 500), {Color_Yellow});
	rs->DrawTexturedRec(NewRectangle(90, 70, 500, 500), {Color_Red});
	rs->DrawTexturedRec(NewRectangle(AppInput->mousePos.x - 250, AppInput->mousePos.y - 250, 500, 500), {Color_Blue});
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
	UiElements_t* ui = &appData->uiElements;
	RenderState_t* rs = &appData->renderState;

	ClearArray(AppOutput->windowTitle);
	if (appData->comPort.isOpen)
	{
		snprintf(AppOutput->windowTitle, sizeof(AppOutput->windowTitle)-1,
			"[%s] Const Port", GetComPortName(appData->comPort.index));
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

	//+==================================+
	//|     Read and write COM port      |
	//+==================================+
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
				
				Line_t* lastLine = GetLastLine(&appData->lineList);
				
				for (i32 cIndex = 0; cIndex < readResult; cIndex++)
				{
					char newChar = buffer[cIndex];
					if (newChar == '\n')
					{
						Line_t* finishedLine = lastLine;
						if (finishedLine->timestamp == 0)
						{
							finishedLine->timestamp = GetTimestamp(PlatformInfo->localTime);
						}
						
						// +========================================+
						// | Check Line Against Regular Expressions |
						// +========================================+
						const char* expression;
						expression = GetRegularExpression(&appData->regexList, GC->genericCountRegexName);
						if (expression != nullptr &&
							TestRegularExpression(expression, finishedLine->chars, finishedLine->numChars))
						{
							DEBUG_WriteLine("Counter++");
							appData->genericCounter++;
						}
						expression = GetRegularExpression(&appData->regexList, GC->markLineRegexName);
						if (expression != nullptr &&
							TestRegularExpression(expression, finishedLine->chars, finishedLine->numChars))
						{
							DEBUG_WriteLine("Auto-Mark");
							FlagSet(finishedLine->flags, LineFlag_MarkBelow | LineFlag_ThickMark);
						}
						
						expression = GetRegularExpression(&appData->regexList, GC->highlight1RegexName);
						if (expression != nullptr && TestRegularExpression(expression, finishedLine->chars, finishedLine->numChars))
						{
							DEBUG_WriteLine("Highlight1");
							finishedLine->matchColor = GC->colors.highlight1;
						}
						expression = GetRegularExpression(&appData->regexList, GC->highlight2RegexName);
						if (expression != nullptr && TestRegularExpression(expression, finishedLine->chars, finishedLine->numChars))
						{
							DEBUG_WriteLine("Highlight2");
							finishedLine->matchColor = GC->colors.highlight2;
						}
						expression = GetRegularExpression(&appData->regexList, GC->highlight3RegexName);
						if (expression != nullptr && TestRegularExpression(expression, finishedLine->chars, finishedLine->numChars))
						{
							DEBUG_WriteLine("Highlight3");
							finishedLine->matchColor = GC->colors.highlight3;
						}
						expression = GetRegularExpression(&appData->regexList, GC->highlight4RegexName);
						if (expression != nullptr && TestRegularExpression(expression, finishedLine->chars, finishedLine->numChars))
						{
							DEBUG_WriteLine("Highlight4");
							finishedLine->matchColor = GC->colors.highlight4;
						}
						expression = GetRegularExpression(&appData->regexList, GC->highlight5RegexName);
						if (expression != nullptr && TestRegularExpression(expression, finishedLine->chars, finishedLine->numChars))
						{
							DEBUG_WriteLine("Highlight5");
							finishedLine->matchColor = GC->colors.highlight5;
						}
						
						lastLine = AddLineToList(&appData->lineList, "");
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
			else if (readResult < 0)
			{
				DEBUG_PrintLine("COM port read Error!: %d", readResult);
			}
		}
		
		if (AppInput->textInputLength > 0)
		{
			// DEBUG_PrintLine("Writing \"%.*s\"", AppInput->textInputLength, AppInput->textInput);
			
			PlatformInfo->WriteComPortPntr(&appData->comPort, &AppInput->textInput[0], AppInput->textInputLength);
			
			appData->txShiftRegister |= 0x80;
		}
		
		if (!comMenu->show && ButtonPressed(Button_Enter))
		{
			DEBUG_WriteLine("Writing New Line");
			
			char newChar = '\n';
			PlatformInfo->WriteComPortPntr(&appData->comPort, &newChar, 1);
			appData->txShiftRegister |= 0x80;
		}
		
		if (!comMenu->show && ButtonPressed(Button_Backspace))
		{
			DEBUG_WriteLine("Writing Backspace");
			
			char newChar = '\b';
			PlatformInfo->WriteComPortPntr(&appData->comPort, &newChar, 1);
			appData->txShiftRegister |= 0x80;
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
			char* selectionTempBuffer = (char*)malloc(selectionSize);
			GetSelection(selectionTempBuffer);

			PlatformInfo->CopyToClipboardPntr(selectionTempBuffer, selectionSize);

			free(selectionTempBuffer);
		}
	}
	
	//+==================================+
	//|   Reload Global Configuration    |
	//+==================================+
	if (ButtonPressed(Button_R) &&
		ButtonDown(Button_Control))
	{
		DisposeGlobalConfig(&appData->globalConfig);
		LoadGlobalConfiguration(PlatformInfo, &appData->globalConfig, &appData->memArena);
		DisposeRegexFile(&appData->regexList);
		LoadRegexFile(&appData->regexList, "Resources/Configuration/RegularExpressions.rgx", &appData->memArena);
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
				StatusError("%s not Available!", GetComPortName(cIndex));
			}
			else
			{
				OpenComPort(cIndex, appData->comPort.settings);
			}
		}
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
	if (gotoEndButtonPressed ||
		ButtonPressed(Button_End))
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
				char* selectionBuffer = (char*)malloc(selectionLength);
				GetSelection(selectionBuffer);
				
				TestRegularExpression(countExpression, selectionBuffer, selectionLength);
				
				free(selectionBuffer);
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
		char* fileBuffer = (char*)malloc(selectionSize);
		GetSelection(fileBuffer);

		//NOTE: GetSelection adds a \0 on the end so need to remove it
		DEBUG_PrintLine("Saving %u bytes to %s", selectionSize-1, fileNameBuffer);
		PlatformInfo->WriteEntireFilePntr(fileNameBuffer, fileBuffer, selectionSize-1);
		DEBUG_WriteLine("Done!");

		StatusSuccess("Saved to %s", fileNameBuffer);

		free(fileBuffer);
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
	
	// DrawThings(AppInput);
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
		// 	"Heap: %u/%u used", appData->memArena.used, appData->memArena.size);
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
			v2 comNameSize = MeasureString(&appData->testFont, GetComPortName(appData->comPort.index));
			rs->DrawString(GetComPortName(appData->comPort.index),
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
	
	// DrawThings();
	
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Left], AppInput->mouseMaxDist[MouseButton_Left], {Color_Red});
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Right], AppInput->mouseMaxDist[MouseButton_Right], {Color_Blue});
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Middle], AppInput->mouseMaxDist[MouseButton_Middle], {Color_Green});
	
	// rs->DrawRectangle(ui->statusBarRec, {Color_Yellow});
	// rs->DrawRectangle(ui->scrollBarGutterRec, {Color_Red});
	// rs->DrawRectangle(ui->scrollBarRec, {Color_Blue});
	// rs->DrawRectangle(NewRectangle(0, 0, ui->gutterRec.width, screenSize.y - ui->statusBarRec.height), {Color_Orange});
	// rs->DrawRectangle(ui->statusBarRec, {Color_Yellow});


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

	DEBUG_WriteLine("Application closing!");


}