/*
File:   osx_main.cpp
Author: Taylor Robbins
Date:   07\10\2017
Description: 
	** This is the main file to be compiled for the OSX platform layer
	** it #includes all other files that need to be compiled 
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <dlfcn.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

#include "platformInterface.h"
#include "osx_version.h"

PlatformInfo_t PlatformInfo;

#include "osx_debug.cpp"
#include "osx_clipboard.cpp"
#include "osx_com.cpp"
#include "osx_files.cpp"
#include "osx_keymap.cpp"
#include "osx_callbacks.cpp"

// +--------------------------------------------------------------+
// |                    Platform Layer Defines                    |
// +--------------------------------------------------------------+
#define WINDOW_TITLE     "ConstPort (OSX)"
#define WINDOW_WIDTH     640
#define WINDOW_HEIGHT    480
#define WINDOW_RESIZABLE true
#define TOPMOST_WINDOW   DEBUG
#define BACKBUFFER_DEPTH_BITS   8
#define BACKBUFFER_STENCIL_BITS 8
#define ANTIALISING_NUM_SAMPLES 4

int main()
{
	i32 screenWidth, screenHeight;
	
	printf("ConstPort OSX Platform v%u.%u(%u)\n", PLATFORM_VERSION_MAJOR, PLATFORM_VERSION_MINOR, PLATFORM_VERSION_BUILD);
	
	// +==============================+
	// |       Initialize GLFW        |
	// +==============================+
	if (!glfwInit())
	{
		printf("Could not initialize GLFW!\n");
		return 1;
	}
	glfwSetErrorCallback(GlfwErrorCallback);
	
	// +==============================+
	// |    Create the GLFW Window    |
	// +==============================+
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true); //Makes MacOSX happy?
	glfwWindowHint(GLFW_CLIENT_API,     GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE,      WINDOW_RESIZABLE);
	glfwWindowHint(GLFW_FLOATING,       TOPMOST_WINDOW);
	glfwWindowHint(GLFW_DECORATED,      true);
	glfwWindowHint(GLFW_FOCUSED,        true);
	glfwWindowHint(GLFW_DOUBLEBUFFER,   true);
	glfwWindowHint(GLFW_RED_BITS,       8);
	glfwWindowHint(GLFW_GREEN_BITS,     8);
	glfwWindowHint(GLFW_BLUE_BITS,      8);
	glfwWindowHint(GLFW_ALPHA_BITS,     8);
	glfwWindowHint(GLFW_DEPTH_BITS,     BACKBUFFER_DEPTH_BITS);
	glfwWindowHint(GLFW_STENCIL_BITS,   BACKBUFFER_STENCIL_BITS);
	glfwWindowHint(GLFW_SAMPLES,        ANTIALISING_NUM_SAMPLES);
	
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (window == nullptr)
	{
		printf("Could not create OpenGL window!\n");
		glfwTerminate();
		return 1;
	}
	
	Assert(false);
	
	// +==============================+
	// |        Configure GLFW        |
	// +==============================+
	glfwMakeContextCurrent(window);
	
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
	glViewport(0, 0, screenWidth, screenHeight);
	glfwSwapInterval(1);
	
	//+--------------------------------------+
	//|         GLEW Initialization          |
	//+--------------------------------------+
	glewExperimental = GL_TRUE;
	GLenum glewInitError = glewInit();
	if (glewInitError != GLEW_OK)
	{
		glfwTerminate();
		printf("Could not initialize GLEW.\n");
		return 1;
	}
	
	// +==============================+
	// |  Initialize PlatformInfo_t   |
	// +==============================+
	PlatformInfo = {};
	PlatformInfo.platformType = Platform_OSX;
	PlatformInfo.screenSize = NewVec2i(screenWidth, screenHeight);
	PlatformInfo.windowHasFocus = true;
	PlatformInfo.window = window;
	
	PlatformInfo.FreeFileMemoryPntr    = OSX_FreeFileMemory;
	PlatformInfo.ReadEntireFilePntr    = OSX_ReadEntireFile;
	PlatformInfo.WriteEntireFilePntr   = OSX_WriteEntireFile;
	PlatformInfo.OpenFilePntr          = OSX_OpenFile;
	PlatformInfo.AppendFilePntr        = OSX_AppendFile;
	PlatformInfo.CloseFilePntr         = OSX_CloseFile;
	PlatformInfo.LaunchFilePntr        = OSX_LaunchFile;
	PlatformInfo.DebugWritePntr        = OSX_Write;
	PlatformInfo.DebugWriteLinePntr    = OSX_WriteLine;
	PlatformInfo.DebugPrintPntr        = OSX_Print;
	PlatformInfo.DebugPrintLinePntr    = OSX_PrintLine;
	PlatformInfo.GetComPortListPntr    = OSX_GetComPortList;
	PlatformInfo.OpenComPortPntr       = OSX_OpenComPort;
	PlatformInfo.CloseComPortPntr      = OSX_CloseComPort;
	PlatformInfo.ReadComPortPntr       = OSX_ReadComPort;
	PlatformInfo.WriteComPortPntr      = OSX_WriteComPort;
	PlatformInfo.CopyToClipboardPntr   = OSX_CopyToClipboard;
	PlatformInfo.CopyFromClipboardPntr = OSX_CopyFromClipboard;
	
	StartProgramInstance_f* StartProgramInstancePntr;
	GetProgramStatus_f*     GetProgramStatusPntr;
	ReadProgramOutput_f*    ReadProgramOutputPntr;
	WriteProgramInput_f*    WriteProgramInputPntr;
	CloseProgramInstance_f* CloseProgramInstancePntr;
	
	// +==============================+
	// |   Load the Application DLL   |
	// +==============================+
	void* dllHandle = dlopen("ConstPort.dll", RTLD_NOW);
	printf("Dll Handle: %p\n", dllHandle);
	
	AppGetVersion_f* App_GetVersion           = (AppGetVersion_f*)dlsym(dllHandle, "App_GetVersion");
	printf("App_GetVersion:      %p\n", App_GetVersion);
	Assert(App_GetVersion != nullptr);
	AppInitialize_f* App_Initialize           = (AppInitialize_f*)dlsym(dllHandle, "App_Initialize");
	printf("App_Initialize:      %p\n", App_Initialize);
	Assert(App_Initialize != nullptr);
	AppReloaded_f* App_Reloaded               = (AppReloaded_f*)dlsym(dllHandle, "App_Reloaded");
	printf("App_Reloaded:        %p\n", App_Reloaded);
	Assert(App_Reloaded != nullptr);
	AppUpdate_f* App_Update                   = (AppUpdate_f*)dlsym(dllHandle, "App_Update");
	printf("App_Update:          %p\n", App_Update);
	Assert(App_Update != nullptr);
	AppGetSoundSamples_f* App_GetSoundSamples = (AppGetSoundSamples_f*)dlsym(dllHandle, "App_GetSoundSamples");
	printf("App_GetSoundSamples: %p\n", App_GetSoundSamples);
	Assert(App_GetSoundSamples != nullptr);
	AppClosing_f* App_Closing                 = (AppClosing_f*)dlsym(dllHandle, "App_Closing");
	printf("App_Closing:         %p\n", App_Closing);
	Assert(App_Closing != nullptr);
	
	// +==================================+
	// | Allocate the Application Memory  |
	// +==================================+
	AppMemory_t appMemory = {};
	appMemory.permanantSize = Megabytes(16);
	appMemory.transientSize = Megabytes(16);
	
	appMemory.permanantPntr = mmap(0, appMemory.permanantSize,
		PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);
	
	appMemory.transientPntr = mmap(0, appMemory.transientSize,
		PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);
	
	printf("Permanant %u bytes: %p\n", appMemory.permanantSize, appMemory.permanantPntr);
	printf("Transient %u bytes: %p\n", appMemory.transientSize, appMemory.transientPntr);
	
	// +==============================+
	// |    Initialize Application    |
	// +==============================+
	App_Initialize(&PlatformInfo, &appMemory);
	
	// +==============================+
	// |       Setup App Input        |
	// +==============================+
	AppOutput_t appOutput = {};
	AppInput_t appInputArray[2] = {};
	AppInput_t* lastInput = &appInputArray[0];
	AppInput_t* currentInput = &appInputArray[1];
	
	glfwSetWindowUserPointer(window, currentInput);
	glfwSetWindowCloseCallback(window,     GlfwWindowCloseCallback);
	glfwSetFramebufferSizeCallback(window, GlfwWindowSizeCallback);
	glfwSetWindowPosCallback(window,       GlfwWindowMoveCallback);
	glfwSetWindowIconifyCallback(window,   GlfwWindowMinimizeCallback);
	glfwSetKeyCallback(window,             GlfwKeyPressedCallback);
	glfwSetCharCallback(window,            GlfwCharPressedCallback);
	glfwSetCursorPosCallback(window,       GlfwCursorPosCallback);
	glfwSetMouseButtonCallback(window,     GlfwMousePressCallback);
	glfwSetScrollCallback(window,          GlfwMouseScrollCallback);
	
	while (!glfwWindowShouldClose(window))
	{
		// +==============================+
		// |    Swap AppInput Pointers    |
		// +==============================+
		AppInput_t* tempInputPointer = lastInput;
		lastInput = currentInput;
		currentInput = tempInputPointer;
		*currentInput = *lastInput;
		currentInput->textInputLength = 0;
		currentInput->scrollDelta = Vec2_Zero;
		//Clear the transition counts
		for (uint32_t bIndex = 0; bIndex < ArrayCount(currentInput->buttons); bIndex++)
		{
			currentInput->buttons[bIndex].transCount = 0;
		}
		PlatformInfo.windowResized = false;
		
		glfwSetWindowUserPointer(window, currentInput);
		
		// +==============================+
		// |      Poll Window Event       |
		// +==============================+
		glfwPollEvents();
		
		App_Update(&PlatformInfo, &appMemory, currentInput, &appOutput);
		
		glfwSwapBuffers(window);
	}
	
	dlclose(dllHandle);
	printf("Dll closed\n");
	
	glfwDestroyWindow(window);
	glfwTerminate();
	printf("Program finished successfully\n");
	return 0;
}