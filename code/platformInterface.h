/*
File:   platformInterface.h
Author: Taylor Robbins
Date:   06\06\2017
Description: 
	** This is the only common file between the application and the platform layer
*/

#ifndef _PLATFORM_INTERFACE_H
#define _PLATFORM_INTERFACE_H

//NOTE: mylib.h checks for WIN32_COMPILATION, OSX_COMPILATION, and LINUX_COMPILATION defines
#include "mylib.h"

//TODO: How do we get rid of this??
#if WINDOWS_COMPILATION
#include <windows.h>

#include "win32_helpers.h"
#include "win32_com.h"
#include "win32_program.h"
#include "win32_clipboard.h"
#include "win32_threading.h"

#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#endif

#if OSX_COMPILATION
#include "osx_helpers.h"
#include "osx_com.h"
#include "osx_program.h"
#include "osx_clipboard.h"

#define EXPORT extern "C" __attribute__((visibility("default")))
#define IMPORT
#endif

#if LINUX_COMPILATION
//TODO: Work on Linux platform layer

#define EXPORT extern "C" __attribute__((visibility("default")))
#define IMPORT
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "my_boundedStrList.h"

#include "timeStructs.h"

// +--------------------------------------------------------------+
// |                   Platform Layer Functions                   |
// +--------------------------------------------------------------+
#define GetComPortList_DEFINITION(functionName) BoundedStrList_t functionName(MemoryArena_t* memArena)
typedef GetComPortList_DEFINITION(GetComPortList_f);
#define OpenComPort_DEFINITION(functionName) ComPort_t functionName(MemoryArena_t* memArena, const char* comPortName, ComSettings_t settings)
typedef OpenComPort_DEFINITION(OpenComPort_f);
#define CloseComPort_DEFINITION(functionName) void functionName(MemoryArena_t* memArena, ComPort_t* comPortPntr)
typedef CloseComPort_DEFINITION(CloseComPort_f);
#define ReadComPort_DEFINITION(functionName) i32 functionName(ComPort_t* comPortPntr, void* outputBuffer, u32 outputBufferLength)
typedef ReadComPort_DEFINITION(ReadComPort_f);
#define WriteComPort_DEFINITION(functionName) u32 functionName(ComPort_t* comPortPntr, const char* newChars, u32 numChars)
typedef WriteComPort_DEFINITION(WriteComPort_f);

#define StartProgramInstance_DEFINITION(functionName) ProgramInstance_t functionName(const char* commandStr)
typedef StartProgramInstance_DEFINITION(StartProgramInstance_f);
#define GetProgramStatus_DEFINITION(functionName) ProgramStatus_t functionName(const ProgramInstance_t* program)
typedef GetProgramStatus_DEFINITION(GetProgramStatus_f);
#define ReadProgramOutput_DEFINITION(functionName) u32 functionName(const ProgramInstance_t* program, char* outputBuffer, u32 outputBufferSize)
typedef ReadProgramOutput_DEFINITION(ReadProgramOutput_f);
#define WriteProgramInput_DEFINITION(functionName) u32 functionName(const ProgramInstance_t* program, const char* dataPntr, u32 numBytes)
typedef WriteProgramInput_DEFINITION(WriteProgramInput_f);
#define CloseProgramInstance_DEFINITION(functionName) void functionName(ProgramInstance_t* program)
typedef CloseProgramInstance_DEFINITION(CloseProgramInstance_f);

