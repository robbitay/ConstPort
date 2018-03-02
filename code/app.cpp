/*
File:   app.cpp
Author: Taylor Robbins
Date:   06\06\2017
Description:
	** Contains all the exported functions and #includes
	** the rest of the source code files.
*/

#define USE_ASSERT_FAILURE_FUNCTION true

#include "platformInterface.h"
#include "appVersion.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "my_tempMemory.h"
#include "my_tempMemory.cpp"
#include "my_linkedList.h"

// +--------------------------------------------------------------+
// |                  Platform Global Variables                   |
// +--------------------------------------------------------------+
const PlatformInfo_t* platform = nullptr;
const AppInput_t* input = nullptr;
AppOutput_t* output = nullptr;

#include "appHelpers.cpp"

// +--------------------------------------------------------------+
// |                     Application Includes                     |
// +--------------------------------------------------------------+
#include "appDefines.h"
#include "appDynamicColor.h"
#include "appStructs.h"
#include "appLineList.h"
#include "appRenderState.h"
#include "appTextBox.h"
#include "appCombobox.h"
#include "appCheckbox.h"
#include "appMenuHandler.h"
#include "appUiHandler.h"
#include "appRegularExpressions.h"
#include "appConfiguration.h"
#include "appComMenu.h"
#include "appData.h"

// +--------------------------------------------------------------+
// |                 Application Global Variables                 |
// +--------------------------------------------------------------+
AppData_t* app = nullptr;
GlobalConfig_t* GC = nullptr;
v2 RenderScreenSize = {};
v2 RenderMousePos = {};
v2 RenderMouseStartPos = {};
RenderState_t* renderState = nullptr;

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

void ChangeActiveElement(const void* elementPntr);
void HandleTextBoxEnter(TextBox_t* textBox);
char* Sanatize(MemoryArena_t* arenaPntr, const char* strPntr, u32 numChars, u16 sanatizationMode, u32* numCharsOut = nullptr);
bool IsCharLineBreakBefore(char newChar);
bool IsCharLineBreakAfter(char newChar);

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
#include "appTextBox.cpp"
#include "appCombobox.cpp"
#include "appCheckbox.cpp"
#include "appUiHandler.cpp"
#include "appRegularExpressions.cpp"
#include "appComMenu.cpp"
#include "appFifo.cpp"
#include "appDataHandler.cpp"

