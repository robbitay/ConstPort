/*
File:   osx_callbacks.cpp
Author: Taylor Robbins
Date:   10\07\2017
Description: 
	** Holds all of the GLFW function callbacks
*/

void GlfwErrorCallback(i32 errorCode, const char* description)
{
	OSX_PrintLine("GlfwErrorCallback: %X \"%s\"", errorCode, description);
}
void GlfwWindowCloseCallback(GLFWwindow* window)
{
	OSX_WriteLine("Window Closing");
	
	//TODO: Should we dissallow the close in some scenarios?
	// glfwSetWindowShouldClose(window, false);
}
void GlfwWindowSizeCallback(GLFWwindow* window, i32 screenWidth, i32 screenHeight)
{
	OSX_PrintLine("Resized: %dx%d", screenWidth, screenHeight);
	
	PlatformInfo.screenSize = NewVec2i(screenWidth, screenHeight);
	PlatformInfo.windowResized = true;
}
void GlfwWindowMoveCallback(GLFWwindow* window, i32 posX, i32 posY)
{
	// OSX_PrintLine("Received GlfwWindowMoveCallback: (%d, %d)", posX, posY);
}
void GlfwWindowMinimizeCallback(GLFWwindow* window, i32 isMinimized)
{
	OSX_PrintLine("Window %s", isMinimized ? "Minimized" : "Restored");
	PlatformInfo.windowIsMinimized = (isMinimized > 0);
}
void GlfwKeyPressedCallback(GLFWwindow* window, i32 key, i32 scanCode, i32 action, i32 modifiers)
{
	// OSX_WriteLine("Received GlfwKeyPressedCallback");
	AppInput_t* currentInput = (AppInput_t*)glfwGetWindowUserPointer(window);
	
	//TODO: Handle repeated keys?
	if (action == GLFW_REPEAT)
		return;
	
	HandleKeyEvent(window, currentInput, key, action == GLFW_PRESS);
}
void GlfwCharPressedCallback(GLFWwindow* window, u32 codepoint)
{
	OSX_PrintLine("Text Input: 0x%X", codepoint);
	AppInput_t* currentInput = (AppInput_t*)glfwGetWindowUserPointer(window);
	
	if (currentInput->textInputLength < ArrayCount(currentInput->textInput))
	{
		currentInput->textInput[currentInput->textInputLength] = (u8)codepoint;
		currentInput->textInputLength++;
	}
}
void GlfwCursorPosCallback(GLFWwindow* window, real64 mouseX, real64 mouseY)
{
	// OSX_PrintLine("Received GlfwCursorPosCallback: (%f, %f)", mouseX, mouseY);
	AppInput_t* currentInput = (AppInput_t*)glfwGetWindowUserPointer(window);
	
	currentInput->mousePos = NewVec2((r32)mouseX*2, (r32)mouseY*2);
	
	if (currentInput->buttons[MouseButton_Left].isDown)
	{
		r32 distance = Vec2Length(currentInput->mousePos - currentInput->mouseStartPos[MouseButton_Left]);
		if (distance > currentInput->mouseMaxDist[MouseButton_Left])
		{
			currentInput->mouseMaxDist[MouseButton_Left] = distance;
		}
	}
	if (currentInput->buttons[MouseButton_Right].isDown)
	{
		r32 distance = Vec2Length(currentInput->mousePos - currentInput->mouseStartPos[MouseButton_Right]);
		if (distance > currentInput->mouseMaxDist[MouseButton_Right])
		{
			currentInput->mouseMaxDist[MouseButton_Right] = distance;
		}
	}
	if (currentInput->buttons[MouseButton_Middle].isDown)
	{
		r32 distance = Vec2Length(currentInput->mousePos - currentInput->mouseStartPos[MouseButton_Middle]);
		if (distance > currentInput->mouseMaxDist[MouseButton_Middle])
		{
			currentInput->mouseMaxDist[MouseButton_Middle] = distance;
		}
	}
}
void GlfwMousePressCallback(GLFWwindow* window, i32 button, i32 action, i32 modifiers)
{
	// OSX_WriteLine("Received GlfwMousePressCallback");
	AppInput_t* currentInput = (AppInput_t*)glfwGetWindowUserPointer(window);
	
	//TODO: Handle repeated keys?
	if (action == GLFW_REPEAT)
		return;
	
	HandleMouseEvent(window, currentInput, button, action == GLFW_PRESS);
}
void GlfwMouseScrollCallback(GLFWwindow* window, real64 deltaX, real64 deltaY)
{
	// OSX_PrintLine("Received GlfwMouseScrollCallback: %f, %f", deltaX, deltaY);
	AppInput_t* currentInput = (AppInput_t*)glfwGetWindowUserPointer(window);
	
	currentInput->scrollValue += NewVec2((r32)deltaX, (r32)deltaY);
	currentInput->scrollDelta += NewVec2((r32)deltaX, (r32)deltaY);
}