#define FreeFileMemory_DEFINITION(functionName)  void functionName(FileInfo_t* fileInfo)
typedef FreeFileMemory_DEFINITION(FreeFileMemory_f);
#define ReadEntireFile_DEFINITION(functionName)  FileInfo_t functionName(const char* filename)
typedef ReadEntireFile_DEFINITION(ReadEntireFile_f);
#define WriteEntireFile_DEFINITION(functionName) bool32 functionName(const char* filename, void* memory, uint32 memorySize)
typedef WriteEntireFile_DEFINITION(WriteEntireFile_f);
#define OpenFile_DEFINITION(functionName) bool32 functionName(const char* fileName, OpenFile_t* openFileOut)
typedef OpenFile_DEFINITION(OpenFile_f);
#define AppendFile_DEFINITION(functionName) bool32 functionName(OpenFile_t* filePntr, const void* newData, u32 newDataSize)
typedef AppendFile_DEFINITION(AppendFile_f);
#define CloseFile_DEFINITION(functionName) void functionName(OpenFile_t* filePntr)
typedef CloseFile_DEFINITION(CloseFile_f);
#define LaunchFile_DEFINITION(functionName) bool32 functionName(const char* filename)
typedef LaunchFile_DEFINITION(LaunchFile_f);

#define DebugWrite_DEFINITION(functionName)     void functionName(const char* string)
typedef DebugWrite_DEFINITION(DebugWrite_f);
#define DebugWriteLine_DEFINITION(functionName) void functionName(const char* message)
typedef DebugWriteLine_DEFINITION(DebugWriteLine_f);
#define DebugPrint_DEFINITION(functionName)     void functionName(const char* formatString, ...)
typedef DebugPrint_DEFINITION(DebugPrint_f);
#define DebugPrintLine_DEFINITION(functionName) void functionName(const char* formatString, ...)
typedef DebugPrintLine_DEFINITION(DebugPrintLine_f);

#define CopyToClipboard_DEFINITION(functionName) void functionName(const void* dataPntr, u32 dataSize)
typedef CopyToClipboard_DEFINITION(CopyToClipboard_f);
#define CopyFromClipboard_DEFINITION(functionName) void* functionName(MemoryArena_t* arenaPntr, u32* dataLengthOut)
typedef CopyFromClipboard_DEFINITION(CopyFromClipboard_f);

#define CreateNewWindow_DEFINITION(functionName) void functionName()
typedef CreateNewWindow_DEFINITION(CreateNewWindow_f);
#define GetAbsolutePath_DEFINITION(functionName) char* functionName(MemoryArena_t* arenaPntr, const char* relativePath)
typedef GetAbsolutePath_DEFINITION(GetAbsolutePath_f);

//+================================================================+
//|                          Structures                            |
//+================================================================+

struct Version_t
{
	u32 major;
	u32 minor;
	u32 build;
};

typedef enum
{
	Platform_Windows,
	Platform_Linux,
	Platform_OSX,
} PlatformType_t;

struct PlatformInfo_t
{
	PlatformType_t platformType;
	Version_t version;
	
	v2i screenSize;
	bool windowResized;
	bool windowHasFocus;
	bool windowIsMinimized;
	
	FreeFileMemory_f*  FreeFileMemory;
	ReadEntireFile_f*  ReadEntireFile;
	WriteEntireFile_f* WriteEntireFile;
	OpenFile_f*        OpenFile;
	AppendFile_f*      AppendFile;
	CloseFile_f*       CloseFile;
	LaunchFile_f*      LaunchFile;
	
	DebugWrite_f*     DebugWrite;
	DebugWriteLine_f* DebugWriteLine;
	DebugPrint_f*     DebugPrint;
	DebugPrintLine_f* DebugPrintLine;
	
	GetComPortList_f* GetComPortList;
	OpenComPort_f*    OpenComPort;
	CloseComPort_f*   CloseComPort;
	ReadComPort_f*    ReadComPort;
	WriteComPort_f*   WriteComPort;
	
	CopyToClipboard_f*   CopyToClipboard;
	CopyFromClipboard_f* CopyFromClipboard;
	
	CreateNewWindow_f* CreateNewWindow;
	GetAbsolutePath_f* GetAbsolutePath;
	
	StartProgramInstance_f* StartProgramInstance;
	GetProgramStatus_f*     GetProgramStatus;
	ReadProgramOutput_f*    ReadProgramOutput;
	WriteProgramInput_f*    WriteProgramInput;
	CloseProgramInstance_f* CloseProgramInstance;
	
	StartThread_f*     StartThread;
	CloseThread_f*     CloseThread;
	GetThreadStatus_f* GetThreadStatus;
	
