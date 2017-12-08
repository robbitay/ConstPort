/*
File:   main.cpp
Author: Taylor Robbins
Date:   06\04\2017
Description: 
	** The main entry point for our windows program 
*/

//Standard Libraries
#include <windows.h>
#include "Shlwapi.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdbool.h>

//External Libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
#define OPEN_CONSOLE_WINDOW       DEBUG

//NOTE: This must match resource.h in build directory!
#define IDI_ICON1               101

#define HandleError(outputString) do {                                               \
		Win32_WriteLine(outputString);                                               \
		MessageBoxA(NULL, outputString, "Initialization Error Encountered!", MB_OK); \
		return 1;                                                                    \
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
#include "win32_configuration.cpp"
#include "win32_appLoading.cpp"
// #include "win32_keymap.cpp"
// #include "win32_callbacks.cpp"
#include "win32_com.cpp"
#include "win32_clipboard.cpp"
#include "win32_program.cpp"

#define DEBUG_Write(formatStr)          Win32_Write(formatStr)
#define DEBUG_WriteLine(formatStr)      Win32_WriteLine(formatStr)
#define DEBUG_Print(formatStr, ...)     Win32_Print(formatStr, __VA_ARGS__);
#define DEBUG_PrintLine(formatStr, ...) Win32_PrintLine(formatStr, __VA_ARGS__);

