/*
File:   win32_threading.cpp
Author: Taylor Robbins
Date:   02\09\2018
Description: 
	** Holds the platform layer functions that provide the application with an
	** interface to start, pause, stop, and read from system threads.
*/

// +==============================+
// |      Win32_StartThread       |
// +==============================+
// AppThread_t StartThread(ThreadFunction_f* threadFunction, void* threadInput, ThreadCallback_f* threadCallback)
StartThread_DEFINITION(Win32_StartThread)
{
	AppThread_t result = {};
	
	result.threadFunction = threadFunction;
	result.threadCallback = threadCallback;
	result.threadInput = threadInput;
	
	result.handle = CreateThread(
		NULL, //Thread Security Attributes
		0, //Thread Stack Size
		threadFunction, //The function to run
		threadInput, //input variable
		NULL, //Creation flags
		&result.id
	);
	
	if (result.handle != NULL)
	{
		result.started = true;
	}
	else
	{
		Win32_PrintLine("Failed to create Win32 thread");
	}
	
	return result;
}

// +==============================+
// |      Win32_CloseThread       |
// +==============================+
// void CloseThread(AppThread_t* thread, bool doCallback)
CloseThread_DEFINITION(Win32_CloseThread)
{
	Assert(thread != nullptr);
	
	if (thread->handle != NULL)
	{
		DWORD exitCode = 0; //TODO: Some other exit code? Have the application pass an exit code?
		BOOL terminateSuccess = TerminateThread(thread->handle, exitCode);
		if (!terminateSuccess)
		{
			Win32_WriteLine("Failed to terminate Win32 thread!");
			//TODO: Use GetLastError for more information
		}
		
		if (doCallback && thread->threadCallback != nullptr)
		{
			thread->threadCallback(thread->threadInput);
		}
	}
	
	ClearPointer(thread);
}

// +==============================+
// |    Win32_GetThreadStatus     |
// +==============================+
// AppThreadStatus_t GetThreadStatus(AppThread_t* thread)
GetThreadStatus_DEFINITION(Win32_GetThreadStatus)
{
	Assert(thread != nullptr);
	Assert(thread->handle != NULL);
	
	AppThreadStatus_t result = AppThreadStatus_Unknown;
	
	DWORD signalResult = WaitForSingleObject(thread->handle, 0);
	if (signalResult == WAIT_OBJECT_0)
	{
		DWORD exitCode = 0;
		if (GetExitCodeThread(thread->handle, &exitCode))
		{
			if (exitCode == 0)
			{
				result = AppThreadStatus_Finished;
			}
			else
			{
				result = AppThreadStatus_Error;
			}
		}
		else
		{
			result = AppThreadStatus_Finished;
		}
	}
	else
	{
		result = AppThreadStatus_Runnning;
	}
	
	return result;
}