	r64 timeDelta;
	u64 programTime;
	RealTime_t systemTime;
	RealTime_t localTime;
	
	GLFWwindow* window;
};

struct AppMemory_t
{
	u32 permanantSize;
	u32 transientSize;
	
	void* permanantPntr;
	void* transientPntr;
};

typedef enum
{
	//NOTE: We put the MouseButtons at the bottom so we can use them
	//		when referencing AppInput mouseStartPos and mouseMaxDist
	MouseButton_Left = 0,
	MouseButton_Right,
	MouseButton_Middle,
	
	Button_A,
	Button_B,
	Button_C,
	Button_D,
	Button_E,
	Button_F,
	Button_G,
	Button_H,
	Button_I,
	Button_J,
	Button_K,
	Button_L,
	Button_M,
	Button_N,
	Button_O,
	Button_P,
	Button_Q,
	Button_R,
	Button_S,
	Button_T,
	Button_U,
	Button_V,
	Button_W,
	Button_X,
	Button_Y,
	Button_Z,
	
	Button_0,
	Button_1,
	Button_2,
	Button_3,
	Button_4,
	Button_5,
	Button_6,
	Button_7,
	Button_8,
	Button_9,
	
	Button_F1,
	Button_F2,
	Button_F3,
	Button_F4,
	Button_F5,
	Button_F6,
	Button_F7,
	Button_F8,
	Button_F9,
	Button_F10,
	Button_F11,
	Button_F12,
	
	Button_Enter,
	Button_Backspace,
	Button_Escape,
	Button_Insert,
	Button_Delete,
	Button_Home,
	Button_End,
	Button_PageUp,
	Button_PageDown,
	Button_Tab,
	Button_CapsLock,
	
	Button_Control,
	Button_Alt,
	Button_Shift,
	
	Button_Right,
	Button_Left,
	Button_Up,
	Button_Down,
	
	Button_Plus,
	Button_Minus,
	Button_Pipe,
	Button_OpenBracket,
	Button_CloseBracket,
	Button_Colon,
	Button_Quote,
	Button_Comma,
	Button_Period,
	Button_QuestionMark,
	Button_Tilde,
	Button_Space,
	
	Buttons_NumButtons,
} Buttons_t;

inline const char* GetButtonName(Buttons_t button)
{
	switch (button)
	{
		case Button_A:            return "A";
		case Button_B:            return "B";
		case Button_C:            return "C";
		case Button_D:            return "D";
		case Button_E:            return "E";
		case Button_F:            return "F";
		case Button_G:            return "G";
		case Button_H:            return "H";
		case Button_I:            return "I";
		case Button_J:            return "J";
		case Button_K:            return "K";
		case Button_L:            return "L";
		case Button_M:            return "M";
		case Button_N:            return "N";
		case Button_O:            return "O";
		case Button_P:            return "P";
		case Button_Q:            return "Q";
		case Button_R:            return "R";
		case Button_S:            return "S";
		case Button_T:            return "T";
		case Button_U:            return "U";
		case Button_V:            return "V";
		case Button_W:            return "W";
		case Button_X:            return "X";
		case Button_Y:            return "Y";
		case Button_Z:            return "Z";
		case Button_0:            return "0";
		case Button_1:            return "1";
		case Button_2:            return "2";
		case Button_3:            return "3";
		case Button_4:            return "4";
		case Button_5:            return "5";
		case Button_6:            return "6";
		case Button_7:            return "7";
		case Button_8:            return "8";
		case Button_9:            return "9";
		case Button_F1:           return "F1";
		case Button_F2:           return "F2";
		case Button_F3:           return "F3";
		case Button_F4:           return "F4";
		case Button_F5:           return "F5";
		case Button_F6:           return "F6";
		case Button_F7:           return "F7";
		case Button_F8:           return "F8";
		case Button_F9:           return "F9";
		case Button_F10:          return "F10";
		case Button_F11:          return "F11";
		case Button_F12:          return "F12";
		case Button_Enter:        return "Enter";
		case Button_Backspace:    return "Backspace";
		case Button_Escape:       return "Escape";
		case Button_Insert:       return "Insert";
		case Button_Delete:       return "Delete";
		case Button_Home:         return "Home";
		case Button_End:          return "End";
		case Button_PageUp:       return "PageUp";
		case Button_PageDown:     return "PageDown";
		case Button_Tab:          return "Tab";
		case Button_CapsLock:     return "Caps Lock";
		case Button_Control:      return "Control";
		case Button_Alt:          return "Alt";
		case Button_Shift:        return "Shift";
		case Button_Right:        return "Right";
		case Button_Left:         return "Left";
		case Button_Up:           return "Up";
		case Button_Down:         return "Down";
		case Button_Plus:         return "Plus";
		case Button_Minus:        return "Minus";
		case Button_Pipe:         return "Pipe";
		case Button_OpenBracket:  return "Open Bracket";
		case Button_CloseBracket: return "Close Bracket";
		case Button_Colon:        return "Colon";
		case Button_Quote:        return "Quote";
		case Button_Comma:        return "Comma";
		case Button_Period:       return "Period";
		case Button_QuestionMark: return "Question Mark";
		case Button_Tilde:        return "Tilde";
		case Button_Space:        return "Space";
		case MouseButton_Left:    return "Mouse Left";
		case MouseButton_Right:   return "Mouse Right";
		case MouseButton_Middle:  return "Mouse Middle";
		default:                  return "Unknown";
	};
}

