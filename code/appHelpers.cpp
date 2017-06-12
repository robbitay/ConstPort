/*
File:   appHelpers.cpp
Author: Taylor Robbins
Date:   06\06\2017
Description: 
	** Contains global functions that can be called from the rest of the application code 
*/

//#included from app.cpp

#define DEBUG_Write(formatStr) do {                        \
	if (Gl_PlatformInfo != nullptr &&                   \
		Gl_PlatformInfo->DebugWritePntr != nullptr)     \
	{                                                      \
		Gl_PlatformInfo->DebugWritePntr(formatStr);     \
	}                                                      \
} while (0)

#define DEBUG_WriteLine(formatStr) do {                    \
	if (Gl_PlatformInfo != nullptr &&                   \
		Gl_PlatformInfo->DebugWriteLinePntr != nullptr) \
	{                                                      \
		Gl_PlatformInfo->DebugWriteLinePntr(formatStr); \
	}                                                      \
} while (0)

#define DEBUG_Print(formatStr, ...) do {                            \
	if (Gl_PlatformInfo != nullptr &&                            \
		Gl_PlatformInfo->DebugPrintPntr != nullptr)              \
	{                                                               \
		Gl_PlatformInfo->DebugPrintPntr(formatStr, __VA_ARGS__); \
	}                                                               \
} while (0)

#define DEBUG_PrintLine(formatStr, ...) do {                            \
	if (Gl_PlatformInfo != nullptr &&                                \
		Gl_PlatformInfo->DebugPrintLinePntr != nullptr)              \
	{                                                                   \
		Gl_PlatformInfo->DebugPrintLinePntr(formatStr, __VA_ARGS__); \
	}                                                                   \
} while (0)