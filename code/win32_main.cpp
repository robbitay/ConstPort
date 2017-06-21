/*
File:   main.cpp
Author: Taylor Robbins
Date:   06\04\2017
Description: 
	** The main entry point for our windows program 
*/

//Standard Libraries
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdbool.h>

//External Libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//Project Headers
#include "platformInterface.h"
#include "win32_version.h"

//+================================================================+
//|                            Defines                             |
//+================================================================+
#define WINDOW_CLASS_NAME         "ConstPort"
#define WINDOW_TITLE              "Const Port"
#define APPLICATION_DLL_NAME      "ConstPort.dll"
#define APPLICATION_DLL_TEMP_NAME "ConstPort_TEMP.dll"
#define WINDOW_WIDTH              600
#define WINDOW_HEIGHT             350
#define WINDOW_MIN_WIDTH          600
#define WINDOW_MIN_HEIGHT         350
#define WINDOW_MAX_WIDTH          GLFW_DONT_CARE
#define WINDOW_MAX_HEIGHT         GLFW_DONT_CARE
#define ENFORCE_ASPECT_RATIO      false
#define WINDOW_ASPECT_RATIO       16,9
#define ALLOW_RESIZE_WINDOW       true
#define TOPMOST_WINDOW            DEBUG
#define OPEN_CONSOLE_WINDOW       DEBUG

//NOTE: This must match resource.h in build directory!
#define IDI_ICON1               101

#define HandleError(outputString) do { \
		Win32_WriteLine(outputString); \
		return 1;                      \
	} while(0)

//+================================================================+
//|                    Platform Layer Globals                      |
//+================================================================+
static bool WindowResized;
static PlatformInfo_t* GL_PlatformInfo;
static Version_t PlatformVersion = {
	PLATFORM_VERSION_MAJOR, 
	PLATFORM_VERSION_MINOR, 
	PLATFORM_VERSION_BUILD
};

//+================================================================+
//|                Platform Layer Implementations                  |
//+================================================================+
#include "win32_debug.cpp"
#include "win32_helpers.cpp"
#include "win32_appLoading.cpp"
#include "win32_keymap.cpp"
#include "win32_callbacks.cpp"
#include "win32_com.cpp"
#include "win32_clipboard.cpp"

