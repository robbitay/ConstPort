/*
File:   appHelpers.cpp
Author: Taylor Robbins
Date:   06\06\2017
Description: 
	** Contains global functions that can be called from the rest of the application code 
*/

//#included from app.cpp

#define DEBUG_Write(formatStr) do {                 \
	if (Gl_PlatformInfo != nullptr &&               \
		Gl_PlatformInfo->DebugWritePntr != nullptr) \
	{                                               \
		Gl_PlatformInfo->DebugWritePntr(formatStr); \
	}                                               \
} while (0)

#define DEBUG_WriteLine(formatStr) do {                 \
	if (Gl_PlatformInfo != nullptr &&                   \
		Gl_PlatformInfo->DebugWriteLinePntr != nullptr) \
	{                                                   \
		Gl_PlatformInfo->DebugWriteLinePntr(formatStr); \
	}                                                   \
} while (0)

#define DEBUG_Print(formatStr, ...) do {                         \
	if (Gl_PlatformInfo != nullptr &&                            \
		Gl_PlatformInfo->DebugPrintPntr != nullptr)              \
	{                                                            \
		Gl_PlatformInfo->DebugPrintPntr(formatStr, __VA_ARGS__); \
	}                                                            \
} while (0)

#define DEBUG_PrintLine(formatStr, ...) do {                         \
	if (Gl_PlatformInfo != nullptr &&                                \
		Gl_PlatformInfo->DebugPrintLinePntr != nullptr)              \
	{                                                                \
		Gl_PlatformInfo->DebugPrintLinePntr(formatStr, __VA_ARGS__); \
	}                                                                \
} while (0)

//NOTE: Must have access to AppInput
#define ButtonPressed(button) ((AppInput->buttons[button].isDown && AppInput->buttons[button].transCount > 0) || AppInput->buttons[button].transCount >= 2)
#define ButtonReleased(button) ((!AppInput->buttons[button].isDown && AppInput->buttons[button].transCount > 0) || AppInput->buttons[button].transCount >= 2)
#define ButtonDown(button) (AppInput->buttons[button].isDown)

//NOTE: Must have access to AppInput
//This code shows up for most buttons so I pulled it out into a macro to make it look nice
#define ButtonColorChoice(buttonColor, textColor, borderColor, rectangle, isSelected, isReady) do \
{                                                                                                 \
	if (ButtonDown(MouseButton_Left) &&                                                           \
		IsInsideRectangle(AppInput->mousePos, rectangle) &&                                       \
		IsInsideRectangle(AppInput->mouseStartPos[MouseButton_Left], rectangle))                  \
	{                                                                                             \
		buttonColor = GC->colors.buttonPress;                                                     \
		textColor   = GC->colors.buttonPressText;                                                 \
		borderColor = GC->colors.buttonPressBorder;                                               \
	}                                                                                             \
	else if (isSelected)                                                                          \
	{                                                                                             \
		buttonColor = GC->colors.buttonSelected;                                                  \
		textColor   = GC->colors.buttonSelectedText;                                              \
		borderColor = GC->colors.buttonSelectedBorder;                                            \
	}                                                                                             \
	else if (IsInsideRectangle(AppInput->mousePos, rectangle))                                    \
	{                                                                                             \
		buttonColor = GC->colors.buttonHover;                                                     \
		textColor   = GC->colors.buttonHoverText;                                                 \
		borderColor = GC->colors.buttonHoverBorder;                                               \
	}                                                                                             \
	else if (isReady)                                                                             \
	{                                                                                             \
		buttonColor = GC->colors.buttonReady;                                                     \
		textColor   = GC->colors.buttonReadyText;                                                 \
		borderColor = GC->colors.buttonReadyBorder;                                               \
	}                                                                                             \
	else                                                                                          \
	{                                                                                             \
		buttonColor = GC->colors.button;                                                          \
		textColor   = GC->colors.buttonText;                                                      \
		borderColor = GC->colors.buttonBorder;                                                    \
	}                                                                                             \
} while(0)

char* FormattedSizeStr(u32 numBytes)
{
	char* result = nullptr;
	
	u32 numMegabytes = numBytes / (1024*1024);
	u32 numKilobytes = (numBytes%(1024*1024)) / 1024;
	u32 remainder    = (numBytes%1024);
	
	if (numMegabytes == 0)
	{
		if (numKilobytes == 0)
		{
			result = TempPrint("%u bytes", remainder);
		}
		else
		{
			if (remainder == 0)
			{
				result = TempPrint("%ukB", numKilobytes);
			}
			else
			{
				result = TempPrint("%ukB and %u bytes", numKilobytes, remainder);
			}
		}
	}
	else
	{
		if (numKilobytes == 0)
		{
			if (remainder == 0)
			{
				result = TempPrint("%uMB", numMegabytes);
			}
			else
			{
				result = TempPrint("%uMB and %u bytes", numMegabytes, remainder);
			}
		}
		else
		{
			if (remainder == 0)
			{
				result = TempPrint("%uMB %ukB", numMegabytes, numKilobytes);
			}
			else
			{
				result = TempPrint("%uMB %ukB and %u bytes", numMegabytes, numKilobytes, remainder);
			}
		}
	}
	
	return result;
}
