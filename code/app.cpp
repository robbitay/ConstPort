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
#include "appUiHandler.h"
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
#include "appUiHandler.cpp"

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
	
	appData->selectionStart = NewTextLocation(0, 0);
	appData->selectionEnd = NewTextLocation(0, 0);
	appData->hoverLocation = NewTextLocation(0, 0);
}

void RefreshComPortList()
{
	AppData_t* appData = GL_AppData;
	const PlatformInfo_t* PlatformInfo = Gl_PlatformInfo;
	
	appData->numComPortsAvailable = PlatformInfo->GetComPortListPntr(
		&appData->availableComPorts[0], ArrayCount(appData->availableComPorts));
	
	DEBUG_PrintLine("Found %u com ports.", appData->numComPortsAvailable);
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
		DEBUG_PrintLine("Closed %s", GetComPortName(appData->comPort.index));
	}
	
	ClearConsole();
	
	appData->comPort = PlatformInfo->OpenComPortPntr(comPortIndex, settings);
	
	if (appData->comPort.isOpen)
	{
		DEBUG_PrintLine("%s port opened successfully", GetComPortName(comPortIndex));
	}
	else
	{
		DEBUG_PrintLine("Couldn't open %s port.", GetComPortName(comPortIndex));
	}
}

void ComMenuUpdate(const PlatformInfo_t* PlatformInfo, const AppInput_t* AppInput, MenuHandler_t* menuHandler, Menu_t* menuPntr)
{
	AppData_t* appData = GL_AppData;
	
	if (menuPntr->show)
	{
		if (ButtonPressed(Button_Escape))
		{
			menuPntr->show = false;
		}
		
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
			v2 comNameSize = MeasureString(&appData->testFont, GetComPortName(ComPort_12));
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
		if (appData->comMenuOptions.isOpen &&
			IsInsideRectangle(AppInput->mousePos, connectButtonRec))
		{
			if (ButtonReleased(MouseButton_Left) && 
				IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], connectButtonRec))
			{
				OpenComPort(appData->comMenuOptions.index, appData->comMenuOptions.settings);
				menuPntr->show = false;
			}
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
		
		renderState->DrawString("Baud Rate", NewVec2(baudRateRec.x, baudRateRec.y - appData->testFont.maxExtendDown), {Color_Foreground});
		renderState->DrawRectangle(baudRateRec, Color_Foreground);
		for (i32 baudIndex = 0; baudIndex < NumBaudRates; baudIndex++)
		{
			const char* baudString = GetBaudRateString((BaudRate_t)baudIndex);
			rec currentRec = NewRectangle(baudRateRec.x, 
				baudRateRec.y + baudIndex*appData->testFont.lineHeight, 
				baudRateRec.width, appData->testFont.lineHeight
			);
			Color_t textColor = Color_Background;
			if ((BaudRate_t)baudIndex == appData->comMenuOptions.settings.baudRate)
			{
				renderState->DrawRectangle(currentRec, Color_Highlight4);
			}
			else if (IsInsideRectangle(AppInput->mousePos, currentRec))
			{
				Color_t backColor = Color_UiLightGray1;
				if (ButtonDown(MouseButton_Left) && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], currentRec))
				{
					backColor = Color_Highlight3;
					textColor = Color_Foreground;
				}
				
				renderState->DrawRectangle(currentRec, backColor);
			}
			renderState->DrawString(baudString,
				baudRateRec.topLeft + NewVec2(
					baudRateRec.width/2 - MeasureString(&appData->testFont, baudString).x/2, 
					appData->testFont.maxExtendUp + baudIndex*appData->testFont.lineHeight),
				textColor);
		}
		
		renderState->DrawString("# Bits", NewVec2(numBitsRec.x, numBitsRec.y - appData->testFont.maxExtendDown), {Color_Foreground});
		renderState->DrawRectangle(numBitsRec, Color_Foreground);
		for (i32 bitIndex = 0; bitIndex < 8; bitIndex++)
		{
			char numBitsString[4] = {};
			snprintf(numBitsString, ArrayCount(numBitsString)-1, "%d", bitIndex+1);
			rec currentRec = NewRectangle(numBitsRec.x, 
				numBitsRec.y + bitIndex*appData->testFont.lineHeight, 
				numBitsRec.width, appData->testFont.lineHeight
			);
			Color_t textColor = Color_Background;
			if ((u8)(bitIndex+1) == appData->comMenuOptions.settings.numBits)
			{
				renderState->DrawRectangle(currentRec, Color_Highlight4);
			}
			else if (IsInsideRectangle(AppInput->mousePos, currentRec))
			{
				Color_t backColor = Color_UiLightGray1;
				if (ButtonDown(MouseButton_Left) && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], currentRec))
				{
					backColor = Color_Highlight3;
					textColor = Color_Foreground;
				}
				
				renderState->DrawRectangle(currentRec, backColor);
			}
			renderState->DrawString(numBitsString,
				numBitsRec.topLeft + NewVec2(
					numBitsRec.width/2 - MeasureString(&appData->testFont, numBitsString).x/2, 
					appData->testFont.maxExtendUp + bitIndex*appData->testFont.lineHeight),
				textColor);
		}
		
		renderState->DrawString("Parity", NewVec2(parityTypesRec.x, parityTypesRec.y - appData->testFont.maxExtendDown), {Color_Foreground});
		renderState->DrawRectangle(parityTypesRec, Color_Foreground);
		for (i32 parityIndex = 0; parityIndex < NumParityTypes; parityIndex++)
		{
			const char* parityString = GetParityString((Parity_t)parityIndex);
			rec currentRec = NewRectangle(parityTypesRec.x, 
				parityTypesRec.y + parityIndex*appData->testFont.lineHeight, 
				parityTypesRec.width, appData->testFont.lineHeight
			);
			Color_t textColor = Color_Background;
			if ((Parity_t)(parityIndex) == appData->comMenuOptions.settings.parity)
			{
				renderState->DrawRectangle(currentRec, Color_Highlight4);
			}
			else if (IsInsideRectangle(AppInput->mousePos, currentRec))
			{
				Color_t backColor = Color_UiLightGray1;
				if (ButtonDown(MouseButton_Left) && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], currentRec))
				{
					backColor = Color_Highlight3;
					textColor = Color_Foreground;
				}
				
				renderState->DrawRectangle(currentRec, backColor);
			}
			renderState->DrawString(parityString,
				parityTypesRec.topLeft + NewVec2(
					parityTypesRec.width/2 - MeasureString(&appData->testFont, parityString).x/2, 
					appData->testFont.maxExtendUp + parityIndex*appData->testFont.lineHeight),
				textColor);
		}
		
		renderState->DrawString("Stop Bits", NewVec2(stopBitsRec.x, stopBitsRec.y - appData->testFont.maxExtendDown), {Color_Foreground});
		renderState->DrawRectangle(stopBitsRec, Color_Foreground);
		for (i32 stopBitIndex = 0; stopBitIndex < NumStopBitTypes; stopBitIndex++)
		{
			const char* stopBitsString = GetStopBitsString((StopBits_t)stopBitIndex);
			rec currentRec = NewRectangle(stopBitsRec.x, 
				stopBitsRec.y + stopBitIndex*appData->testFont.lineHeight, 
				stopBitsRec.width, appData->testFont.lineHeight
			);
			Color_t textColor = Color_Background;
			
			if ((StopBits_t)stopBitIndex == appData->comMenuOptions.settings.stopBits)
			{
				renderState->DrawRectangle(currentRec, Color_Highlight4);
			}
			else if (IsInsideRectangle(AppInput->mousePos, currentRec))
			{
				Color_t backColor = Color_UiLightGray1;
				if (ButtonDown(MouseButton_Left) && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], currentRec))
				{
					backColor = Color_Highlight3;
					textColor = Color_Foreground;
				}
				
				renderState->DrawRectangle(currentRec, backColor);
			}
			renderState->DrawString(stopBitsString,
				stopBitsRec.topLeft + NewVec2(
					stopBitsRec.width/2 - MeasureString(&appData->testFont, stopBitsString).x/2, 
					appData->testFont.maxExtendUp + stopBitIndex*appData->testFont.lineHeight),
				textColor);
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
				
				Color_t buttonColor = {Color_White};
				Color_t borderColor {Color_Black};
				Color_t textColor = {Color_Black};
				
				if (appData->comMenuOptions.isOpen == true && (ComPortIndex_t)comIndex == appData->comMenuOptions.index)
				{
					buttonColor = Color_Highlight4;
					borderColor = Color_Foreground;
				}
				else if (IsInsideRectangle(AppInput->mousePos, tabRec))
				{
					if (AppInput->buttons[MouseButton_Left].isDown && 
						IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], tabRec))
					{
						buttonColor = Color_Highlight3;
						borderColor = Color_Foreground;
						textColor = Color_Foreground;
					}
					else
					{
						buttonColor = Color_UiLightGray1;
					}
				}
				
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
			
			Color_t buttonColor = {Color_White};
			Color_t borderColor = Color_UiGray4;
			Color_t textColor = Color_UiGray4;
			
			if (!appData->comMenuOptions.isOpen)
			{
				buttonColor = Color_UiGray1;
			}
			else if (IsInsideRectangle(AppInput->mousePos, connectButtonRec))
			{
				if (AppInput->buttons[MouseButton_Left].isDown && 
					IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], connectButtonRec))
				{
					buttonColor = Color_Highlight3;
					borderColor = Color_Foreground;
					textColor = Color_Foreground;
				}
				else
				{
					buttonColor = Color_UiLightGray1;
				}
			}
			else if (settingsHaveChanged)
			{
				buttonColor = Color_Highlight2;
				borderColor = Color_Foreground;
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
	appData->renderState.DrawString(ui->contextStringBuffer, textPos, Color_Foreground);
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
	
	void* arenaBase = (void*)(appData+1);
	u32 arenaSize = AppMemory->permanantSize - sizeof(AppData_t);
	InitializeMemoryArenaHeap(&appData->memArena, arenaBase, arenaSize);
	
	InitializeUiElements(&appData->uiElements);
	InitializeRenderState(PlatformInfo, &appData->renderState);
	InitializeMenuHandler(&appData->menuHandler, &appData->memArena);
	
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
	
	DEBUG_WriteLine("App Reloaded");
	
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
	
	RecalculateUiElements(AppInput->mousePos, ui);
	
	Menu_t* comMenu = GetMenuByName(&appData->menuHandler, "COM Menu");
	Menu_t* contextMenu = GetMenuByName(&appData->menuHandler, "Context Menu");
	Menu_t* hoverMenu = GetMenuAtPoint(&appData->menuHandler, AppInput->mousePos);
	ui->mouseInMenu = (hoverMenu != nullptr && hoverMenu != contextMenu);
	Color_t color1 = ColorFromHSV((i32)(PlatformInfo->programTime*180) % 360, 1.0f, 1.0f);
	Color_t color2 = ColorFromHSV((i32)(PlatformInfo->programTime*180 + 125) % 360, 1.0f, 1.0f);
	Color_t selectionColor = ColorLerp({Color_White}, {Color_Gray}, (Sin32((r32)PlatformInfo->programTime*6.0f) + 1.0f) / 2.0f);
	Color_t hoverLocColor = ColorLerp(Color_Foreground, Color_Background, (Sin32((r32)PlatformInfo->programTime*8.0f) + 1.0f) / 2.0f);
	// Color_t selectionColor = ColorFromHSV(180, 1.0f, (r32)(Sin32((r32)PlatformInfo->programTime*5) + 1.0f) / 2.0f);
	
	appData->hoverLocation = PointToTextLocation(&appData->lineList, &appData->testFont, 
		ui->mousePos - NewVec2(LINE_SPACING + ui->gutterRec.width, ui->viewRec.y - ui->scrollOffset));
	
	//Context Menu Showing/Filling
	contextMenu->show = false;
	if (ui->mouseInMenu == false &&
		AppInput->buttons[Button_Control].isDown &&// && mousePos.x <= ui->gutterRec.width)
		(IsInsideRectangle(ui->mousePos, ui->viewRec) || IsInsideRectangle(ui->mousePos, ui->gutterRec)))
	{
		Line_t* linePntr = GetLineAt(&appData->lineList, appData->hoverLocation.lineNum);
		
		if (linePntr != nullptr && linePntr->timestamp != 0)
		{
			RealTime_t lineTime = RealTimeAt(linePntr->timestamp);
			
			if (AppInput->buttons[Button_Shift].isDown)
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
				u32 numDays = (u32)(absDifference/(60*60*24));
				u32 numHours = (u32)(absDifference/(60*60)) - (numDays*24);
				u32 numMinutes = (u32)(absDifference/60) - (numDays*60*24) - (numHours*60);
				u32 numSeconds = (u32)(absDifference) - (numDays*60*60*24) - (numHours*60*60) - (numMinutes*60);
				// DEBUG_PrintLine("Diff %d", secondsDifference);
				if (numDays > 0)
				{
					snprintf(ui->contextStringBuffer, sizeof(ui->contextStringBuffer)-1,
						"%ud %uh %um %us %s", numDays, numHours, numMinutes, numSeconds,
						secondsDifference >= 0 ? "Ago" : "In the future?");
				}
				else if (numHours > 0)
				{
					snprintf(ui->contextStringBuffer, sizeof(ui->contextStringBuffer)-1,
						"%uh %um %us %s", numHours, numMinutes, numSeconds,
						secondsDifference >= 0 ? "Ago" : "In the future?");
				}
				else if (numMinutes > 0)
				{
					snprintf(ui->contextStringBuffer, sizeof(ui->contextStringBuffer)-1,
						"%um %us %s", numMinutes, numSeconds,
						secondsDifference >= 0 ? "Ago" : "In the future?");
				}
				else
				{
					snprintf(ui->contextStringBuffer, sizeof(ui->contextStringBuffer)-1,
						"%us %s", numSeconds,
						secondsDifference >= 0 ? "Ago" : "In the future?");
				}
			}
			
			contextMenu->show = true;
		}
	}
	
	//Read and write COM port
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
						//TODO: Do we want to process the finished line in some way?
						
						lastLine = AddLineToList(&appData->lineList, "");
						lastLine->color = Color_Foreground;
						if (ui->followingEndOfFile)
						{
							ui->scrollOffsetGoto += 19;//appData->testFont.lineHeight;
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
			// DEBUG_PrintLine("Writing \"%.*s\"", AppInput->textInputLength, AppInput->textInput);
			
			PlatformInfo->WriteComPortPntr(&appData->comPort, &AppInput->textInput[0], AppInput->textInputLength);
		}
		
		if (AppInput->buttons[Button_Enter].transCount > 0 && AppInput->buttons[Button_Enter].isDown)
		{
			DEBUG_WriteLine("Writing New Line");
			
			char newChar = '\n';
			PlatformInfo->WriteComPortPntr(&appData->comPort, &newChar, 1);
		}
	}
	
	//Recenter COM menu
	if (PlatformInfo->windowResized)
	{
		comMenu->drawRec.topLeft = NewVec2(
			PlatformInfo->screenSize.x / 2 - comMenu->drawRec.width/2,
			PlatformInfo->screenSize.y / 2 - comMenu->drawRec.height/2);
	}
	// Show COM Menu with Ctrl+O
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
			appData->comMenuOptions = appData->comPort;
		}
	}
	
	// Clear Console and Copy to Clipboard with Ctrl(+Shift)+C
	if (AppInput->buttons[Button_C].transCount > 0 && AppInput->buttons[Button_C].isDown &&
		AppInput->buttons[Button_Control].isDown)
	{
		if (AppInput->buttons[Button_Shift].isDown)
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
	
	//Select/Deselect all with Ctrl+A
	if (AppInput->buttons[Button_A].transCount > 0 && AppInput->buttons[Button_A].isDown &&
		AppInput->buttons[Button_Control].isDown)
	{
		if (appData->selectionStart.lineNum == appData->selectionEnd.lineNum &&
			appData->selectionStart.charIndex == appData->selectionEnd.charIndex)
		{
			//Select all
			appData->selectionStart = NewTextLocation(0, 0);
			Line_t* lastLinePntr = GetLastLine(&appData->lineList);
			appData->selectionEnd = NewTextLocation(appData->lineList.numLines-1, lastLinePntr->numChars);
		}
		else
		{
			//Deselect all
			appData->selectionStart = NewTextLocation(0, 0);
			appData->selectionEnd = NewTextLocation(0, 0);
		}
	}
	
	// Quick keys for opening COM ports with Ctrl+(1-9)
	if (AppInput->buttons[Button_Control].isDown)
	{
		ComPortIndex_t cIndex = NumComPorts;
		
		if (AppInput->buttons[Button_1].transCount > 0 && AppInput->buttons[Button_1].isDown) cIndex = ComPort_1;
		if (AppInput->buttons[Button_2].transCount > 0 && AppInput->buttons[Button_2].isDown) cIndex = ComPort_2;
		if (AppInput->buttons[Button_3].transCount > 0 && AppInput->buttons[Button_3].isDown) cIndex = ComPort_3;
		if (AppInput->buttons[Button_4].transCount > 0 && AppInput->buttons[Button_4].isDown) cIndex = ComPort_4;
		if (AppInput->buttons[Button_5].transCount > 0 && AppInput->buttons[Button_5].isDown) cIndex = ComPort_5;
		if (AppInput->buttons[Button_6].transCount > 0 && AppInput->buttons[Button_6].isDown) cIndex = ComPort_6;
		if (AppInput->buttons[Button_7].transCount > 0 && AppInput->buttons[Button_7].isDown) cIndex = ComPort_7;
		if (AppInput->buttons[Button_8].transCount > 0 && AppInput->buttons[Button_8].isDown) cIndex = ComPort_8;
		if (AppInput->buttons[Button_9].transCount > 0 && AppInput->buttons[Button_9].isDown) cIndex = ComPort_9;
		
		if (cIndex != NumComPorts)
		{
			if (appData->availableComPorts[cIndex] == false)
			{
				DEBUG_PrintLine("%s not Available!", GetComPortName(cIndex));
			}
			else
			{
				OpenComPort(cIndex, appData->comPort.settings);
			}
		}
	}
	
	RecalculateUiElements(AppInput->mousePos, ui);
	
	if (AppInput->buttons[Button_Down].isDown)
	{
		ui->scrollOffsetGoto += AppInput->buttons[Button_Shift].isDown ? 16 : 5;
	}
	if (AppInput->buttons[Button_Up].isDown)
	{
		ui->scrollOffsetGoto -= AppInput->buttons[Button_Shift].isDown ? 16 : 5;
	}
	if (AppInput->scrollDelta.y != 0)
	{
		ui->scrollOffsetGoto -= AppInput->scrollDelta.y * SCROLL_MULTIPLIER;
	}
	if (AppInput->buttons[Button_End].isDown && AppInput->buttons[Button_End].transCount > 0)
	{
		ui->scrollOffsetGoto = ui->maxScrollOffset;
	}
	if (AppInput->buttons[Button_Home].isDown && AppInput->buttons[Button_Home].transCount > 0)
	{
		ui->scrollOffsetGoto = 0;
	}
	if (AppInput->buttons[Button_PageUp].isDown && AppInput->buttons[Button_PageUp].transCount > 0)
	{
		ui->scrollOffsetGoto -= ui->viewRec.height;
	}
	if (AppInput->buttons[Button_PageDown].isDown && AppInput->buttons[Button_PageDown].transCount > 0)
	{
		ui->scrollOffsetGoto += ui->viewRec.height;
	}
	
	//Main Menu Buttons
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
						DEBUG_WriteLine("Button does nothing.");
					};
				};
			}
		}
	}
	
	//Scrollbar Interaction and Text Selection
	if (AppInput->buttons[MouseButton_Left].isDown && !ui->mouseInMenu)
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
						ui->scrollOffset += ui->viewRec.height;
					}
					else
					{
						ui->scrollOffset -= ui->viewRec.height;
					}
					ui->scrollOffsetGoto = ui->scrollOffset;
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
				
				ui->scrollOffset = (newPixelLocation / (ui->scrollBarGutterRec.height - ui->scrollBarRec.height)) * ui->maxScrollOffset;
				ui->scrollOffsetGoto = ui->scrollOffset;
			}
		}
		else if (IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], ui->viewRec))
		{
			if (AppInput->buttons[MouseButton_Left].transCount > 0)//Pressed the button down
			{
				appData->selectionStart = appData->hoverLocation;
				appData->selectionEnd = appData->hoverLocation;
			}
			else //if (IsInsideRectangle(ui->mousePos, ui->viewRec)) //Mouse Button Holding
			{
				appData->selectionEnd = appData->hoverLocation;
			}
		}
	}
	
	//Mark lines using the mouse
	ui->markIndex = -1;
	if (//AppInput->mouseMaxDist[MouseButton_Left] < MOUSE_CLICK_TOLERANCE &&
		ui->mouseInMenu == false &&
		IsInsideRectangle(ui->mousePos, ui->gutterRec) && IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], ui->gutterRec))
	{
		ui->markIndex = (u32)((r32)(ui->mousePos.y - ui->viewRec.y + ui->scrollOffset) / ui->lineHeight - 0.5f);
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
	
	// Update Menus
	MenuHandlerUpdate(PlatformInfo, AppInput, &appData->menuHandler);
	
	UpdateUiElements(AppInput, ui);
	RecalculateUiElements(AppInput->mousePos, ui);
	appData->hoverLocation = PointToTextLocation(&appData->lineList, &appData->testFont, 
		ui->mousePos - NewVec2(LINE_SPACING + ui->gutterRec.width, ui->viewRec.y - ui->scrollOffset));
	
	if (IsInsideRectangle(ui->mousePos, ui->viewRec) && !ui->mouseInMenu)
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
	
	rs->DrawGradient(ui->gutterRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	// rs->DrawGradient(NewRectangle(0, 0, 300, 300), color1, color2, Direction2D_Right);
	
	//+--------------------------------------+
	//|            Render Lines              |
	//+--------------------------------------+
	{
		i32 firstLine = max(0, (i32)((r32)ui->scrollOffset / ui->lineHeight));
		i32 lastLine = min(appData->lineList.numLines, (i32)((r32)(ui->scrollOffset + ui->viewRec.height) / ui->lineHeight));
		
		rs->SetViewMatrix(Matrix4Translate(NewVec3(0, ui->viewRec.y - ui->scrollOffset, 0)));
		{//Items drawn relative to view
			
			v2 currentPos = NewVec2(ui->gutterRec.width + LINE_SPACING, firstLine * ui->lineHeight + appData->testFont.maxExtendUp);
			for (i32 lineIndex = firstLine; lineIndex < appData->lineList.numLines && lineIndex <= lastLine; lineIndex++)
			{
				Line_t* linePntr = GetLineAt(&appData->lineList, lineIndex);
				v2 lineSize = MeasureString(&appData->testFont, linePntr->chars);
				rec backRec = NewRectangle(currentPos.x, currentPos.y - appData->testFont.maxExtendUp, ui->viewRec.width, appData->testFont.lineHeight);
				// backRec = RectangleInflate(backRec, 1);
				if (lineIndex == appData->hoverLocation.lineNum && IsInsideRectangle(ui->mousePos, ui->viewRec) && !ui->mouseInMenu)
				{
					rs->DrawRectangle(backRec, Color_UiGray3);
				}
				
				rs->PrintString(NewVec2(0, currentPos.y), {Color_White}, 1.0f, "%u", lineIndex+1);
				
				DrawLine(appData, linePntr, currentPos);
				
				if (lineIndex == appData->hoverLocation.lineNum && IsInsideRectangle(ui->mousePos, ui->viewRec) && !ui->mouseInMenu)
				{
					v2 skipSize = MeasureString(&appData->testFont, linePntr->chars, appData->hoverLocation.charIndex);
					rec cursorRec = backRec;
					cursorRec.x += skipSize.x;
					cursorRec.width = 1;
					rs->DrawRectangle(cursorRec, hoverLocColor);
				}
				
				if (IsFlagSet(linePntr->flags, LineFlag_MarkBelow) ||
					(ButtonDown(MouseButton_Left) && ui->markIndex != -1 && ui->markIndex == lineIndex))
				{
					rec markRec = NewRectangle(
						ui->gutterRec.x, 
						currentPos.y + appData->testFont.maxExtendDown, 
						ui->gutterRec.width + ui->viewRec.width,
						MARK_SIZE
					);
					if (IsFlagSet(linePntr->flags, LineFlag_ThickMark) ||
						(ui->markIndex == lineIndex && ButtonDown(Button_Shift)))
					{
						markRec.y -= 1;
						markRec.height = THICK_MARK_SIZE;
					}
					rs->DrawRectangle(markRec, (ui->markIndex == lineIndex) ? Color_Highlight2 : Color_MarkColor);
				}
				
				currentPos.y += ui->lineHeight;
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
		r32 lineHeight = appData->testFont.lineHeight + LINE_SPACING;
		i32 firstLine = max(0, max(minLocation.lineNum, (i32)((r32)ui->scrollOffset / lineHeight)));
		i32 lastLine = min(appData->lineList.numLines, min(maxLocation.lineNum, (i32)((r32)(ui->scrollOffset + ui->viewRec.height) / lineHeight)));
		
		rs->SetViewMatrix(Matrix4Translate(NewVec3(0, ui->viewRec.y - ui->scrollOffset, 0)));
		{//Items drawn relative to view
			
			v2 currentPos = NewVec2(ui->gutterRec.width + LINE_SPACING, firstLine * lineHeight + appData->testFont.maxExtendUp);
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
				rs->DrawRectangle(backRec, color1);//selectionColor);
				
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
	rs->DrawTexturedRec(NewRectangle(0, ui->screenSize.y, (r32)appData->frameTexture.width, (r32)-appData->frameTexture.height), {Color_White});
	
	
	rs->BindShader(&appData->simpleShader);
	rs->UpdateShader();
	//+--------------------------------------+
	//|         Render UI Elements           |
	//+--------------------------------------+
	rs->DrawGradient(ui->statusBarRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	// rs->DrawGradient(NewRectangle(10, 10, 300, 300), color1, color2, Direction2D_Right);
	// rs->PrintString( 
	// 	NewVec2(0, ui->screenSize.y-appData->testFont.maxExtendDown), Color_Foreground, 1.0f, 
	// 	"Heap: %u/%u used", appData->memArena.used, appData->memArena.size);
	rs->PrintString( 
		NewVec2(0, ui->screenSize.y-appData->testFont.maxExtendDown), Color_Foreground, 1.0f, 
		"Line %d Char %d", appData->hoverLocation.lineNum+1, appData->hoverLocation.charIndex);
	// rs->PrintString( 
	// 	NewVec2(0, ui->screenSize.y-appData->testFont.maxExtendDown), Color_Foreground, 1.0f, 
	// 	"%s %u:%02u%s (%s %s, %u) [%u]",
	// 	GetDayOfWeekStr(GetDayOfWeek(PlatformInfo->localTime)), 
	// 	Convert24HourTo12Hour(PlatformInfo->localTime.hour), PlatformInfo->localTime.minute,
	// 	IsPostMeridian(PlatformInfo->localTime.hour) ? "pm" : "am",
	// 	GetMonthStr((Month_t)PlatformInfo->localTime.month), GetDayOfMonthString(PlatformInfo->localTime.day), PlatformInfo->localTime.year,
	// 	GetTimestamp(PlatformInfo->localTime));
	// rs->PrintString( 
	// 	NewVec2(0, ui->screenSize.y-appData->testFont.maxExtendDown), Color_Foreground, 1.0f, 
	// 	"First: %d Last: %d", firstLine, lastLine);
	// PrintString(appData, appData->testFont, 
	// 	NewVec2(0, ui->screenSize.y-appData->testFont.maxExtendDown), Color_Foreground, 1.0f, 
	// 	"%u Lines Offset: %f (%fpx long)", appData->lineList.numLines, ui->scrollOffset, ui->fileHeight);
	
	Line_t* lastLine = GetLastLine(&appData->lineList);
	// DEBUG_PrintLine("Last Item: %p", lastLine->header.lastItem);
	if (lastLine->timestamp == 0 && appData->lineList.numLines > 1)
	{
		lastLine = GetLineAt(&appData->lineList, appData->lineList.numLines-1 - 1);
	}
	
	if (appData->selectionStart.lineNum != appData->selectionEnd.lineNum ||
		appData->selectionStart.charIndex != appData->selectionEnd.charIndex)
	{
		rs->PrintString( 
			NewVec2(170, ui->screenSize.y-appData->testFont.maxExtendDown), Color_Foreground, 1.0f, 
			"Selection: %u chars", GetSelection());
	}
	//Print elapsed time since last receive
	else if (lastLine != nullptr && lastLine->timestamp != 0)
	{
		RealTime_t lineTime = RealTimeAt(lastLine->timestamp);
		i64 secondsDifference = SubtractTimes(PlatformInfo->localTime, lineTime, TimeUnit_Seconds);
		i64 absDifference = Abs64i(secondsDifference);
		if (absDifference >= MIN_SECONDS_STATUS_BAR)
		{
			u32 numDays = (u32)(absDifference/(60*60*24));
			u32 numHours = (u32)(absDifference/(60*60)) - (numDays*24);
			u32 numMinutes = (u32)(absDifference/60) - (numDays*60*24) - (numHours*60);
			u32 numSeconds = (u32)(absDifference) - (numDays*60*60*24) - (numHours*60*60) - (numMinutes*60);
			// DEBUG_PrintLine("Diff %d", secondsDifference);
			if (numDays > 0)
			{
				rs->PrintString( 
					NewVec2(170, ui->screenSize.y-appData->testFont.maxExtendDown), Color_Foreground, 1.0f, 
					"%ud %uh %um %us %s", numDays, numHours, numMinutes, numSeconds,
					secondsDifference >= 0 ? "Ago" : "In the future?");
			}
			else if (numHours > 0)
			{
				rs->PrintString( 
					NewVec2(170, ui->screenSize.y-appData->testFont.maxExtendDown), Color_Foreground, 1.0f, 
					"%uh %um %us %s", numHours, numMinutes, numSeconds,
					secondsDifference >= 0 ? "Ago" : "In the future?");
			}
			else if (numMinutes > 0)
			{
				rs->PrintString( 
					NewVec2(170, ui->screenSize.y-appData->testFont.maxExtendDown), Color_Foreground, 1.0f, 
					"%um %us %s", numMinutes, numSeconds,
					secondsDifference >= 0 ? "Ago" : "In the future?");
			}
			else
			{
				rs->PrintString( 
					NewVec2(170, ui->screenSize.y-appData->testFont.maxExtendDown), Color_Foreground, 1.0f, 
					"%us %s", numSeconds,
					secondsDifference >= 0 ? "Ago" : "In the future?");
			}
		}
	}
	
	if (appData->comPort.isOpen)
	{
		v2 comNameSize = MeasureString(&appData->testFont, GetComPortName(appData->comPort.index));
		rs->DrawString(GetComPortName(appData->comPort.index),
			NewVec2(ui->screenSize.x - comNameSize.x - 10, ui->screenSize.y-appData->testFont.maxExtendDown), 
			Color_Foreground, 1.0f);
	}
	
	//Draw Scrollbar
	rs->DrawGradient(NewRectangle(ui->scrollBarGutterRec.x - 8, ui->scrollBarGutterRec.y, 8, ui->scrollBarGutterRec.height), 
		{Color_TransparentBlack}, {Color_HalfTransparentBlack}, Direction2D_Right);
	rs->DrawGradient(ui->scrollBarGutterRec, Color_Background, Color_UiGray3, Direction2D_Right);
	
	rec centerScrollBarRec = ui->scrollBarRec;
	centerScrollBarRec.y += ui->scrollBarRec.width;
	centerScrollBarRec.height -= 2 * ui->scrollBarRec.width;
	rec startCapRec = NewRectangle(ui->scrollBarRec.x, ui->scrollBarRec.y, ui->scrollBarRec.width, ui->scrollBarRec.width);
	rec endCapRec = NewRectangle(ui->scrollBarRec.x, ui->scrollBarRec.y + ui->scrollBarRec.height - ui->scrollBarRec.width, ui->scrollBarRec.width, ui->scrollBarRec.width);
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
	
	//Draw Main Menu
	rs->DrawGradient(ui->mainMenuRec, Color_UiGray1, Color_UiGray3, Direction2D_Down);
	rs->DrawRectangle(NewRectangle(0, ui->mainMenuRec.height-1, ui->mainMenuRec.width, 1), Color_UiGray4);
	
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