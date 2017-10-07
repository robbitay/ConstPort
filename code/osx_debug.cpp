/*
File:   osx_debug.cpp
Author: Taylor Robbins
Date:   10\07\2017
Description: 
	** Holds the functions responsible for ouptutting debug data to the standard output
*/

#define DEBUG_PRINT_BUFFER_SIZE 2048

// +==============================+
// |          OSX_Write           |
// +==============================+
DebugWrite_DEFINITION(OSX_Write)
{
	printf("%s", string);
}

// +==============================+
// |        OSX_WriteLine         |
// +==============================+
DebugWriteLine_DEFINITION(OSX_WriteLine)
{
	printf("%s", message);
	printf("\n");
}

// +==============================+
// |          OSX_Print           |
// +==============================+
DebugPrint_DEFINITION(OSX_Print)
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

// +==============================+
// |        OSX_PrintLine         |
// +==============================+
DebugPrintLine_DEFINITION(OSX_PrintLine)
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