bool ButtonIsPolling(Buttons_t button)
{
	switch (button)
	{
		case Button_Shift: return true;
		case Button_Control: return true;
		case Button_Alt: return true;
	};
	return false;
}

typedef enum : u8
{
	Modifier_Ctrl     = 0x01,
	Modifier_Alt      = 0x02,
	Modifier_Shift    = 0x04,
	Modifier_CapsLock = 0x08,
} ButtonModifier_t;

struct ButtonState_t
{
	u32  transCount; //Transition count since last frame
	u32  pressCount; //Number of times the button was pressed (including holding key messages)
	bool isDown; //Whether or not the button ended down on this frame
};

struct AppInput_t
{
	bool mouseInsideWindow;
	v2 mousePos;
	v2 mouseStartPos[3];
	r32 mouseMaxDist[3];
	v2 scrollDelta;
	v2 scrollValue;
	
	uint32_t numButtonsDown;
	ButtonModifier_t modifiers;
	ButtonState_t buttons[Buttons_NumButtons];
	
	uint8_t textInputLength;
	char textInput[64];
	
	u8 numDroppedFiles;
	const char* droppedFiles[4];
};

typedef enum
{
	Cursor_Default,
	Cursor_Text,
	Cursor_Pointer,
	Cursor_ResizeHorizontal,
	Cursor_ResizeVertical,
	
	NumCursorTypes,
} CursorType_t;

struct AppOutput_t
{
	bool recenterMouse;
	bool showMouse;
	bool closeWindow;
	
	CursorType_t cursorType;
	
	char windowTitle[64];
};


//+================================================================+
//|                  Game Function Definitions                     |
//+================================================================+

#define AppGetVersion_DEFINITION(functionName)      Version_t functionName(bool* resetApplication)
typedef AppGetVersion_DEFINITION(AppGetVersion_f);

#define AppInitialize_DEFINITION(functionName)      void functionName(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory)
typedef AppInitialize_DEFINITION(AppInitialize_f);

#define AppReloading_DEFINITION(functionName)       void functionName(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory)
typedef AppReloading_DEFINITION(AppReloading_f);

#define AppReloaded_DEFINITION(functionName)        void functionName(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory)
typedef AppReloaded_DEFINITION(AppReloaded_f);

#define AppUpdate_DEFINITION(functionName)          void functionName(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory, const AppInput_t* appInput, AppOutput_t* appOutput)
typedef AppUpdate_DEFINITION(AppUpdate_f);

#define AppClosing_DEFINITION(functionName)         void functionName(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory)
typedef AppClosing_DEFINITION(AppClosing_f);

#endif