//+================================================================+
//|                      Windows Entry Point                       |
//+================================================================+
#if OPEN_CONSOLE_WINDOW
int main()
#else
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
#endif
{
	GLFWwindow* window = nullptr;
	int32 screenWidth = 0, screenHeight = 0;
	char exePath[MAX_PATH] = {};
	char appDllFullPath[MAX_PATH] = {};
	char tempDllFullPath[MAX_PATH] = {};
	PlatformInfo_t platformInfo = {};
	GL_PlatformInfo = &platformInfo;
	LoadedApp_t loadedApp = {};
	AppMemory_t appMemory = {};
	AppInput_t inputRingBuffer[2] = {};
	AppInput_t* currentInput = &inputRingBuffer[0];
	AppInput_t* lastInput = &inputRingBuffer[1];
	
	Win32_WriteLine("Application Starting...");
	
	glfwSetErrorCallback(GlfwErrorCallback);
	
	if (!glfwInit())
	{
		HandleError("GLFW Initialization Failed!");
	}
	
	//+--------------------------------------+
	//|           Window Creation            |
	//+--------------------------------------+
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //Makes MacOSX happy?
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);//GLFW_OPENGL_CORE_PROFILE
	glfwWindowHint(GLFW_RESIZABLE, ALLOW_RESIZE_WINDOW);
	glfwWindowHint(GLFW_FLOATING, TOPMOST_WINDOW);
	glfwWindowHint(GLFW_DECORATED, GL_TRUE);
	glfwWindowHint(GLFW_FOCUSED, GL_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 8);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	glfwWindowHint(GLFW_SAMPLES, 4);
	
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	
	if (window == nullptr)
	{
		glfwTerminate();
		HandleError("Window could not be created!");
	}
	
	//TODO: Create load_icon function to make this work!
	// GLFWimage images[1];
	// images[0] = load_icon("Resources/Sprites/test.png");
	// glfwSetWindowIcon(window, 1, images);
	
	glfwMakeContextCurrent(window);
	
	//+--------------------------------------+
	//|         GLEW Initialization          |
	//+--------------------------------------+
	glewExperimental = GL_TRUE;
	GLenum glewInitError = glewInit();
	if (glewInitError != GLEW_OK)
	{
		glfwTerminate();
		HandleError("Could not initialize GLEW.");
	}
	
	//+--------------------------------------+
	//|         OpenGL Information           |
	//+--------------------------------------+
	const GLubyte* openglVersionStr = glGetString(GL_VERSION);
	const GLubyte* rendererStr = glGetString(GL_RENDERER);
	const GLFWvidmode* glfwModePntr = glfwGetVideoMode(glfwGetPrimaryMonitor());
	Win32_PrintLine("OpenGL Version %s", openglVersionStr);
	Win32_PrintLine("Rendering with \"%s\"", rendererStr);
	Win32_PrintLine("Monitor Refresh Rate: %dHz", glfwModePntr->refreshRate);
	
	glfwSetWindowSizeLimits(window, 
		WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT,
		WINDOW_MAX_WIDTH, WINDOW_MAX_HEIGHT);
	#if ENFORCE_ASPECT_RATIO
	glfwSetWindowAspectRatio(window, WINDOW_ASPECT_RATIO);
	#endif
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
	glViewport(0, 0, screenWidth, screenHeight);
	Win32_PrintLine("Screen Size: %dx%d", screenWidth, screenHeight);
	glfwSwapInterval(1);
	
	//+--------------------------------------+
	//|         Register Callbacks           |
	//+--------------------------------------+
	glfwSetWindowCloseCallback(window,     GlfwWindowCloseCallback);
	glfwSetFramebufferSizeCallback(window, GlfwWindowSizeCallback);
	glfwSetWindowPosCallback(window,       GlfwWindowMoveCallback);
	glfwSetWindowIconifyCallback(window,   GlfwWindowMinimizeCallback);
	glfwSetKeyCallback(window,             GlfwKeyPressedCallback);
	glfwSetCharCallback(window,            GlfwCharPressedCallback);
	glfwSetCursorPosCallback(window,       GlfwCursorPosCallback);
	glfwSetMouseButtonCallback(window,     GlfwMousePressCallback);
	glfwSetScrollCallback(window,          GlfwMouseScrollCallback);
	
	//+--------------------------------------+
	//|        Fill Platform Info            |
	//+--------------------------------------+
	platformInfo.platformType          = Platform_Windows;
	platformInfo.screenSize            = NewVec2i(screenWidth, screenHeight);
	platformInfo.windowHasFocus        = true;
	platformInfo.DebugWritePntr        = Win32_Write;
	platformInfo.DebugWriteLinePntr    = Win32_WriteLine;
	platformInfo.DebugPrintPntr        = Win32_Print;
	platformInfo.DebugPrintLinePntr    = Win32_PrintLine;
	platformInfo.FreeFileMemoryPntr    = Win32_FreeFileMemory;
	platformInfo.ReadEntireFilePntr    = Win32_ReadEntireFile;
	platformInfo.WriteEntireFilePntr   = Win32_WriteEntireFile;
	platformInfo.GetComPortListPntr    = Win32_GetComPortList;
	platformInfo.OpenComPortPntr       = Win32_OpenComPort;
	platformInfo.CloseComPortPntr      = Win32_CloseComPort;
	platformInfo.ReadComPortPntr       = Win32_ReadComPort;
	platformInfo.WriteComPortPntr      = Win32_WriteComPort;
	platformInfo.CopyToClipboardPntr   = Win32_CopyToClipboard;
	platformInfo.CopyFromClipboardPntr = Win32_CopyFromClipboard;
	platformInfo.window                = window;
	
	//+--------------------------------------+
	//|         Application Memory           |
	//+--------------------------------------+
	appMemory.permanantSize = Megabytes(64);
	appMemory.transientSize = Gigabytes(1);
	appMemory.permanantPntr = (void*)VirtualAlloc(0, appMemory.permanantSize,
		MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	appMemory.transientPntr = (void*)VirtualAlloc(0, appMemory.transientSize,
		MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	
	if (appMemory.permanantPntr == nullptr || appMemory.transientPntr == nullptr)
	{
		glfwTerminate();
		HandleError("Could not allocate enough memory for application!");
	}
	
	//+--------------------------------------+
	//|      Loading Application Code        |
	//+--------------------------------------+
	uint32 exePathLength = GetRunningDirectory(exePath, ArrayCount(exePath));
	Win32_PrintLine("Our exe path is \"%s\"", exePath);
	
	bool catSuccess = CatStrings(exePath, APPLICATION_DLL_NAME, appDllFullPath, ArrayCount(appDllFullPath));
	Assert(catSuccess);
	catSuccess = CatStrings(exePath, APPLICATION_DLL_TEMP_NAME, tempDllFullPath, ArrayCount(tempDllFullPath));
	Assert(catSuccess);
	
	if (LoadDllCode(appDllFullPath, tempDllFullPath, &loadedApp))
	{
		Win32_PrintLine("Loaded application version %u.%u:%03u",
			loadedApp.version.major, loadedApp.version.minor, loadedApp.version.build);
	}
	else
	{
		glfwTerminate();
		HandleError("Could not load application DLL!");
	}
	
	UpdateWindowTitle(window, WINDOW_TITLE, &PlatformVersion, &loadedApp.version);
	
	loadedApp.AppInitializePntr(&platformInfo, &appMemory);
	
	//+--------------------------------------+
	//|              Main Loop               |
	//+--------------------------------------+
	AppOutput_t appOutput = {};
	while (glfwWindowShouldClose(window) == false)
	{
		#if DEBUG
			//TOOD: Delay this so we don't get double loads
			//Check to see if we should reload the application DLL
			FILETIME newDllFiletime = GetFileWriteTime(appDllFullPath);
			if (CompareFileTime(&newDllFiletime, &loadedApp.lastWriteTime) != 0)
			{
				FreeDllCode(&loadedApp);
				
				if (LoadDllCode(appDllFullPath, tempDllFullPath, &loadedApp))
				{
					Win32_PrintLine("Loaded application version %u.%u:%03u",
						loadedApp.version.major, loadedApp.version.minor, loadedApp.version.build);
					
					bool resetApplication = false;
					loadedApp.AppGetVersionPntr(&resetApplication);
					if (resetApplication)
					{
						Win32_WriteLine("Resetting application");
						
						memset(appMemory.permanantPntr, 0x00, appMemory.permanantSize);
						memset(appMemory.transientPntr, 0x00, appMemory.transientSize);
						
						//TODO: Find a way to reset the opengl context or
						//		maybe re-open the window altogether
						
						loadedApp.AppInitializePntr(&platformInfo, &appMemory);
					}
					else
					{
						loadedApp.AppReloadedPntr(&platformInfo, &appMemory);
					}
				}
				else
				{
					glfwTerminate();
					HandleError("Could not load application DLL!");
				}
				
				// UpdateWindowTitle(window, &PlatformVersion, &loadedApp.version);
			}
		#endif
		
		//Swap the pointers for current and last input
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
		
		platformInfo.windowResized = false;
		
		// Set the user pointer on the window so we can access 
		// the input from the callbacks
		glfwSetWindowUserPointer(window, currentInput);
		
		glfwPollEvents();
		
		//TODO: Should we do some sort of polling keyboard input
		//		periodically to make sure we didn't miss a key release?
		
		glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
		glViewport(0, 0, screenWidth, screenHeight);
		platformInfo.screenSize = NewVec2i(screenWidth, screenHeight);
		
		r64 lastTime = platformInfo.programTime;
		platformInfo.programTime = glfwGetTime();
		platformInfo.timeDelta = platformInfo.programTime - lastTime;
		
		SYSTEMTIME systemTime = {};
		SYSTEMTIME localTime = {};
		GetSystemTime(&systemTime);
		GetLocalTime(&localTime);
		
		platformInfo.systemTime = {};
		platformInfo.systemTime.year        = systemTime.wYear;
		platformInfo.systemTime.month       = (systemTime.wMonth-1);
		platformInfo.systemTime.day         = (systemTime.wDay-1);
		platformInfo.systemTime.hour        = systemTime.wHour;
		platformInfo.systemTime.minute      = systemTime.wMinute;
		platformInfo.systemTime.second      = systemTime.wSecond;
		platformInfo.systemTime.millisecond = systemTime.wMilliseconds;
		platformInfo.localTime = {};
		platformInfo.localTime.year         = localTime.wYear;
		platformInfo.localTime.month        = (localTime.wMonth-1);
		platformInfo.localTime.day          = (localTime.wDay-1);
		platformInfo.localTime.hour         = localTime.wHour;
		platformInfo.localTime.minute       = localTime.wMinute;
		platformInfo.localTime.second       = localTime.wSecond;
		platformInfo.localTime.millisecond  = localTime.wMilliseconds;
		
		ClearStruct(appOutput);
		loadedApp.AppUpdatePntr(&platformInfo, &appMemory, currentInput, &appOutput);
		
		glfwSwapBuffers(window);
		
		UpdateWindowTitle(window, appOutput.windowTitle, &PlatformVersion, &loadedApp.version);
	}
	
	glfwTerminate();
	
	Win32_WriteLine("Program finished.");
	return 0;
}