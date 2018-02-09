/*
File:   win32_threading.h
Author: Taylor Robbins
Date:   02\09\2018
*/

#ifndef _WIN_32_THREADING_H
#define _WIN_32_THREADING_H

#define ThreadFunction(functionName) DWORD functionName(void* threadInput)
typedef ThreadFunction(ThreadFunction_f);

#define ThreadCallback(functionName) void functionName(void* threadInput)
typedef ThreadCallback(ThreadCallback_f);

struct AppThread_t
{
	bool started;
	ThreadFunction_f* threadFunction;
	ThreadCallback_f* threadCallback;
	void* threadInput;
	
	//Win32 specific
	DWORD id;
	HANDLE handle;
};

typedef enum
{
	AppThreadStatus_Unknown = 0,
	AppThreadStatus_Runnning,
	AppThreadStatus_Finished,
	AppThreadStatus_Error,
} AppThreadStatus_t;

#define StartThread_DEFINITION(functionName) AppThread_t functionName(ThreadFunction_f* threadFunction, void* threadInput, ThreadCallback_f* threadCallback)
typedef StartThread_DEFINITION(StartThread_f);

#define CloseThread_DEFINITION(functionName) void functionName(AppThread_t* thread, bool doCallback)
typedef CloseThread_DEFINITION(CloseThread_f);

#define GetThreadStatus_DEFINITION(functionName) AppThreadStatus_t functionName(AppThread_t* thread)
typedef GetThreadStatus_DEFINITION(GetThreadStatus_f);

#endif //  _WIN_32_THREADING_H
