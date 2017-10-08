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
#include <libgen.h> //needed for dirname
#include <fcntl.h> //needed for open
#include <unistd.h> //needed for close
#include <cerrno>
#include <time.h> //needed for gmtime
#include <sys/stat.h> //needed for stat

#include "platformInterface.h"
#include "osx_version.h"

PlatformInfo_t PlatformInfo;

#include "osx_helpers.cpp"
#include "osx_debug.cpp"
#include "osx_clipboard.cpp"
#include "osx_com.cpp"
#include "osx_files.cpp"
#include "osx_keymap.cpp"
#include "osx_callbacks.cpp"
#include "osx_appLoading.cpp"

// +--------------------------------------------------------------+
// |                    Platform Layer Defines                    |
// +--------------------------------------------------------------+
#define APPLICATION_DLL_NAME "ConstPort.dll"

#define WINDOW_TITLE     "ConstPort (OSX)"
#define WINDOW_WIDTH     640
#define WINDOW_HEIGHT    480
#define WINDOW_RESIZABLE true
#define TOPMOST_WINDOW   DEBUG
#define BACKBUFFER_DEPTH_BITS   8
#define BACKBUFFER_STENCIL_BITS 8
#define ANTIALISING_NUM_SAMPLES 4

const char* GetExecutableDirectory(int argc, char** argv)
{
	const char* result = nullptr;
	
	char* filePath = realpath(argv[0], 0);
	result = dirname(filePath);
	free(filePath);
	
	return result;
}

int main(int argc, char** argv)
{
	i32 screenWidth, screenHeight;
	
	printf("ConstPort OSX Platform v%u.%u(%u)\n", PLATFORM_VERSION_MAJOR, PLATFORM_VERSION_MINOR, PLATFORM_VERSION_BUILD);
	
	const char* exeDirectory = GetExecutableDirectory(argc, argv);
	
	// +==================================+
	// | Print the command line arguments |
	// +==================================+
	#if 0
	printf("Command Line Arguments: %d\n", argc);
	for (i32 aIndex = 0; aIndex < argc; aIndex++)
	{
		printf("[%d] = \"%s\"\n", aIndex, argv[aIndex]);
	}
	printf("Current Directory: \"%s\"\n", realpath("./", 0));
	printf("Executable Directory: \"%s\"\n", exeDirectory);
	#endif
	
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
	char dllPathBuffer[256] = {};
	snprintf(dllPathBuffer, ArrayCount(dllPathBuffer), "%s/%s", exeDirectory, APPLICATION_DLL_NAME);
	char tempDllPathBuffer[256] = {};
	snprintf(tempDllPathBuffer, ArrayCount(tempDllPathBuffer), "%s/TEMP_%s", exeDirectory, APPLICATION_DLL_NAME);
	LoadedApp_t application;
	LoadDllCode(dllPathBuffer, tempDllPathBuffer, &application);
	if (application.isValid == false)
	{
		printf("Unable to load application DLL at \"%s\"\n", dllPathBuffer);
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}
	
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
	application.Initialize(&PlatformInfo, &appMemory);
	
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
	
	// +==============================+
	// |       Main Update Loop       |
	// +==============================+
	while (!glfwWindowShouldClose(window))
	{
		#if DEBUG
		time_t newFileWriteTime = GetLastModifiedTime(dllPathBuffer);
		
		if (application.lastWriteTime != newFileWriteTime)
		{
			FreeDllCode(&application);
			LoadDllCode(dllPathBuffer, tempDllPathBuffer, &application);
			if (application.isValid)
			{
				printf("Loaded new application version %u.%u(%u)\n",
					application.version.major, application.version.minor, application.version.build);
			}
			application.Reloaded(&PlatformInfo, &appMemory);
		}
		#endif
		
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
		
		application.Update(&PlatformInfo, &appMemory, currentInput, &appOutput);
		
		glfwSwapBuffers(window);
	}
	
	glfwDestroyWindow(window);
	glfwTerminate();
	printf("Program finished successfully\n");
	return 0;
}