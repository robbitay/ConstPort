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

FreeFileMemory_DEFINITION(OSX_FreeFileMemory)
{
	if (fileInfo == nullptr) { return; }
	if (fileInfo->content == nullptr) { return; }
	
	free(fileInfo->content);
	fileInfo->content = nullptr;
	fileInfo->size = 0;
}

ReadEntireFile_DEFINITION(OSX_ReadEntireFile)
{
	FileInfo_t result = {};
	
	FILE* fileHandle = fopen(filename, "rb");
	
	if (fileHandle != nullptr)
	{
		fseek(fileHandle, 0, SEEK_END);
		result.size = ftell(fileHandle);
		result.content = malloc(result.size+1);
		
		fseek(fileHandle, 0, SEEK_SET);
		fread(result.content, 1, result.size, fileHandle);
		
		((u8*)result.content)[result.size] = '\0';
		
		fclose(fileHandle);
	}
	
	printf("ReadEntireFile(\"%s\") result: %u bytes %p\n", filename, result.size, result.content);
	return result;
}

WriteEntireFile_DEFINITION(OSX_WriteEntireFile)
{
	//TODO: Write the file
	
	return false;
}

OpenFile_DEFINITION(OSX_OpenFile)
{
	ClearPointer(openFileOut);
	
	//TODO: Open the file
	
	return false;
}

AppendFile_DEFINITION(OSX_AppendFile)
{
	//TODO: Append the file
	
	return false;
}

CloseFile_DEFINITION(OSX_CloseFile)
{
	//TODO: Close the file handle
	
	ClearPointer(filePntr);
}

LaunchFile_DEFINITION(OSX_LaunchFile)
{
	//TODO: Launch the file
	
	return false;
}

DebugWrite_DEFINITION(OSX_DebugWrite)
{
	printf("%s", string);
}

DebugWriteLine_DEFINITION(OSX_DebugWriteLine)
{
	printf("%s", message);
	printf("\n");
}

#define DEBUG_PRINT_BUFFER_SIZE 2048

DebugPrint_DEFINITION(OSX_DebugPrint)
{
	char printBuffer[DEBUG_PRINT_BUFFER_SIZE];
	va_list args;
	
	va_start(args, formatString);
	size_t length = vsnprintf(printBuffer, DEBUG_PRINT_BUFFER_SIZE, formatString, args);
	va_end(args);
	
	if (length == 0)
	{
		
	}
	else if (length < DEBUG_PRINT_BUFFER_SIZE)
	{
		printBuffer[length] = '\0';
		printf("%s", printBuffer);
	}
	else
	{
		printf("[DEBUG PRINT BUFFER OVERFLOW]");
	}
}

DebugPrintLine_DEFINITION(OSX_DebugPrintLine)
{
	char printBuffer[DEBUG_PRINT_BUFFER_SIZE];
	va_list args;
	
	va_start(args, formatString);
	size_t length = vsnprintf(printBuffer, DEBUG_PRINT_BUFFER_SIZE, formatString, args);
	va_end(args);
	
	if (length == 0)
	{
		
	}
	else if (length < DEBUG_PRINT_BUFFER_SIZE)
	{
		printBuffer[length] = '\0';
		printf("%s", printBuffer);
	}
	else
	{
		printf("[DEBUG PRINT BUFFER OVERFLOW]");
	}
	
	printf("\n");
}

GetComPortList_DEFINITION(OSX_GetComPortList)
{
	u32 result = 0;
	
	for (u32 bIndex = 0; bIndex < arrayOutSize; bIndex++)
	{
		//TODO: Check to see if the com port exists
		
		arrayOut[bIndex] = false;
	}
	
	return result;
}

OpenComPort_DEFINITION(OSX_OpenComPort)
{
	ComPort_t result = {};
	
	//TODO: Attempt to open the COM port
	
	return result;
}

CloseComPort_DEFINITION(OSX_CloseComPort)
{
	//TODO: Close the COM port
	
	ClearPointer(comPortPntr);
}

ReadComPort_DEFINITION(OSX_ReadComPort)
{
	//TODO: Read data from the COM port
	
	return 0;
}

WriteComPort_DEFINITION(OSX_WriteComPort)
{
	//TODO: Write to the COM port
}

CopyToClipboard_DEFINITION(OSX_CopyToClipboard)
{
	//TODO: Copy to the clipboard
}

CopyFromClipboard_DEFINITION(OSX_CopyFromClipboard)
{
	u32 result = 0;
	
	//TODO: Copy contents from the clipboard
	
	return result;
}


// +--------------------------------------------------------------+
// |                    Platform Layer Defines                    |
// +--------------------------------------------------------------+
#define WINDOW_TITLE     "ConstPort (OSX)"
#define WINDOW_WIDTH     640
#define WINDOW_HEIGHT    480
#define WINDOW_RESIZABLE false
#define TOPMOST_WINDOW   DEBUG
#define BACKBUFFER_DEPTH_BITS   8
#define BACKBUFFER_STENCIL_BITS 8
#define ANTIALISING_NUM_SAMPLES 4

#define HandleError(outputString) do {                             \
	OSX_WriteLine(outputString);                                   \
	MessageBox("An error occurred during startup!", outputString); \
	return 1;                                                      \
} while (0)

void GlfwErrorCallback(int error, const char* description)
{
	printf("GLFW Error: %s\n", description);
}

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
	
	glfwMakeContextCurrent(window);
	
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
	glViewport(0, 0, screenWidth, screenHeight);
	
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
	PlatformInfo_t platformInfo = {};
	platformInfo.platformType = Platform_OSX;
	platformInfo.screenSize = NewVec2i(screenWidth, screenHeight);
	platformInfo.windowHasFocus = true;
	platformInfo.window = window;
	
	platformInfo.FreeFileMemoryPntr    = OSX_FreeFileMemory;
	platformInfo.ReadEntireFilePntr    = OSX_ReadEntireFile;
	platformInfo.WriteEntireFilePntr   = OSX_WriteEntireFile;
	platformInfo.OpenFilePntr          = OSX_OpenFile;
	platformInfo.AppendFilePntr        = OSX_AppendFile;
	platformInfo.CloseFilePntr         = OSX_CloseFile;
	platformInfo.LaunchFilePntr        = OSX_LaunchFile;
	platformInfo.DebugWritePntr        = OSX_DebugWrite;
	platformInfo.DebugWriteLinePntr    = OSX_DebugWriteLine;
	platformInfo.DebugPrintPntr        = OSX_DebugPrint;
	platformInfo.DebugPrintLinePntr    = OSX_DebugPrintLine;
	platformInfo.GetComPortListPntr    = OSX_GetComPortList;
	platformInfo.OpenComPortPntr       = OSX_OpenComPort;
	platformInfo.CloseComPortPntr      = OSX_CloseComPort;
	platformInfo.ReadComPortPntr       = OSX_ReadComPort;
	platformInfo.WriteComPortPntr      = OSX_WriteComPort;
	platformInfo.CopyToClipboardPntr   = OSX_CopyToClipboard;
	platformInfo.CopyFromClipboardPntr = OSX_CopyFromClipboard;
	
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
	App_Initialize(&platformInfo, &appMemory);
	
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		
		//Do game
		
		glClearColor(20/255.f, 20/255.f, 20/255.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwSwapBuffers(window);
	}
	
	dlclose(dllHandle);
	printf("Dll closed\n");
	
	glfwDestroyWindow(window);
	glfwTerminate();
	printf("Program finished successfully\n");
	return 0;
}