#include "plat_keymap.cpp"
#include "plat_callbacks.cpp"

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
	CursorType_t currentCursor;
	GLFWcursor* glfwCursors[NumCursorTypes];
	PlatformConfig_t platformConfig = {};
	
	Win32_WriteLine("Platform Starting...");
	
	glfwSetErrorCallback(GlfwErrorCallback);
	
	Win32_WriteLine("Loading Platform Configuration File...");
	LoadGlobalConfiguration(&platformConfig);
	
	Win32_Write("Initializing glfw...");
	if (!glfwInit())
	{
		HandleError("GLFW Initialization Failed!");
	}
	Win32_WriteLine("Done!");
	
	i32 openglMajor, openglMinor, openglRevision;
	glfwGetVersion(&openglMajor, &openglMinor, &openglRevision);
	Win32_PrintLine("OpenGL %d.%d(%d) supported.", openglMajor, openglMinor, openglRevision);
	
	//+--------------------------------------+
	//|        GLFW Window Creation          |
	//+--------------------------------------+
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE); //Makes MacOSX happy?
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);//GLFW_OPENGL_CORE_PROFILE
	glfwWindowHint(GLFW_RESIZABLE, platformConfig.allowResizeWindow);
	glfwWindowHint(GLFW_FLOATING, platformConfig.topmostWindow);
	glfwWindowHint(GLFW_DECORATED, GL_TRUE);
	glfwWindowHint(GLFW_FOCUSED, GL_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 8);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	glfwWindowHint(GLFW_SAMPLES, platformConfig.antiAliasingSamples);
	
	Win32_Write("Creating GLFW window...");
	window = glfwCreateWindow(platformConfig.windowWidth, platformConfig.windowHeight, WINDOW_TITLE, NULL, NULL);
	
	if (window == nullptr)
	{
		glfwTerminate();
		HandleError("GLFW window creation failed!");
	}
	glfwPollEvents();
	glfwMakeContextCurrent(window);
	glClearColor(20/255.f, 20/255.f, 20/255.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(window);
	Win32_WriteLine("Done!");
	
	//+--------------------------------------+
	//|             Window Icon              |
	//+--------------------------------------+
	{
		FileInfo_t iconFile16 = Win32_ReadEntireFile("Resources/Sprites/icon16.png");
		FileInfo_t iconFile32 = Win32_ReadEntireFile("Resources/Sprites/icon32.png");
		
		i32 numChannels16, numChannels32;
		i32 width16, height16, width32, height32;
		u8* imageData16 = stbi_load_from_memory(
			(u8*)iconFile16.content, iconFile16.size,
			&width16, &height16, &numChannels16, 4);
		u8* imageData32 = stbi_load_from_memory(
			(u8*)iconFile32.content, iconFile32.size,
			&width32, &height32, &numChannels32, 4);
		
		GLFWimage images[2];
		images[0].width = width16;
		images[0].height = height16;
		images[0].pixels = imageData16;
		images[1].width = width32;
		images[1].height = height32;
		images[1].pixels = imageData32;
		
		glfwSetWindowIcon(window, 2, images);
		
		stbi_image_free(imageData16);
		stbi_image_free(imageData32);
		Win32_FreeFileMemory(&iconFile16);
		Win32_FreeFileMemory(&iconFile32);
	}
	
	//+--------------------------------------+
	//|               Cursors                |
	//+--------------------------------------+
	glfwCursors[Cursor_Default]          = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	glfwCursors[Cursor_Text]             = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	glfwCursors[Cursor_Pointer]          = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
	glfwCursors[Cursor_ResizeHorizontal] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	glfwCursors[Cursor_ResizeVertical]   = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	glfwSetCursor(window, glfwCursors[Cursor_Default]);
	currentCursor = Cursor_Default;
	
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
		platformConfig.minWindowWidth, platformConfig.minWindowHeight,
		GLFW_DONT_CARE, GLFW_DONT_CARE);
	if (platformConfig.forceAspectRatio)
	{
		glfwSetWindowAspectRatio(window, platformConfig.aspectRatio.x, platformConfig.aspectRatio.y);
	}
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
	glfwSetWindowFocusCallback(window,     GlfwWindowFocusCallback);
	glfwSetKeyCallback(window,             GlfwKeyPressedCallback);
	glfwSetCharCallback(window,            GlfwCharPressedCallback);
	glfwSetCursorPosCallback(window,       GlfwCursorPosCallback);
	glfwSetMouseButtonCallback(window,     GlfwMousePressCallback);
	glfwSetScrollCallback(window,          GlfwMouseScrollCallback);
	glfwSetCursorEnterCallback(window,     GlfwCursorEnteredCallback);
	
	//+--------------------------------------+
	//|        Fill Platform Info            |
	//+--------------------------------------+
	platformInfo.platformType             = Platform_Windows;
	platformInfo.version.major            = PLATFORM_VERSION_MAJOR;
	platformInfo.version.minor            = PLATFORM_VERSION_MINOR;
	platformInfo.version.build            = PLATFORM_VERSION_BUILD;
	platformInfo.screenSize               = NewVec2i(screenWidth, screenHeight);
	platformInfo.windowHasFocus           = true;
	platformInfo.window                   = window;
	
	platformInfo.DebugWrite           = Win32_Write;
	platformInfo.DebugWriteLine       = Win32_WriteLine;
	platformInfo.DebugPrint           = Win32_Print;
	platformInfo.DebugPrintLine       = Win32_PrintLine;
	platformInfo.FreeFileMemory       = Win32_FreeFileMemory;
	platformInfo.ReadEntireFile       = Win32_ReadEntireFile;
	platformInfo.WriteEntireFile      = Win32_WriteEntireFile;
	platformInfo.OpenFile             = Win32_OpenFile;
	platformInfo.AppendFile           = Win32_AppendFile;
	platformInfo.CloseFile            = Win32_CloseFile;
	platformInfo.LaunchFile           = Win32_LaunchFile;
	platformInfo.GetComPortList       = Win32_GetComPortList;
	platformInfo.OpenComPort          = Win32_OpenComPort;
	platformInfo.CloseComPort         = Win32_CloseComPort;
	platformInfo.ReadComPort          = Win32_ReadComPort;
	platformInfo.WriteComPort         = Win32_WriteComPort;
	platformInfo.CopyToClipboard      = Win32_CopyToClipboard;
	platformInfo.CopyFromClipboard    = Win32_CopyFromClipboard;
	platformInfo.StartProgramInstance = Win32_StartProgramInstance;
	platformInfo.GetProgramStatus     = Win32_GetProgramStatus;
	platformInfo.ReadProgramOutput    = Win32_ReadProgramOutput;
	platformInfo.WriteProgramInput    = Win32_WriteProgramInput;
	platformInfo.CloseProgramInstance = Win32_CloseProgramInstance;
	platformInfo.CreateNewWindow      = Win32_CreateNewWindow;
	platformInfo.GetAbsolutePath      = Win32_GetAbsolutePath;
	
	//+--------------------------------------+
	//|         Application Memory           |
	//+--------------------------------------+
	appMemory.permanantSize = Megabytes(platformConfig.permanantMemorySize);
	appMemory.transientSize = Megabytes(platformConfig.transientMemorySize);
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
	
	if (PathFileExists(appDllFullPath) == false)
	{
		glfwTerminate();
		HandleError("Could not find application DLL");
	}
	
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
	
	// +==============================+
	// |   Fill Initial AppInput_t    |
	// +==============================+
	{
		r64 mousePosX, mousePosY;
		glfwGetCursorPos(window, &mousePosX, &mousePosY);
		currentInput->mousePos = NewVec2((r32)mousePosX, (r32)mousePosY);
		currentInput->mouseInsideWindow = (mousePosX >= 0 && mousePosY >= 0 && mousePosX < platformInfo.screenSize.x && mousePosY < platformInfo.screenSize.y);
	}
	
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
			currentInput->buttons[bIndex].pressCount = 0;
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
		
		u64 lastTime = platformInfo.programTime;
		platformInfo.programTime = (u64)(glfwGetTime() * 1000);
		platformInfo.timeDelta = (r64)(platformInfo.programTime - lastTime) / (1000 / glfwModePntr->refreshRate);
		
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
		
		if (appOutput.cursorType != currentCursor)
		{
			glfwSetCursor(window, glfwCursors[appOutput.cursorType]);
			currentCursor = appOutput.cursorType;
		}
	}
	
	glfwTerminate();
	
	Win32_WriteLine("Program finished.");
	return 0;
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
	Win32_PrintLine("Assertion Failure! %s in \"%s\" line %d: (%s) is not true", function, &filename[fileNameStart], lineNumber, expressionStr);
}
#endif