void ContextMenuUpdate(MenuHandler_t* menuHandler, Menu_t* menu)
{
	UiElements_t* ui = &app->uiElements;
	
	if (ui->contextString != nullptr)
	{
		v2 textSize = MeasureString(&app->uiFont, ui->contextString);
		menu->drawRec.size = textSize;
		menu->drawRec = RecInflate(menu->drawRec, CONTEXT_MENU_PADDING);
		menu->drawRec.topLeft = RenderMousePos + NewVec2(0, -3 - menu->drawRec.height);
	}
}
void ContextMenuRender(MenuHandler_t* menuHandler, Menu_t* menu)
{
	UiElements_t* ui = &app->uiElements;
	
	if (ui->contextString != nullptr)
	{
		v2 textPos = menu->usableRec.topLeft + NewVec2(CONTEXT_MENU_PADDING, CONTEXT_MENU_PADDING + app->uiFont.maxExtendUp);
		RsBindFont(&app->uiFont);
		RsDrawString(ui->contextString, textPos, GC->colors.contextMenuText);
		RsBindFont(&app->mainFont);
	}
}

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
void AboutMenuRender(MenuHandler_t* menuHandler, Menu_t* menu)
{
	const char* aboutInfoString = TempPrint(ABOUT_INFO_FORMAT_STRING,
		APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_BUILD,
		platform->version.major, platform->version.minor, platform->version.build);
	r32 logoScale = MinR32(
		(r32)menu->usableRec.width / app->crappyLogo.width,
		(r32)menu->usableRec.height / REAL_LOGO_HEIGHT);
	rec logoRec = NewRec(
		menu->usableRec.x, menu->usableRec.y,
		app->crappyLogo.width * logoScale,
		REAL_LOGO_HEIGHT * logoScale
	);
	RsBindTexture(&app->crappyLogo);
	RsDrawTexturedRec(logoRec, {Color_White}, NewRec(0, 0, (r32)app->crappyLogo.width, REAL_LOGO_HEIGHT));
	
	
	RsBindFont(&app->uiFont);
	RsDrawFormattedString(aboutInfoString,
		NewVec2(menu->usableRec.width/2, REAL_LOGO_HEIGHT + ABOUT_INFO_TEXT_PADDING + app->uiFont.maxExtendUp) + menu->usableRec.topLeft,
		menu->usableRec.width - ABOUT_INFO_TEXT_PADDING*2,
		GC->colors.uiText, Alignment_Center, true
	);
	RsBindFont(&app->mainFont);
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

void DrawPopupOverlay()
{
	if (app->popupMessage[0] != '\0' && platform->programTime - app->popupTime < app->popupDuration)
	{
		rec overlayRec = NewRec(
			RenderScreenSize.x,
			app->uiElements.mainMenuRec.y + app->uiElements.mainMenuRec.height + 5,
			POPUP_MAX_WIDTH, 0
		);
		
		if (overlayRec.width > (RenderScreenSize.x))
		{
			overlayRec.width = (RenderScreenSize.x);
		}
		
		r32 messagePadding = POPUP_MESSAGE_PADDING + MaxR32((r32)GC->popupCornerRadius/2, (r32)GC->popupOutlineThickness);
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
		
		// RsDrawRectangle(RecInflate(overlayRec, (r32)GC->menuBorderThickness), GC->colors.windowOutline);
		// RsDrawGradient(overlayRec, GC->colors.statusBar1, GC->colors.statusBar2, Dir2_Right);
		
		r32 cornerRadius = (r32)GC->popupCornerRadius;
		r32 innerRingThickness = (r32)GC->popupOutlineThickness;
		Color_t outerBorderColor = GC->colors.popupOutlineOuter;
		Color_t innerRingColor   = GC->colors.popupOutlineCenter;
		Color_t innerBorderColor = GC->colors.popupOutlineInner;
		Color_t innerColor = GC->colors.popupBackground;
		
		rec innerRec = NewRec(
			overlayRec.x,
			overlayRec.y + cornerRadius,
			overlayRec.width,
			overlayRec.height - cornerRadius*2
		);
		rec outerRec = NewRec(
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
		
		RsDrawRectangle(RecInflateX(innerRec, 1.0f), outerBorderColor);
		RsDrawRectangle(RecInflateY(outerRec, 1.0f), outerBorderColor);
		RsDrawCircle(ulCircleCenter, ulCIrcleRadius + 1.0f, outerBorderColor);
		RsDrawCircle(blCircleCenter, blCIrcleRadius + 1.0f, outerBorderColor);
		
		RsDrawRectangle(innerRec, innerRingColor);
		RsDrawRectangle(outerRec, innerRingColor);
		RsDrawCircle(ulCircleCenter, ulCIrcleRadius, innerRingColor);
		RsDrawCircle(blCircleCenter, blCIrcleRadius, innerRingColor);
		
		innerRec = RecInflateX(innerRec, -innerRingThickness);
		outerRec = RecInflateY(outerRec, -innerRingThickness);
		ulCIrcleRadius -= innerRingThickness;
		blCIrcleRadius -= innerRingThickness;
		
		RsDrawRectangle(innerRec, innerBorderColor);
		RsDrawRectangle(outerRec, innerBorderColor);
		RsDrawCircle(ulCircleCenter, ulCIrcleRadius, innerBorderColor);
		RsDrawCircle(blCircleCenter, blCIrcleRadius, innerBorderColor);
		
		RsDrawRectangle(RecInflate(innerRec, -1.0f), innerColor);
		RsDrawRectangle(RecInflate(outerRec, -1.0f), innerColor);
		RsDrawCircle(ulCircleCenter, ulCIrcleRadius - 1.0f, innerColor);
		RsDrawCircle(blCircleCenter, blCIrcleRadius - 1.0f, innerColor);
		
		v2 textPos = overlayRec.topLeft + NewVec2(overlayRec.width/2, overlayRec.height/2 - textSize.y/2 + app->uiFont.maxExtendUp);
		
		RsBindFont(&app->uiFont);
		RsDrawFormattedString(app->popupMessage, textPos, textAreaWidth, app->popupColor, Alignment_Center, true);
		RsBindFont(&app->mainFont);
	}
}

void DrawSelectionOnFormattedLine(Line_t* linePntr, v2 position, u32 startIndex, u32 endIndex, Color_t selectionColor)
{
	Assert(linePntr != nullptr);
	Assert(startIndex >= 0 && (u32)startIndex <= linePntr->numChars);
	Assert(endIndex >= 0 && (u32)endIndex <= linePntr->numChars);
	Assert(startIndex <= endIndex);
	
	// if (startIndex == endIndex) { return; }
	
	if (linePntr->numChars == 0)
	{
		rec selectionRec = NewRec(
			position.x, position.y,
			1, app->mainFont.lineHeight
		);
		selectionRec = RecInflate(selectionRec, (r32)GC->lineSpacing/2);
		RsDrawRectangle(selectionRec, selectionColor);
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
			
			rec selectionRec = NewRec(
				position.x + skipSize,
				position.y + (app->mainFont.lineHeight*numLines),
				selectionWidth,
				app->mainFont.lineHeight
			);
			selectionRec = RecInflate(selectionRec, (r32)GC->lineSpacing/2);
			RsDrawRectangle(selectionRec, selectionColor);
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

void ChangeActiveElement(const void* elementPntr)
{
	if (app->activeElement == &app->inputBox)
	{
		//TODO: Do anything we need to when the inputBox is deselected
	}
	
	app->activeElement = elementPntr;
}

void PushInputHistory(const char* inputStr, u32 inputStrLength)
{
	Assert(inputStrLength <= INPUT_TEXT_BUFFER_SIZE);
	
	u32 lastInputLength = (u32)strlen(&app->inputHistory[0][0]);
	i32 compareLength = MinI32(inputStrLength, lastInputLength);
	if (app->numHistoryItems > 0 && strncmp(&app->inputHistory[0][0], inputStr, inputStrLength) == 0 && app->inputHistory[0][compareLength] == '\0')
	{
		DEBUG_WriteLine("Ignoring duplicate input history item");
		return;
	}
	
	//Shift all the history items up one
	for (u32 hIndex = MAX_INPUT_HISTORY-1; hIndex > 0; hIndex--)
	{
		memcpy(&app->inputHistory[hIndex], &app->inputHistory[hIndex-1], INPUT_TEXT_BUFFER_SIZE+1);
	}
	
	memcpy(&app->inputHistory[0], inputStr, inputStrLength);
	app->inputHistory[0][inputStrLength] = '\0';
	
	if (app->numHistoryItems < MAX_INPUT_HISTORY)
	{
		app->numHistoryItems++;
	}
}

char* GetInputHistory(u32 historyIndex)
{
	Assert(historyIndex < MAX_INPUT_HISTORY);
	
	return &app->inputHistory[historyIndex][0];
}

void HandleTextBoxEnter(TextBox_t* textBox)
{
	// +==============================+
	// |   Send Input Text Box Text   |
	// +==============================+
	if (textBox == &app->inputBox)
	{
		if (ButtonDown(Button_Control))
		{
			//Convert HEX input into raw data
			if (app->inputBox.numChars > 0)
			{
				TempPushMark();
				
				u32 numHexBytes = 0;
				u8* hexBytes = GetHexForAsciiString(app->inputBox.chars, app->inputBox.numChars, &numHexBytes, TempArena);
				if (hexBytes != nullptr && numHexBytes > 0)
				{
					DEBUG_Print("Writing %u HEX bytes: { ", numHexBytes);
					for (u32 hIndex = 0; hIndex < numHexBytes; hIndex++)
					{
						DEBUG_Print("%02X ", hexBytes[hIndex]);
					}
					DEBUG_WriteLine("}");
					
					ProcessInputData((const char*)hexBytes, numHexBytes, false);
					PushInputHistory(app->inputBox.chars, app->inputBox.numChars);
					app->inputHistoryIndex = 0;
					
					TextBoxClear(&app->inputBox);
				}
				else
				{
					StatusError("No hex value to send");
				}
				
				TempPopMark();
			}
		}
		else
		{
			//Send the data in the input textbox
			DEBUG_PrintLine("Writing %u byte input: \"%.*s\"", app->inputBox.numChars, app->inputBox.numChars, app->inputBox.chars);
			
			if (app->inputBox.numChars > 0)
			{
				ProcessInputData(app->inputBox.chars, app->inputBox.numChars, true);
				PushInputHistory(app->inputBox.chars, app->inputBox.numChars);
				app->inputHistoryIndex = 0;
				TextBoxClear(&app->inputBox);
			}
			ProcessInputData("\n", 1, true);
		}
	}
}

void ParseLineBreakConfigOptions()
{
	if (app->hexLineBreakBeforeChars != nullptr)
	{
		ArenaPop(&app->mainHeap, app->hexLineBreakBeforeChars);
		app->hexLineBreakBeforeChars = nullptr;
	}
	if (app->hexLineBreakAfterChars != nullptr)
	{
		ArenaPop(&app->mainHeap, app->hexLineBreakAfterChars);
		app->hexLineBreakAfterChars = nullptr;
	}
	
	app->hexLineBreakBeforeCharCount = 0;
	app->hexLineBreakAfterCharCount = 0;
	
	if (GC->hexLineBreakBeforeChars != nullptr)
	{
		app->hexLineBreakBeforeChars = BytesFromMixedHexString(&app->mainHeap, NtStr(GC->hexLineBreakBeforeChars), &app->hexLineBreakBeforeCharCount);
	}
	if (GC->hexLineBreakAfterChars != nullptr)
	{
		app->hexLineBreakAfterChars = BytesFromMixedHexString(&app->mainHeap, NtStr(GC->hexLineBreakAfterChars), &app->hexLineBreakAfterCharCount);
	}
}

bool IsCharLineBreakBefore(char newChar)
{
	if (app->hexLineBreakBeforeChars != nullptr)
	{
		for (u32 cIndex = 0; cIndex < app->hexLineBreakBeforeCharCount; cIndex++)
		{
			if (newChar == app->hexLineBreakBeforeChars[cIndex])
			{
				return true;
			}
		}
	}
	return false;
}

bool IsCharLineBreakAfter(char newChar)
{
	if (app->hexLineBreakAfterChars != nullptr)
	{
		for (u32 cIndex = 0; cIndex < app->hexLineBreakAfterCharCount; cIndex++)
		{
			if (newChar == app->hexLineBreakAfterChars[cIndex])
			{
				return true;
			}
		}
	}
	return false;
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
// void App_Initialize(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory)
EXPORT AppInitialize_DEFINITION(App_Initialize)
{
	platform = platformInfo;
	app = (AppData_t*)appMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	output = nullptr;
	renderState = &app->renderState;
	
	DEBUG_WriteLine("Initializing Game...");
	
	// +==================================+
	// |          Memory Arenas           |
	// +==================================+
	Assert(appMemory->permanantSize > INPUT_ARENA_SIZE);
	ClearPointer(app);
	
	app->inputArenaBase = (char*)(app + 1);
	app->inputArenaSize = INPUT_ARENA_SIZE;
	
	u32 mainHeapSize = appMemory->permanantSize - sizeof(AppData_t) - INPUT_ARENA_SIZE;
	InitializeMemoryArenaHeap(&app->mainHeap, app->inputArenaBase + app->inputArenaSize, mainHeapSize);
	
	InitializeMemoryArenaTemp(&app->tempArena, appMemory->transientPntr, appMemory->transientSize, TRANSIENT_MAX_NUMBER_MARKS);
	
	DEBUG_PrintLine("Input Arena: %u bytes", INPUT_ARENA_SIZE);
	DEBUG_PrintLine("Main Heap:   %u bytes", mainHeapSize);
	DEBUG_PrintLine("Temp Arena:  %u bytes", appMemory->transientSize);
	
	TempPushMark();
	
	// +================================+
	// |    External Initializations    |
	// +================================+
	LoadGlobalConfiguration(platform, &app->globalConfig, &app->mainHeap);
	ParseLineBreakConfigOptions();
	InitializeUiElements(&app->uiElements);
	InitializeRenderState();
	InitializeMenuHandler(&app->menuHandler, &app->mainHeap);
	InitializeRegexList(&app->regexList, &app->mainHeap);
	LoadRegexFile(&app->regexList, "Resources/Configuration/RegularExpressions.rgx", &app->mainHeap);
	// void InitializeLineList(LineList_t* lineList, char* charStorageBase, u32 charStorageSize)
	InitializeLineList(&app->lineList, app->inputArenaBase, app->inputArenaSize);
	
	DEBUG_WriteLine("Creating menus");
	
	Menu_t* contextMenu = AddMenu(&app->menuHandler, "Context Menu", NewRec(0, 0, 100, 100),
		ContextMenuUpdate, ContextMenuRender);
	contextMenu->titleBarSize = 0;
	contextMenu->show = false;
	Menu_t* aboutMenu = AddMenu(&app->menuHandler, "About Menu", NewRec(0, 0, 400, 400),
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
	app->refreshSprite = LoadTexture("Resources/Sprites/refresh.png");
	app->arrowSprite = LoadTexture("Resources/Sprites/arrow.png");
	
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
	ComMenuInitialize(&app->comMenu);
	ComMenuShow(&app->comMenu);
	
	InitializeCheckbox(&app->lineWrapCheckbox, NewRec(100, 10, 10, 10), &app->mainHeap, "Line Wrap", NewColor(Color_White));
	CheckboxSet(&app->lineWrapCheckbox, GC->lineWrapEnabled);
	InitializeCheckbox(&app->hexModeCheckbox, NewRec(100, 10, 10, 10), &app->mainHeap, "Hex Mode", NewColor(Color_White));
	CheckboxSet(&app->hexModeCheckbox, false);
	InitializeCheckbox(&app->elapsedBannersCheckbox, NewRec(100, 10, 10, 10), &app->mainHeap, "Elapsed Banners", NewColor(Color_White));
	CheckboxSet(&app->elapsedBannersCheckbox, GC->elapsedBannerEnabled);
	
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
	
	app->testTextBox = NewTextBox(
		NewRec(100, 100, 300, app->uiFont.lineHeight + 10), 
		"Hello", 5, 256, &app->mainHeap, &app->uiFont);
	
	app->inputBox = NewTextBox(NewRec(0, 0, 0, 0), //NOTE: This will get set in  the update loop
		"", 0, INPUT_TEXT_BUFFER_SIZE, &app->mainHeap, &app->mainFont);
	
	if (GC->showInputTextBox)
	{
		app->activeElement = &app->inputBox;
	}
	else
	{
		app->activeElement = &app->uiElements.viewRec;
	}
	
	// +==============================+
	// |         Testing Area         |
	// +==============================+
	#if 0
	char* testStr = StrReplaceChar(TempArena, NtStr("Hello World!"), 'l', '1');
	DEBUG_PrintLine("testStr = \"%s\"", testStr);
	
	char* replaceStr = ArenaString(TempArena, NtStr("Hello World! This is a great World!"));
	StrReplaceInPlace(NtStr(replaceStr), "World", "12345", 5);
	DEBUG_PrintLine("replaceStr = \"%s\"", replaceStr);
	
	char* spliceStr = StrSplice(TempArena, NtStr("Hello World!"), 5, 0, NtStr(" Hi"));
	DEBUG_PrintLine("spliceStr = \"%s\"", spliceStr);
	
	spliceStr = ArenaString(TempArena, NtStr("Hello World!"));
	StrSpliceInPlace(NtStr(spliceStr), 0, 5, "12345");
	DEBUG_PrintLine("spliceStr = \"%s\"", spliceStr);
	#endif
	
	TempPopMark();
	DEBUG_WriteLine("Initialization Done!");
	app->appInitTempHighWaterMark = ArenaGetHighWaterMark(TempArena);
	ArenaResetHighWaterMark(TempArena);
}

//+================================================================+
//|                         App Update                             |
//+================================================================+
// void App_Update(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory, const AppInput_t* appInput, AppOutput_t* appOutput)
EXPORT AppUpdate_DEFINITION(App_Update)
{
	platform = platformInfo;
	input = appInput;
	appOutput = appOutput;
	app = (AppData_t*)appMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	RenderMousePos = NewVec2(input->mousePos.x, input->mousePos.y) / (r32)GUI_SCALE;
	RenderMouseStartPos = NewVec2(input->mouseStartPos[MouseButton_Left].x, input->mouseStartPos[MouseButton_Left].y) / (r32)GUI_SCALE;
	appOutput->cursorType = Cursor_Default;
	renderState = &app->renderState;
	
	UiElements_t* ui = &app->uiElements;
	Menu_t* contextMenu = GetMenuByName(&app->menuHandler, "Context Menu");
	Menu_t* aboutMenu = GetMenuByName(&app->menuHandler, "About Menu");
	Menu_t* hoverMenu = GetMenuAtPoint(&app->menuHandler, RenderMousePos);
	Color_t color1 = ColorFromHSV((i32)(platform->programTime*180) % 360, 1.0f, 1.0f);
	Color_t color2 = ColorFromHSV((i32)(platform->programTime*180 + 125) % 360, 1.0f, 1.0f);
	Color_t selectionColor = ColorLerp(GC->colors.selection1, GC->colors.selection2, (SinR32((platform->programTime/1000.0f)*6.0f) + 1.0f) / 2.0f);
	
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
	// |      Start Test Thread       |
	// +==============================+
	if (ButtonPressed(Button_F2) && app->testThread.started == false)
	{
		app->testThread = platform->StartThread(TestThreadFunction, app, nullptr);
		if (app->testThread.started)
		{
			DEBUG_WriteLine("Thread Started!");
		}
	}
	
	// +==============================+
	// |     Check on Test Thread     |
	// +==============================+
	if (app->testThread.started)
	{
		AppThreadStatus_t threadStatus = platform->GetThreadStatus(&app->testThread);
		if (threadStatus == AppThreadStatus_Finished)
		{
			DEBUG_WriteLine("Thread Finished!");
			platform->CloseThread(&app->testThread, true);
		}
		else if (threadStatus == AppThreadStatus_Error)
		{
			DEBUG_WriteLine("Thread Failed!");
			platform->CloseThread(&app->testThread, true);
		}
	}
	
	// +==============================+
	// |     Set the Window Title     |
	// +==============================+
	ClearArray(appOutput->windowTitle);
	if (app->comPort.isOpen)
	{
		snprintf(appOutput->windowTitle, sizeof(appOutput->windowTitle)-1,
			"%s - Const Port", GetPortUserName(app->comPort.name));
	}
	else
	{
		snprintf(appOutput->windowTitle, sizeof(appOutput->windowTitle)-1,
			"Const Port [Disconnected]");
	}
	
	RecalculateUiElements(ui, true);
	TextBoxRellocate(&app->inputBox, ui->textInputRec);
	ui->mouseInMenu = (
		(hoverMenu != nullptr && hoverMenu != contextMenu) ||
		(input->mouseInsideWindow && IsInsideRec(app->comMenu.drawRec, RenderMousePos))
	);
	
	if (ButtonDown(Button_Control) && ButtonPressed(Button_QuestionMark))
	{
		DEBUG_PrintLine("RenderScreenSize = (%f, %f)", RenderScreenSize.width, RenderScreenSize.height);
		GLint dims[4] = {0};
		glGetIntegerv(GL_VIEWPORT, dims);
		GLint frameWidth = dims[2];
		GLint frameHeight = dims[3];
		DEBUG_PrintLine("FrameBufferSize = (%d, %d)", frameWidth, frameHeight);
		DEBUG_PrintLine("Mouse Pos = (%f, %f)", RenderMousePos.x, RenderMousePos.y);
		
		if (app->comPort.isOpen)
		{
			r32 framesPerSecond = 60.0f;
			u32 numBytesPerFrame = GetNumBytesInPeriodForComSettings(&app->comPort.settings, (1000000.0f / framesPerSecond));
			DEBUG_PrintLine("%.2fus: %u bytes", (1000000.0f / framesPerSecond), numBytesPerFrame);
		}
		
		DEBUG_Print("Hex Line Break Before Chars: [%u]{ ", app->hexLineBreakBeforeCharCount);
		for (u32 cIndex = 0; cIndex < app->hexLineBreakBeforeCharCount; cIndex++)
		{
			DEBUG_Print("%02X ", app->hexLineBreakBeforeChars[cIndex]);
		}
		DEBUG_WriteLine("}");
		DEBUG_Print("Hex Line Break After Chars: [%u]{ ", app->hexLineBreakAfterCharCount);
		for (u32 cIndex = 0; cIndex < app->hexLineBreakAfterCharCount; cIndex++)
		{
			DEBUG_Print("%02X ", app->hexLineBreakAfterChars[cIndex]);
		}
		DEBUG_WriteLine("}");
		
		DEBUG_PrintLine("AutoNewLineTime = %d", GC->autoNewLineTime);
	}
	
	// +==================================+
	// | Font Scaling Using Scroll Wheel  |
	// +==================================+
	if (ButtonDown(Button_Control) && input->scrollDelta.y != 0)
	{
		ui->scrollOffsetGoto.y -= input->scrollDelta.y * (r32)GC->scrollMultiplier;
		GC->mainFontSize += (i32)(input->scrollDelta.y);
		if (GC->mainFontSize < 10) { GC->mainFontSize = 10; }
		if (GC->mainFontSize > 64) { GC->mainFontSize = 64; }
		
		LoadApplicationFonts();
	}
	
	//+================================+
	//|  Context Menu Showing/Filling  |
	//+================================+
	contextMenu->show = false;
	if (ui->mouseInMenu == false &&
		ButtonDown(Button_Control) &&// && mousePos.x <= ui->gutterRec.width)
		(IsInsideRec(ui->viewRec, RenderMousePos) || IsInsideRec(ui->gutterRec, RenderMousePos)))
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
				i64 absDifference = AbsI64(secondsDifference);
				ui->contextString = TempPrint("%s Ago", GetElapsedString((u64)absDifference));
			}
			
			contextMenu->show = true;
		}
	}
	
	// +--------------------------------------------------------------+
	// |      Process Inputs from COM port, Python, and AppInput      |
	// +--------------------------------------------------------------+
	{
		// +==============================+
		// |    Read From The COM Port    |
		// +==============================+
		if (app->comPort.isOpen)
		{
			char buffer[4096] = {};
			u32 readCount = 0;
			while (readCount < MAX_COM_READ_LOOPS)
			{
				i32 readResult = platform->ReadComPort(&app->comPort, buffer, ArrayCount(buffer));
				if (readResult > 0)
				{
					// DEBUG_PrintLine("Read %d bytes \"%.*s\"", readResult, readResult, buffer);
					ProcessComData(buffer, readResult);
				}
				else if (readResult < 0)
				{
					char* tempComName = ArenaString(TempArena, NtStr(app->comPort.name));
					platform->CloseComPort(&app->mainHeap, &app->comPort);
					// ClearConsole();
					RefreshComPortList();
					app->comMenu.comListSelectedIndex = app->availablePorts.count;
					PopupError("Disconnected from \"%s\"", tempComName);
					break;
				}
				else //No more bytes to read
				{
					break;
				}
				
				readCount++;
			}
			
			if (readCount >= MAX_COM_READ_LOOPS)
			{
				StatusError("Too much information recieved in a single frame");
			}
		}
		
		// +==============================+
		// |  Read From Program Instance  |
		// +==============================+
		if (app->programInstance.isOpen)
		{
			char readBuffer[256] = {};
			while (u32 numBytesRead = platform->ReadProgramOutput(&app->programInstance, readBuffer, ArrayCount(readBuffer)))
			{
				// DEBUG_PrintLine("Read %u bytes from program: \"%.*s\"", numBytesRead, numBytesRead, readBuffer);
				ProcessPythonData(readBuffer, numBytesRead);
			}
			
			ProgramStatus_t programStatus = platform->GetProgramStatus(&app->programInstance);
			if (programStatus != ProgramStatus_Running)
			{
				DEBUG_WriteLine("Program instance finished!");
				platform->CloseProgramInstance(&app->programInstance);
			}
		}
		
		// +==============================+
		// |  Route Dropped File Content  |
		// +==============================+
		if (app->droppedFile.content != nullptr && app->droppedFileProgress < app->droppedFile.size)
		{
			u32 numBytesToWrite = app->droppedFile.size - app->droppedFileProgress;
			r32 framesPerSecond = 60.0f;
			u32 numBytesPerFrame = GetNumBytesInPeriodForComSettings(&app->comPort.settings, (1000000.0f / framesPerSecond));
			if (numBytesToWrite > numBytesPerFrame) { numBytesToWrite = numBytesPerFrame; }
			const char* fileDataPntr = ((const char*)app->droppedFile.content) + app->droppedFileProgress;
			
			DEBUG_PrintLine("Writing %u bytes from dropped file", numBytesToWrite);
			ProcessInputData(fileDataPntr, numBytesToWrite, Sanatization_None);
			
			app->droppedFileProgress += numBytesToWrite;
		}
		if (app->droppedFile.content != nullptr && app->droppedFileProgress >= app->droppedFile.size)
		{
			PopupInfo("Finished writing dropped file");
			platform->FreeFileMemory(&app->droppedFile);
			ClearStruct(app->droppedFile);
			app->droppedFileProgress = 0;
		}
		
		// +==============================+
		// |     Route Keyboard Input     |
		// +==============================+
		if (input->textInputLength > 0 && IsActiveElement(&ui->viewRec))
		{
			// DEBUG_PrintLine("Keyboard input \"%.*s\"", input->textInputLength, input->textInput);
			ProcessInputData(&input->textInput[0], input->textInputLength, true);
		}
		
		// +===============================+
		// | Check for Send Button Pressed |
		// +===============================+
		if (IsInsideRec(ui->sendButtonRec, RenderMousePos) && input->mouseInsideWindow && !ui->mouseInMenu)
		{
			appOutput->cursorType = Cursor_Pointer;
			if (ButtonReleased(MouseButton_Left) && IsInsideRec(ui->sendButtonRec, RenderMouseStartPos))
			{
				HandleTextBoxEnter(&app->inputBox);
			}
		}
		
		// +==============================+
		// |  Route Paste from Clipboard  |
		// +==============================+
		if (IsActiveElement(&ui->viewRec) && ButtonDown(Button_Control) && ButtonPressed(Button_V))
		{
			TempPushMark();
			
			u32 clipboardDataSize = 0;
			char* clipboardData = (char*)platform->CopyFromClipboard(TempArena, &clipboardDataSize);
			
			if (clipboardData != nullptr)
			{
				StatusSuccess("Pasted %u bytes from clipboard", clipboardDataSize);
				ProcessInputData(clipboardData, clipboardDataSize, true);
			}
			
			TempPopMark();
		}
		
		// +==============================+
		// |    Drag and Dropped Files    |
		// +==============================+
		if (input->numDroppedFiles > 0)
		{
			for (u32 fIndex = 0; fIndex < input->numDroppedFiles; fIndex++)
			{
				const char* filePath = input->droppedFiles[fIndex];
				
				FileInfo_t fileInfo = platform->ReadEntireFile(filePath);
				if (fileInfo.content != nullptr)
				{
					if (app->droppedFile.content == nullptr)
					{
						PopupInfo("Writing %u bytes from \"%s\"", fileInfo.size, filePath);
						app->droppedFileProgress = 0;
						app->droppedFile = fileInfo;
					}
					else
					{
						PopupError("Already writing file to port");
					}
				}
				else
				{
					PopupError("Failed to open dropped file \"%s\"", filePath);
				}
			}
		}
		
		// +==============================+
		// | Check Auto New Line Timeout  |
		// +==============================+
		if (GC->autoNewLineTime > 0)
		{
			Line_t* lastLine = app->lineList.lastLine;
			if (lastLine != nullptr && lastLine->numChars > 0 && app->lastLineListPush > 0 && platform->programTime >= app->lastLineListPush)
			{
				u64 difference = platform->programTime - app->lastLineListPush;
				if (difference >= GC->autoNewLineTime)
				{
					DEBUG_WriteLine("Auto new line timeout hit");
					PushLineListData("\n", 1, true);
				}
			}
		}
	}
	
	// +--------------------------------------------------------------+
	// |                      Check UI Elements                       |
	// +--------------------------------------------------------------+
	// +==============================+
	// |       Toggle Line Wrap       |
	// +==============================+
	if (app->lineWrapCheckbox.checked != GC->lineWrapEnabled)
	{
		GC->lineWrapEnabled = app->lineWrapCheckbox.checked;
	}
	
	// +==============================+
	// |    Toggle Elapsed Banners    |
	// +==============================+
	if (app->elapsedBannersCheckbox.checked != GC->elapsedBannerEnabled)
	{
		GC->elapsedBannerEnabled = app->elapsedBannersCheckbox.checked;
	}
	
	// +==============================+
	// |      Recall Last Input       |
	// +==============================+
	if (GC->showInputTextBox && ButtonPressed(Button_Up))
	{
		if (app->inputHistoryIndex < app->numHistoryItems)
		{
			app->inputHistoryIndex++;
			DEBUG_PrintLine("Pull history %u", app->inputHistoryIndex);
			TextBoxSet(&app->inputBox, NtStr(&app->inputHistory[app->inputHistoryIndex-1][0]));
		}
	}
	if (GC->showInputTextBox && ButtonPressed(Button_Down))
	{
		if (app->inputHistoryIndex > 1)
		{
			app->inputHistoryIndex--;
			DEBUG_PrintLine("Pull history %u", app->inputHistoryIndex);
			TextBoxSet(&app->inputBox, NtStr(&app->inputHistory[app->inputHistoryIndex-1][0]));
		}
		else
		{
			app->inputHistoryIndex = 0;
			DEBUG_PrintLine("Pull history %u", app->inputHistoryIndex);
			TextBoxClear(&app->inputBox);
		}
	}
	
	// +==============================+
	// |       Close Shortcuts        |
	// +==============================+
	if (ButtonPressed(Button_W) && ButtonDown(Button_Control))
	{
		if (ButtonDown(Button_Shift))
		{
			appOutput->closeWindow = true;
		}
		else
		{
			if (app->comPort.isOpen)
			{
				char* tempComName = ArenaString(TempArena, NtStr(app->comPort.name));
				platform->CloseComPort(&app->mainHeap, &app->comPort);
				// ClearConsole();
				RefreshComPortList();
				app->comMenu.comListSelectedIndex = app->availablePorts.count;
				PopupError("Closed \"%s\"", tempComName);
			}
		}
	}
	
	//+================================+
	//|         Show COM Menu          |
	//+================================+
	if (ButtonPressed(Button_D) && ButtonDown(Button_Control))
	{
		ComMenuToggle(&app->comMenu);
		
	}
	if (app->comMenu.open && ButtonPressedUnhandled(Button_Escape))
	{
		ComMenuHide(&app->comMenu);
	}
	
	ComMenuUpdate(&app->comMenu);
	
	// +==============================+
	// | Copy Selection to Clipboard  |
	// +==============================+
	if ((IsActiveElement(&ui->viewRec) || (GC->showInputTextBox && IsActiveElement(&app->inputBox))) && ButtonDown(Button_Control) && ButtonPressed(Button_C) && !ButtonDown(Button_Shift))
	{
		u32 selectionSize = GetSelection(app->selectionStart, app->selectionEnd, false);
		if (selectionSize != 0)
		{
			TempPushMark();
			
			char* selectionTempBuffer = TempString(selectionSize+1);
			GetSelection(app->selectionStart, app->selectionEnd, false, selectionTempBuffer);
			
			platform->CopyToClipboard(selectionTempBuffer, selectionSize);
			StatusSuccess("Copied %u bytes to clipboard", selectionSize);
			
			TempPopMark();
		}
	}
	
	// +==============================+
	// |    Clear Console Shortcut    |
	// +==============================+
	if (ButtonDown(Button_Control) && ButtonDown(Button_Shift) && ButtonPressed(Button_C))
	{
		ClearConsole();
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
			ParseLineBreakConfigOptions();
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
	if (ButtonPressed(Button_O) && ButtonDown(Button_Control))
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
	
	RecalculateUiElements(ui, false);
	TextBoxRellocate(&app->inputBox, ui->textInputRec);
	
	// +==============================+
	// |     Arrow Key Scrolling      |
	// +==============================+
	if (GC->showInputTextBox == false)
	{
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
	}
	
	// +==============================+
	// | Mouse Scroll Wheel Handling  |
	// +==============================+
	if (!ButtonDown(Button_Control))
	{
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
	}
	
	// +==============================+
	// |   Goto End Button Pressed    |
	// +==============================+
	bool gotoEndButtonPressed = false;
	if (IsInsideRec(ui->gotoEndButtonRec, RenderMousePos) && input->mouseInsideWindow && !ui->mouseInMenu)
	{
		appOutput->cursorType = Cursor_Pointer;
		if (ButtonReleased(MouseButton_Left) && IsInsideRec(ui->gotoEndButtonRec, RenderMouseStartPos))
		{
			gotoEndButtonPressed = true;
		}
	}
	if (gotoEndButtonPressed || ButtonPressed(Button_End))
	{
		ui->followingEndOfFile = true;
	}
	
	// +==================================+
	// | Home PageUp and PageDown Hotkeys |
	// +==================================+
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
	
	// +==============================+
	// |      Create New Window       |
	// +==============================+
	if (ButtonDown(Button_Control) && ButtonDown(Button_Shift) && ButtonPressed(Button_N))
	{
		#if DEBUG
		PopupError("Cannot open new window in debug mode");
		#else
		platform->CreateNewWindow();
		#endif
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
				appOutput->cursorType = Cursor_Pointer;
				
				if (ButtonReleased(MouseButton_Left) && Vec2Length(RenderMouseStartPos - buttonCenter) <= buttonRadius)
				{
					switch (bIndex)
					{
						case Button_ComPort:
						{
							ComMenuToggle(&app->comMenu);
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
	if (IsInsideRec(ui->clearButtonRec, RenderMousePos) && input->mouseInsideWindow && !ui->mouseInMenu)
	{
		appOutput->cursorType = Cursor_Pointer;
		if (ButtonReleased(MouseButton_Left) &&
			IsInsideRec(ui->clearButtonRec, RenderMouseStartPos))
		{
			ClearConsole();
		}
	}
	
	// +======================================+
	// | Check Pause Button Pressed (Rx LED)  |
	// +======================================+
	if (IsInsideRec(ui->rxLedRec, RenderMousePos) && input->mouseInsideWindow && !ui->mouseInMenu)
	{
		appOutput->cursorType = Cursor_Pointer;
		if (ButtonReleased(MouseButton_Left) &&
			IsInsideRec(ui->rxLedRec, RenderMouseStartPos))
		{
			app->comRxPaused = !app->comRxPaused;
		}
	}
	
	//+==================================+
	//|    Save To File Button Press     |
	//+==================================+
	bool saveButtonPressed = false;
	if (IsInsideRec(ui->saveButtonRec, RenderMousePos) && input->mouseInsideWindow && !ui->mouseInMenu)
	{
		appOutput->cursorType = Cursor_Pointer;
		if (ButtonReleased(MouseButton_Left) && IsInsideRec(ui->saveButtonRec, RenderMouseStartPos))
		{
			saveButtonPressed = true;
		}
	}
	if (saveButtonPressed || (ButtonDown(Button_Control) && ButtonPressed(Button_S)))
	{
		SaveSelectionToFile(app->selectionStart, app->selectionEnd, GC->showFileAfterSaving);
	}
	
	//+==========================================+
	//| Scrollbar Interaction and Text Selection |
	//+==========================================+
	if (ButtonDown(MouseButton_Left) && !ui->mouseInMenu)
	{
		//Handle scrollbar interaction with mouse
		if (IsInsideRec(ui->scrollBarGutterRec, RenderMouseStartPos) &&
			ui->scrollBarRec.height < ui->scrollBarGutterRec.height)
		{
			if (input->buttons[MouseButton_Left].transCount > 0)//Pressed the button down
			{
				ui->mouseScrollbarOffset = RenderMousePos.y - ui->scrollBarRec.y;
				if (IsInsideRec(ui->scrollBarRec, RenderMousePos))
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
		else if (IsInsideRec(ui->viewRec, RenderMouseStartPos))
		{
			if (input->buttons[MouseButton_Left].transCount > 0)//Pressed the button down
			{
				app->selectionStart = ui->mouseTextLocation;
				app->selectionEnd = ui->mouseTextLocation;
			}
			else //if (IsInsideRec(RenderMousePos, ui->viewRec)) //Mouse Button Holding
			{
				app->selectionEnd = ui->mouseTextLocation;
			}
		}
	}
	
	//+================================+
	//|   Mark lines using the mouse   |
	//+================================+
	if (ui->mouseInMenu == false &&
		IsInsideRec(ui->gutterRec, RenderMousePos) && IsInsideRec(ui->gutterRec, RenderMouseStartPos))
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
	TextBoxRellocate(&app->inputBox, ui->textInputRec);
	if (ui->followingEndOfFile)
	{
		ui->scrollOffsetGoto.y = ui->maxScrollOffset.y;
	}
	TextBoxUpdate(&app->inputBox, IsActiveElement(&app->inputBox) && platform->windowHasFocus);
	rec lastButtonRec = ui->buttonRecs[ArrayCount(ui->buttonRecs)-1];
	rec checkboxRec = NewRec(lastButtonRec.x + lastButtonRec.width + 5, 5, 15, 15);;
	app->lineWrapCheckbox.drawRec = checkboxRec;
	checkboxRec.y += checkboxRec.height + 2;
	app->hexModeCheckbox.drawRec = checkboxRec;
	checkboxRec.y += checkboxRec.height + 2;
	app->elapsedBannersCheckbox.drawRec = checkboxRec;
	CheckboxUpdate(&app->lineWrapCheckbox);
	CheckboxUpdate(&app->hexModeCheckbox);
	CheckboxUpdate(&app->elapsedBannersCheckbox);
	
	// +==============================+
	// |      Text Box Selection      |
	// +==============================+
	if (input->mouseInsideWindow && !ui->mouseInMenu)
	{
		if (ClickedOnRec(app->inputBox.drawRec))
		{
			HandleButton(MouseButton_Left);
			ChangeActiveElement(&app->inputBox);
		}
		else if (ClickedOnRec(ui->viewRec))
		{
			HandleButton(MouseButton_Left);
			if (GC->showInputTextBox) //TODO: Make a configuration option to disable this?
			{
				ChangeActiveElement(&app->inputBox);
			}
			else
			{
				ChangeActiveElement(&ui->viewRec);
			}
		}
	}
	
	// +==============================+
	// |       Show Text Cursor       |
	// +==============================+
	if (IsInsideRec(ui->viewRec, RenderMousePos) && !ui->mouseInMenu && GC->showTextCursor)
	{
		appOutput->cursorType = Cursor_Text;
	}
	
	// +--------------------------------------------------------------+
	// |                       Rendering Setup                        |
	// +--------------------------------------------------------------+
	RsBegin(&app->simpleShader, &app->mainFont, NewRec(0, 0, (r32)platform->screenSize.x, (r32)platform->screenSize.y));
	
	RsClearColorBuffer(GC->colors.textBackground);
	RsClearDepthBuffer(1.0f);
	
	// +==============================+
	// |         Render Lines         |
	// +==============================+
	{
		r32 lineWrapWidth = ui->viewRec.width - GC->lineSpacing;
		i32 firstLine = max(0, ui->firstRenderLine);
		
		RsSetViewMatrix(Mat4Translate(NewVec3(ui->viewRec.x - ui->scrollOffset.x, ui->viewRec.y - ui->scrollOffset.y, 0)));
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
					Color_t fileCursorColor  = ColorLerp(GC->colors.fileCursor1, GC->colors.fileCursor2, (SinR32((platform->programTime/1000.0f)*8.0f) + 1.0f) / 2.0f);
					v2 skipSize = MeasureString(&app->mainFont, linePntr->chars, linePntr->numChars);
					rec cursorRec = NewRec(
						currentPos.x + skipSize.x,
						currentPos.y - app->mainFont.maxExtendUp,
						1, app->mainFont.lineHeight
					);
					RsDrawRectangle(cursorRec, fileCursorColor);
				}
				
				// +==============================+
				// |    Draw the Hover Cursor     |
				// +==============================+
				if (GC->showHoverCursor && input->mouseInsideWindow &&
					lineIndex == ui->mouseTextLocation.lineIndex &&
					IsInsideRec(ui->viewRec, RenderMousePos) &&
					!ui->mouseInMenu)
				{
					Color_t hoverCursorColor  = ColorLerp(GC->colors.hoverCursor1, GC->colors.hoverCursor2, (SinR32((platform->programTime/1000.0f)*8.0f) + 1.0f) / 2.0f);
					const char* skipStrPntr = &linePntr->chars[ui->mouseTextLocation.charIndex - ui->mouseTextLineLocation.charIndex];
					v2 skipSize = MeasureString(&app->mainFont, skipStrPntr, ui->mouseTextLineLocation.charIndex);
					rec cursorRec = NewRec(
						currentPos.x + skipSize.x,
						currentPos.y - app->mainFont.maxExtendUp + (ui->mouseTextLineLocation.lineIndex * app->mainFont.lineHeight),
						1, app->mainFont.lineHeight
					);
					RsDrawRectangle(cursorRec, hoverCursorColor);
				}
				
				currentPos.y += lineSize.y + GC->lineSpacing;
				if (currentPos.y - app->mainFont.maxExtendUp >= ui->scrollOffset.y + ui->viewRec.height)
				{
					//We've reached the bottom of the view
					break;
				}
			}
			
		}
		RsSetViewMatrix(Mat4_Identity);
	}
	
	RsBindFrameBuffer(&app->frameBuffer);
	RsClearColorBuffer(NewColor(Color_TransparentBlack));
	RsClearDepthBuffer(1.0f);
	
	// +==============================+
	// |       Render Selection       |
	// +==============================+
	if (app->selectionStart.lineIndex != app->selectionEnd.lineIndex ||
		app->selectionStart.charIndex != app->selectionEnd.charIndex)
	{
		TextLocation_t minLocation = TextLocationMin(app->selectionStart, app->selectionEnd);
		TextLocation_t maxLocation = TextLocationMax(app->selectionStart, app->selectionEnd);
		i32 firstLine = max(0, ui->firstRenderLine);
		
		RsSetViewMatrix(Mat4Translate(NewVec3(ui->viewRec.x - ui->scrollOffset.x, ui->viewRec.y - ui->scrollOffset.y, 0)));
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
						
						DrawSelectionOnFormattedLine(linePntr,
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
						
						rec backRec = NewRec(GC->lineSpacing + currentPos.x + skipSize.x, currentPos.y - app->mainFont.maxExtendUp, selectionSize.x, app->mainFont.lineHeight);//linePntr->lineHeight);
						backRec = RecInflate(backRec, (r32)GC->lineSpacing/2);
						RsDrawRectangle(backRec, selectionColor);
						
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
		RsSetViewMatrix(Mat4_Identity);
	}
	
	// +==============================+
	// |       Post Processing        |
	// +==============================+
	RsBindFrameBuffer(nullptr);
	RsBindShader(&app->outlineShader);
	RsUpdateShader();
	
	RsSetSecondaryColor(NewColor(0, 0, 0, 20)); //TODO: Add this as a configuration option
	RsBindTexture(&app->frameTexture);
	RsDrawTexturedRec(NewRec(0, RenderScreenSize.y, (r32)app->frameTexture.width, (r32)-app->frameTexture.height), {Color_White});
	
	RsBindShader(&app->simpleShader);
	RsUpdateShader();
	
	// +==============================+
	// | Render Line Gutter Elements  |
	// +==============================+
	RsDrawGradient(ui->gutterRec, GC->colors.gutter1, GC->colors.gutter2, Dir2_Right);
	{
		i32 firstLine = max(0, ui->firstRenderLine);
		
		RsSetViewMatrix(Mat4Translate(NewVec3(ui->gutterRec.x, ui->gutterRec.y - ui->scrollOffset.y, 0)));
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
		RsSetViewMatrix(Mat4_Identity);
	}
	
	// +==============================+
	// |       Render Scrollbar       |
	// +==============================+
	{
		RsDrawGradient(NewRec(ui->scrollBarGutterRec.x - 8, ui->scrollBarGutterRec.y, 8, ui->scrollBarGutterRec.height),
			{Color_TransparentBlack}, {Color_HalfTransparentBlack}, Dir2_Right);
		RsDrawGradient(ui->scrollBarGutterRec, GC->colors.scrollbarBack1, GC->colors.scrollbarBack2, Dir2_Right);
		
		rec centerScrollBarRec = ui->scrollBarRec;
		centerScrollBarRec.y += ui->scrollBarRec.width;
		centerScrollBarRec.height -= 2 * ui->scrollBarRec.width;
		rec startCapRec = NewRec(ui->scrollBarRec.x, ui->scrollBarRec.y, ui->scrollBarRec.width, ui->scrollBarRec.width);
		rec endCapRec = NewRec(ui->scrollBarRec.x, ui->scrollBarRec.y + ui->scrollBarRec.height - ui->scrollBarRec.width, ui->scrollBarRec.width, ui->scrollBarRec.width);
		endCapRec.y += endCapRec.height;
		endCapRec.height = -endCapRec.height;
		RsDrawRectangle(RecInflate(centerScrollBarRec, 1), GC->colors.scrollbarOutline);
		RsBindAlphaTexture(&app->scrollBarEndcapTexture);
		RsDrawRectangle(RecInflate(startCapRec, 1), GC->colors.scrollbarOutline);
		RsDrawRectangle(RecInflate(endCapRec, 1), GC->colors.scrollbarOutline);
		
		RsDrawGradient(startCapRec, GC->colors.scrollbar1, GC->colors.scrollbar2, Dir2_Right);
		RsDrawGradient(endCapRec, GC->colors.scrollbar1, GC->colors.scrollbar2, Dir2_Right);
		RsDisableAlphaTexture();
		RsDrawGradient(centerScrollBarRec, GC->colors.scrollbar1, GC->colors.scrollbar2, Dir2_Right);
	}
	
	// +==============================+
	// |     Redner Input TextBox     |
	// +==============================+
	TextBoxRender(&app->inputBox, IsActiveElement(&app->inputBox) && platform->windowHasFocus);
	
	// +==============================+
	// |      Render Send Button      |
	// +==============================+
	if (GC->showInputTextBox)
	{
		const char* sendButtonText = (ButtonDown(Button_Control) ? "HEX" : "Send");
		v2 textSize = MeasureString(&app->uiFont, sendButtonText);
		v2 textPos = ui->sendButtonRec.topLeft + ui->sendButtonRec.size/2.f - textSize/2.f + NewVec2(0, app->uiFont.maxExtendUp);
		
		Color_t buttonColor = GC->colors.button;
		Color_t textColor = GC->colors.buttonText;
		Color_t borderColor = GC->colors.buttonBorder;
		ButtonColorChoice(buttonColor, textColor, borderColor, ui->sendButtonRec, ButtonDown(Button_Enter), app->inputBox.numChars > 0);
		
		RsDrawButton(ui->sendButtonRec, buttonColor, borderColor, 1);
		RsBindFont(&app->uiFont);
		RsDrawString(sendButtonText, textPos, textColor);
		RsBindFont(&app->mainFont);
	}
	
	// +==============================+
	// |   Render File Progress Bar   |
	// +==============================+
	if (app->droppedFile.content != nullptr)
	{
		r32 percentComplete = (r32)app->droppedFileProgress / (r32)app->droppedFile.size;
		
		rec progressRec = app->inputBox.drawRec;
		progressRec.height = 1; progressRec.y -= 2;
		rec filledRec = progressRec;
		filledRec.width *= percentComplete;
		RsDrawRectangle(filledRec, GC->colors.successMessage);
		
		v2 textPos = progressRec.topLeft + progressRec.size;
		RsBindFont(&app->uiFont);
		RsPrintString(textPos, GC->colors.uiText, 1.0f, "%.1f%%", percentComplete*100);
		RsBindFont(&app->mainFont);
	}
	
	// +==============================+
	// |      Render Status Bar       |
	// +==============================+
	{
		RsDrawGradient(ui->statusBarRec, GC->colors.statusBar1, GC->colors.statusBar2, Dir2_Right);
		
		#if 0
		RsBindFont(&app->uiFont);
		RsPrintString(NewVec2(ui->statusBarRec.x+5, ui->statusBarRec.y + app->uiFont.maxExtendUp), GC->colors.uiText, 1.0f,
			"Mouse Line Location: (%d, %d)", ui->mouseTextLineLocation.lineIndex, ui->mouseTextLineLocation.charIndex);
		RsBindFont(&app->mainFont);
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
				RsBindFont(&app->uiFont);
				RsDrawString(comPortUserName, renderPos, GC->colors.uiText, 1.0f);
				RsBindFont(&app->mainFont);
				
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
				
				RsBindFont(&app->uiFont);
				RsDrawFormattedString(app->statusMessage, numCharacters, stringPos, availableWidth, textColor, Alignment_Left, false);
				if (numCharacters < stringLength)
				{
					v2 charPos = NewVec2(stringSize.x, 0);
					if (charPos.x < availableWidth)
					{
						RsDrawString("|", 1, stringPos + charPos, textColor, 1.0f, Alignment_Left);
					}
				}
				RsBindFont(&app->mainFont);
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
			
			RsDrawButton(ui->gotoEndButtonRec, buttonColor, borderColor);
		}
		
		// +==============================+
		// |   Render Update Indicator    |
		// +==============================+
		if (GC->showUpdateIndicator)
		{
			static u32 indicatorStep = 0;
			
			rec indicatorRec = NewRec(
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
			
			RsDrawRectangle(indicatorRec, GC->colors.updateIndicatorColor2);
			
			if (pMin <= pMax)
			{
				rec rec1 = indicatorRec;
				rec1.height = (r32)pMax-(r32)pMin;
				rec1.y += indicatorRec.height - rec1.height;
				rec1.y -= pMin;
				
				RsDrawRectangle(rec1, GC->colors.updateIndicatorColor1);
			}
			else
			{
				rec rec1 = indicatorRec;
				rec1.height = indicatorRec.height - pMin;
				
				rec rec2 = indicatorRec;
				rec2.height = (r32)pMax;
				rec2.y += indicatorRec.height - rec2.height;
				
				RsDrawRectangle(rec1, GC->colors.updateIndicatorColor1);
				RsDrawRectangle(rec2, GC->colors.updateIndicatorColor1);
			}
		}
	}
	
	// +==============================+
	// |       Render Main Menu       |
	// +==============================+
	RsDrawGradient(ui->mainMenuRec, GC->colors.mainMenu1, GC->colors.mainMenu2, Dir2_Down);
	// RsDrawRectangle(NewRec(0, ui->mainMenuRec.height-1, ui->mainMenuRec.width, 1), GC->colors.uiGray4); //TODO: Reimplement line at bottom of menu?
	
	r32 mainMenuButtonsRight = 0;
	for (u32 bIndex = 0; bIndex < NumMainMenuButtons; bIndex++)
	{
		rec buttonRec = ui->buttonRecs[bIndex];
		// Color_t baseColor = {Color_White};
		// Color_t highlightColor = {Color_Red};
		// Color_t iconColor = {Color_White};
		
		// if (IsInsideRec(RenderMousePos, buttonRec) && !ui->mouseInMenu)
		// {
		// 	if (ButtonDown(MouseButton_Left) && IsInsideRec(RenderMouseStartPos, buttonRec))
		// 	{
		// 		// iconColor = Color_Highlight2;
		// 		highlightColor = {Color_Black};//Color_Highlight4;
		// 	}
		// }
		// highlightColor.a = 200;
		
		// RsBindTexture(&ui->buttonBaseTexture);
		// RsDrawTexturedRec(buttonRec, baseColor);
		
		// if (IsInsideRec(RenderMousePos, buttonRec) && !ui->mouseInMenu)
		// if (Vec2Length(RenderMousePos - buttonCenter) < buttonRadius)
		// {
		// 	RsBindTexture(&ui->buttonHighlightTexture);
		// 	RsDrawTexturedRec(buttonRec, highlightColor);
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
		
		RsDrawCircle(buttonCenter, buttonRadius, borderColor);
		RsDrawCircle(buttonCenter, buttonRadius-1, centerColor);
		
		RsBindTexture(&ui->buttonTextures[bIndex]);
		RsDrawTexturedRec(buttonRec, iconColor);
		mainMenuButtonsRight = buttonRec.x + buttonRec.width;
	}
	
	rec mainMenuSettingsRec = NewRec(lastButtonRec.x + lastButtonRec.width, ui->mainMenuRec.y, 0, ui->mainMenuRec.height);
	mainMenuSettingsRec.width = ui->clearButtonRec.x - mainMenuSettingsRec.x;
	RsSetViewport(mainMenuSettingsRec);
	CheckboxRender(&app->lineWrapCheckbox, &app->uiFont);
	CheckboxRender(&app->hexModeCheckbox, &app->uiFont);
	CheckboxRender(&app->elapsedBannersCheckbox, &app->uiFont);
	RsSetViewport(NewRec(Vec2_Zero, NewVec2(platform->screenSize)));
	
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
		if (app->comRxPaused)
		{
			rec leftRec = ui->rxLedRec;
			leftRec.width = (leftRec.width/2);
			leftRec.width -= 1;
			rec rightRec = leftRec;
			rightRec.x += rightRec.width + 1;
			RsDrawRectangle(leftRec, centerColor);
			RsDrawRectangle(rightRec, centerColor);
		}
		else if (!GC->circularRxLed) { RsDrawRectangle(ui->rxLedRec, centerColor); }
		else                         { RsDrawCircle(rxLedCenter, rxLedRadius, centerColor); }
		
		v2 txLedCenter = ui->txLedRec.topLeft + ui->txLedRec.size/2;
		r32 txLedRadius = ui->txLedRec.width/4 * 3;
		centerColor = GC->colors.transmitLed;
		if ((app->txShiftRegister&0x80) > 0 ||
			(app->txShiftRegister&0x40) > 0)
		{
			centerColor = GC->colors.transmitLedActive;
		}
		if (!GC->circularTxLed) { RsDrawRectangle(ui->txLedRec, centerColor); }
		else                    { RsDrawCircle(txLedCenter, txLedRadius, centerColor); }
		
		for (u32 shift = 0; shift < sizeof(u8)*8; shift++)
		{
			u32 ringNumber = (7 - shift);
			
			if (IsFlagSet(app->rxShiftRegister, (1<<shift)))
			{
				if (GC->circularRxLed == false || app->comRxPaused)
				{
					rec deflatedRec = RecInflate(ui->rxLedRec, (r32)(ringNumber+1) * GC->rxTxLedRingSize);
					RsDrawButton(deflatedRec, {Color_TransparentBlack}, GC->colors.receiveLedRing, (r32)GC->rxTxLedRingSize);
				}
				else
				{
					r32 radius = rxLedRadius -0.2f + (ringNumber+1) * GC->rxTxLedRingSize;
					r32 innerRadius = rxLedRadius -0.2f + ringNumber * GC->rxTxLedRingSize - 1.8f;
					RsDrawDonut(rxLedCenter, radius, innerRadius, GC->colors.receiveLedRing);
				}
			}
			
			if (IsFlagSet(app->txShiftRegister, (1<<shift)))
			{
				if (GC->circularTxLed == false)
				{
					rec deflatedRec = RecInflate(ui->txLedRec, (r32)(ringNumber+1) * GC->rxTxLedRingSize);
					RsDrawButton(deflatedRec, {Color_TransparentBlack}, GC->colors.transmitLedRing, (r32)GC->rxTxLedRingSize);
				}
				else
				{
					r32 radius = txLedRadius -0.2f + (ringNumber+1) * GC->rxTxLedRingSize;
					r32 innerRadius = txLedRadius -0.2f + ringNumber * GC->rxTxLedRingSize - 1.8f;
					RsDrawDonut(txLedCenter, radius, innerRadius, GC->colors.transmitLedRing);
				}
			}
		}
	}
	
	// +==============================+
	// |         Clear Button         |
	// +==============================+
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
		
		RsDrawButton(ui->clearButtonRec, buttonColor, borderColor);
		RsBindFont(&app->uiFont);
		RsDrawString(clearStr, textPos, textColor);
		RsBindFont(&app->mainFont);
	}
	
	// +==============================+
	// |     Python Running Label     |
	// +==============================+
	if (app->programInstance.isOpen)
	{
		rec pythonIconRec = NewRec(
			ui->txLedRec.x - GC->rxTxLedRingSize*8 - PYTHON_ICON_SIZE - 5,
			ui->txLedRec.y,
			PYTHON_ICON_SIZE,
			PYTHON_ICON_SIZE
		);
		RsBindTexture(&app->pythonIcon);
		RsDrawTexturedRec(pythonIconRec, {Color_White});
	}
	
	// +==============================+
	// |       Generic Counter        |
	// +==============================+
	if (GetRegularExpression(&app->regexList, GC->genericCountRegexName) != nullptr)
	{
		// RsPrintString(NewVec2(mainMenuButtonsRight + 10, app->testFont.maxExtendUp + 10), GC->colors.foreground, 1.0f,
		// 				"Counter: %u", app->genericCounter);
	}
	
	// +==============================+
	// |     Save To File Button      |
	// +==============================+
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
		
		RsDrawButton(ui->saveButtonRec, buttonColor, borderColor);
		RsBindFont(&app->uiFont);
		RsDrawString(clearStr, textPos, textColor);
		RsBindFont(&app->mainFont);
	}
	
	MenuHandlerDrawMenus(&app->menuHandler);
	ComMenuDraw(&app->comMenu);
	
	DrawPopupOverlay();
	
	#if DEBUG
	#if 0
	TextBoxRender(&app->testTextBox, rs, testBoxSelected && platform->windowHasFocus);
	#endif
	
	// +==============================+
	// |      Draw Debug Overlay      |
	// +==============================+
	if (app->showDebugMenu)
	{
		rec overlayRec = NewRec(10, 10, (r32)RenderScreenSize.x - 10*2, (r32)RenderScreenSize.y - 10*2);
		
		RsDrawButton(overlayRec, ColorTransparent({Color_Black}, 0.5f), ColorTransparent({Color_White}, 0.5f));
		
		v2 textPos = NewVec2(overlayRec.x + 5, overlayRec.y + 5 + app->uiFont.maxExtendUp);
		RsBindFont(&app->uiFont);
		
		RsPrintString(textPos, {Color_White}, 1.0f, "AppData Size: %u/%u (%.3f%%)",
			sizeof(AppData_t), appMemory->permanantSize,
			(r32)sizeof(AppData_t) / (r32)appMemory->permanantSize * 100.0f);
		textPos.y += app->uiFont.lineHeight;
		
		RsPrintString(textPos, {Color_White}, 1.0f, "Input Arena: %u/%u (%.3f%%)",
			app->lineList.charDataSize, app->lineList.charDataMaxSize,
			(r32)app->lineList.charDataSize / (r32)app->lineList.charDataMaxSize * 100.0f);
		textPos.y += app->uiFont.lineHeight;
		
		RsPrintString(textPos, {Color_White}, 1.0f, "Main Heap: %u/%u (%.3f%%)",
			app->mainHeap.used, app->mainHeap.size,
			(r32)app->mainHeap.used / (r32)app->mainHeap.size * 100.0f);
		textPos.y += app->uiFont.lineHeight;
		
		RsPrintString(textPos, {Color_White}, 1.0f, "Temp Init: %u/%u (%.3f%%)",
			app->appInitTempHighWaterMark, TempArena->size,
			(r32)app->appInitTempHighWaterMark / (r32)TempArena->size * 100.0f);
		textPos.y += app->uiFont.lineHeight;
		
		RsPrintString(textPos, {Color_White}, 1.0f, "Temp Update: %u/%u (%.3f%%)",
			ArenaGetHighWaterMark(TempArena), TempArena->size,
			(r32)ArenaGetHighWaterMark(TempArena) / (r32)TempArena->size * 100.0f);
		textPos.y += app->uiFont.lineHeight;
		
		RsPrintString(textPos, {Color_White}, 1.0f, "Program Time: %lu", platform->programTime);
		textPos.y += app->uiFont.lineHeight;
		
		RsPrintString(textPos, {Color_White}, 1.0f, "Time Delta: %.2f", platform->timeDelta);
		textPos.y += app->uiFont.lineHeight;
		
		RsPrintString(textPos, {Color_White}, 1.0f, "System Timestamp: %ld", GetTimestamp(platform->systemTime));
		textPos.y += app->uiFont.lineHeight;
		
		RsPrintString(textPos, {Color_White}, 1.0f, "System Time: %s", FormattedTimeStr(platform->systemTime));
		textPos.y += app->uiFont.lineHeight;
		
		RsPrintString(textPos, {Color_White}, 1.0f, "Local Timestamp: %ld", GetTimestamp(platform->localTime));
		textPos.y += app->uiFont.lineHeight;
		
		RsPrintString(textPos, {Color_White}, 1.0f, "Local Time: %s", FormattedTimeStr(platform->localTime));
		textPos.y += app->uiFont.lineHeight;
		
		RsBindFont(&app->mainFont);
	}
	#endif
	
	// RsDrawCircle(RenderMouseStartPos, input->mouseMaxDist[MouseButton_Left]/GUI_SCALE, {Color_Red});
	// RsDrawCircle(input->mouseStartPos[MouseButton_Right]/GUI_SCALE, input->mouseMaxDist[MouseButton_Right]/GUI_SCALE, {Color_Blue});
	// RsDrawCircle(input->mouseStartPos[MouseButton_Middle]/GUI_SCALE, input->mouseMaxDist[MouseButton_Middle]/GUI_SCALE, {Color_Green});
	
	// RsDrawRectangle(ui->statusBarRec, {Color_Yellow});
	// RsDrawRectangle(ui->scrollBarGutterRec, {Color_Red});
	// RsDrawRectangle(ui->scrollBarRec, {Color_Blue});
	// RsDrawRectangle(NewRec(0, 0, ui->gutterRec.width, screenSize.y - ui->statusBarRec.height), {Color_Orange});
	// RsDrawRectangle(ui->statusBarRec, {Color_Yellow});
	
	// +==============================+
	// |  Temp Arena Update Loop Pop  |
	// +==============================+
	TempPopMark();
	if (ButtonPressed(Button_T) && ButtonDown(Button_Control)) { ArenaResetHighWaterMark(TempArena); }
}

// +--------------------------------------------------------------+
// |                        App Reloading                         |
// +--------------------------------------------------------------+
// void App_Reloading(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory)
EXPORT AppReloading_DEFINITION(App_Reloading)
{
	platform = platformInfo;
	app = (AppData_t*)appMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	RenderMousePos = NewVec2(input->mousePos.x, input->mousePos.y) / (r32)GUI_SCALE;
	RenderMouseStartPos = NewVec2(input->mouseStartPos[MouseButton_Left].x, input->mouseStartPos[MouseButton_Left].y) / (r32)GUI_SCALE;
	renderState = &app->renderState;
	
	if (app->testThread.started)
	{
		platform->CloseThread(&app->testThread, false);
	}
}

// +--------------------------------------------------------------+
// |                         App Reloaded                         |
// +--------------------------------------------------------------+
// void App_Reloaded(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory)
EXPORT AppReloaded_DEFINITION(App_Reloaded)
{
	platform = platformInfo;
	app = (AppData_t*)appMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	renderState = &app->renderState;
	
	PopupSuccess("App Reloaded");
	
	//Make sure our callbacks still match the location of the functions in the new DLL
	Menu_t* menuPntr = GetMenuByName(&app->menuHandler, "Context Menu");
	menuPntr->specialPntr = nullptr;
	menuPntr->updateFunctionPntr = (void*)ContextMenuUpdate;
	menuPntr->renderFunctionPntr = (void*)ContextMenuRender;
	menuPntr = GetMenuByName(&app->menuHandler, "About Menu");
	menuPntr->specialPntr = nullptr;
	menuPntr->updateFunctionPntr = (void*)AboutMenuUpdate;
	menuPntr->renderFunctionPntr = (void*)AboutMenuRender;
}

// +--------------------------------------------------------------+
// |                         App Closing                          |
// +--------------------------------------------------------------+
// void App_Closing(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory)
EXPORT AppClosing_DEFINITION(App_Closing)
{
	platform = platformInfo;
	app = (AppData_t*)appMemory->permanantPntr;
	GC = &app->globalConfig;
	TempArena = &app->tempArena;
	RenderScreenSize = NewVec2((r32)platform->screenSize.x / GUI_SCALE, (r32)platform->screenSize.y / GUI_SCALE);
	renderState = &app->renderState;
	
	DEBUG_WriteLine("Application closing!");
	
	if (app->programInstance.isOpen)
	{
		platform->CloseProgramInstance(&app->programInstance);
	}
	
	if (app->testThread.started)
	{
		platform->CloseThread(&app->testThread, false);
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