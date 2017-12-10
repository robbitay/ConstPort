/*
File:   plat_callbacks.cpp
Author: Taylor Robbins
Date:   06\06\2017
Description: 
	** Contains all the callback functions that we register with GLFW

#included from win32_main.cpp 
*/

void GlfwErrorCallback(i32 errorCode, const char* description)
{
	DEBUG_PrintLine("GlfwErrorCallback: %X \"%s\"", errorCode, description);
}
void GlfwWindowCloseCallback(GLFWwindow* window)
{
	DEBUG_WriteLine("Window Closing");
	
	//TODO: Should we dissallow the close in some scenarios?
	// glfwSetWindowShouldClose(window, false);
}
void GlfwWindowSizeCallback(GLFWwindow* window, i32 screenWidth, i32 screenHeight)
{
	DEBUG_PrintLine("Resized: %dx%d", screenWidth, screenHeight);
	
	PlatformInfo.screenSize = NewVec2i(screenWidth, screenHeight);
	PlatformInfo.windowResized = true;
}
void GlfwWindowMoveCallback(GLFWwindow* window, i32 posX, i32 posY)
{
	// DEBUG_PrintLine("Received GlfwWindowMoveCallback: (%d, %d)", posX, posY);
}
void GlfwWindowMinimizeCallback(GLFWwindow* window, i32 isMinimized)
{
	DEBUG_PrintLine("Window %s", isMinimized ? "Minimized" : "Restored");
	PlatformInfo.windowIsMinimized = (isMinimized > 0);
}
void GlfwWindowFocusCallback(GLFWwindow* window, i32 isFocused)
{
	DEBUG_PrintLine("Window %s focus!", isFocused ? "Gained" : "Lost");
	PlatformInfo.windowHasFocus = (isFocused > 0);
}
void GlfwKeyPressedCallback(GLFWwindow* window, i32 key, i32 scanCode, i32 action, i32 modifiers)
{
	// const char* actionStr = "Pressed";
	// if (action == GLFW_REPEAT) actionStr = "Repeated";
	// if (action == GLFW_RELEASE) actionStr = "Released";
	// DEBUG_PrintLine("KeyPress %d %d %s (%d)", key, scanCode, actionStr, modifiers);
	AppInput_t* currentInput = (AppInput_t*)glfwGetWindowUserPointer(window);
	
	HandleKeyEvent(window, currentInput, key, action);
	
	//Push enter, tab, and backspace characters onto the textInput
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		if (key == GLFW_KEY_ENTER || key == GLFW_KEY_TAB || key == GLFW_KEY_BACKSPACE)
		{
			char newChar = '?';
			if (key == GLFW_KEY_ENTER)     { newChar = '\n'; }
			if (key == GLFW_KEY_TAB)       { newChar = '\t'; }
			if (key == GLFW_KEY_BACKSPACE) { newChar = '\b'; }
			
			if (currentInput->textInputLength < ArrayCount(currentInput->textInput))
			{
				currentInput->textInput[currentInput->textInputLength] = newChar;
				currentInput->textInputLength++;
			}
		}
	}
}
void GlfwCharPressedCallback(GLFWwindow* window, u32 codepoint)
{
	// DEBUG_PrintLine("Text Input: 0x%X", codepoint);
	AppInput_t* currentInput = (AppInput_t*)glfwGetWindowUserPointer(window);
	
	if (currentInput->textInputLength < ArrayCount(currentInput->textInput))
	{
		currentInput->textInput[currentInput->textInputLength] = (u8)codepoint;
		currentInput->textInputLength++;
	}
}
void GlfwCursorPosCallback(GLFWwindow* window, real64 mouseX, real64 mouseY)
{
	// DEBUG_PrintLine("Received GlfwCursorPosCallback: (%f, %f)", mouseX, mouseY);
	AppInput_t* currentInput = (AppInput_t*)glfwGetWindowUserPointer(window);
	
	#if DOUBLE_MOUSE_POS
	currentInput->mousePos = NewVec2((r32)mouseX*2, (r32)mouseY*2);
	#else
	currentInput->mousePos = NewVec2((r32)mouseX, (r32)mouseY);
	#endif
	
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
	// const char* actionStr = "Pressed";
	// if (action == GLFW_REPEAT) actionStr = "Repeated";
	// if (action == GLFW_RELEASE) actionStr = "Released";
	// DEBUG_PrintLine("MousePress %d %s (%d)", button, actionStr, modifiers);
	AppInput_t* currentInput = (AppInput_t*)glfwGetWindowUserPointer(window);
	
	//TODO: Handle repeated keys?
	if (action == GLFW_REPEAT)
		return;
	
	HandleMouseEvent(window, currentInput, button, action == GLFW_PRESS);
}
void GlfwMouseScrollCallback(GLFWwindow* window, real64 deltaX, real64 deltaY)
{
	// DEBUG_PrintLine("Received GlfwMouseScrollCallback: %f, %f", deltaX, deltaY);
	AppInput_t* currentInput = (AppInput_t*)glfwGetWindowUserPointer(window);
	
	currentInput->scrollValue += NewVec2((r32)deltaX, (r32)deltaY);
	currentInput->scrollDelta += NewVec2((r32)deltaX, (r32)deltaY);
}
void GlfwCursorEnteredCallback(GLFWwindow* window, i32 entered)
{
	DEBUG_PrintLine("Mouse %s window", entered ? "entered" : "left");
	AppInput_t* currentInput = (AppInput_t*)glfwGetWindowUserPointer(window);
	currentInput->mouseInsideWindow = (entered > 0);
}

