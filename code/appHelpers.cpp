/*
File:   appHelpers.cpp
Author: Taylor Robbins
Date:   06\06\2017
Description: 
	** Contains global functions that can be called from the rest of the application code 
*/

//#included from app.cpp

#define DEBUG_Write(formatStr) do {          \
	if (platform != nullptr &&               \
		platform->DebugWritePntr != nullptr) \
	{                                        \
		platform->DebugWritePntr(formatStr); \
	}                                        \
} while (0)

#define DEBUG_WriteLine(formatStr) do {          \
	if (platform != nullptr &&                   \
		platform->DebugWriteLinePntr != nullptr) \
	{                                            \
		platform->DebugWriteLinePntr(formatStr); \
	}                                            \
} while (0)

#define DEBUG_Print(formatStr, ...) do {                  \
	if (platform != nullptr &&                            \
		platform->DebugPrintPntr != nullptr)              \
	{                                                     \
		platform->DebugPrintPntr(formatStr, __VA_ARGS__); \
	}                                                     \
} while (0)

#define DEBUG_PrintLine(formatStr, ...) do {                  \
	if (platform != nullptr &&                                \
		platform->DebugPrintLinePntr != nullptr)              \
	{                                                         \
		platform->DebugPrintLinePntr(formatStr, __VA_ARGS__); \
	}                                                         \
} while (0)

#define HandleButton(button) do { app->buttonHandled[button] = true; } while(0)

#define ButtonPressed(button) ((input->buttons[button].isDown && input->buttons[button].transCount > 0) || input->buttons[button].transCount >= 2)
#define ButtonPressedUnhandled(button) (app->buttonHandled[button] == false && ((input->buttons[button].isDown && input->buttons[button].transCount > 0) || input->buttons[button].transCount >= 2))
#define ButtonReleased(button) ((!input->buttons[button].isDown && input->buttons[button].transCount > 0) || input->buttons[button].transCount >= 2)
#define ButtonReleasedUnhandled(button) (app->buttonHandled[button] == false && ((!input->buttons[button].isDown && input->buttons[button].transCount > 0) || input->buttons[button].transCount >= 2))
#define ButtonDown(button) (input->buttons[button].isDown)
#define ButtonDownUnhandled(button) (app->buttonHandled[button] == false && input->buttons[button].isDown)

//This code shows up for most buttons so I pulled it out into a macro to make it look nice
#define ButtonColorChoice(buttonColor, textColor, borderColor, rectangle, isSelected, isReady) do \
{                                                                                                 \
	if (ButtonDown(MouseButton_Left) &&                                                           \
		IsInsideRectangle(RenderMousePos, rectangle) &&                                           \
		IsInsideRectangle(input->mouseStartPos[MouseButton_Left]/GUI_SCALE, rectangle))           \
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
	else if (isReady)                                                                             \
	{                                                                                             \
		buttonColor = GC->colors.buttonReady;                                                     \
		textColor   = GC->colors.buttonReadyText;                                                 \
		borderColor = GC->colors.buttonReadyBorder;                                               \
	}                                                                                             \
	if (IsInsideRectangle(RenderMousePos, rectangle) && !ButtonDown(MouseButton_Left))            \
	{                                                                                             \
		buttonColor = ColorMultiply(buttonColor, GC->colors.buttonHover);                         \
		textColor   = ColorMultiply(textColor,   GC->colors.buttonHoverText);                     \
		borderColor = ColorMultiply(borderColor, GC->colors.buttonHoverBorder);                   \
	}                                                                                             \
} while(0)

#define BufferPrint(array, formatStr, ...) do                                    \
{                                                                                \
	int snprintfResult = snprintf(array, sizeof(array), formatStr, __VA_ARGS__); \
	Assert(snprintfResult >= 0 && snprintfResult < sizeof(array));               \
	array[snprintfResult] = '\0';                                                \
} while (0)

char* DupStrN(const char* str, u32 strLength, MemoryArena_t* memoryArenaPntr)
{
	char* result;
	
	result = PushArray(memoryArenaPntr, char, (u32)strLength+1);
	strncpy(result, str, strLength);
	result[strLength] = '\0';
	
	return result;
}
char* DupStr(const char* str, MemoryArena_t* memoryArenaPntr)
{
	u32 strLength = (u32)strlen(str);
	return DupStrN(str, strLength, memoryArenaPntr);
}

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


