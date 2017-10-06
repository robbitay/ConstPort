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

#include "platformInterface.h"

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
	printf("Hello World!\n");
	
	if (!glfwInit())
	{
		printf("Could not initialize GLFW!\n");
		return 1;
	}
	glfwSetErrorCallback(GlfwErrorCallback);
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true); //Makes MacOSX happy?
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//
	glfwWindowHint(GLFW_RESIZABLE, true);
	glfwWindowHint(GLFW_FLOATING, true);
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
	
	GLFWwindow* window = glfwCreateWindow(640, 480, "OSX GLFW Window!", NULL, NULL);
	if (window == nullptr)
	{
		printf("Could not create OpenGL window!\n");
		glfwTerminate();
		return 1;
	}
	
	glfwMakeContextCurrent(window);
	
	void* dllHandle = dlopen("ConstPort.dll", RTLD_NOW);
	printf("Dll Handle: %p\n", dllHandle);
	
	void* App_GetVersion      = dlsym(dllHandle, "App_GetVersion");
	printf("App_GetVersion:      %p\n", App_GetVersion);
	void* App_Initialize      = dlsym(dllHandle, "App_Initialize");
	printf("App_Initialize:      %p\n", App_Initialize);
	void* App_Reloaded        = dlsym(dllHandle, "App_Reloaded");
	printf("App_Reloaded:        %p\n", App_Reloaded);
	void* App_Update          = dlsym(dllHandle, "App_Update");
	printf("App_Update:          %p\n", App_Update);
	void* App_GetSoundSamples = dlsym(dllHandle, "App_GetSoundSamples");
	printf("App_GetSoundSamples: %p\n", App_GetSoundSamples);
	void* App_Closing         = dlsym(dllHandle, "App_Closing");
	printf("App_Closing:         %p\n", App_Closing);
	
	dlclose(dllHandle);
	printf("Dll closed\n");
	
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		
		//Do game
		
		glClearColor(20/255.f, 20/255.f, 20/255.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwSwapBuffers(window);
	}
	
	glfwDestroyWindow(window);
	glfwTerminate();
	printf("Program finished successfully\n");
	return 0;
}