/*
File:   win32_program.cpp
Author: Taylor Robbins
Date:   09\24\2017
Description: 
	** Handles starting and interacting with external programs. 

#included from win32_main.cpp
*/

// +==============================+
// |  Win32_StartProgramInstance  |
// +==============================+
// ProgramInstance_t StartProgramInstance(const char* commandStr)
StartProgramInstance_DEFINITION(Win32_StartProgramInstance)
{
	ProgramInstance_t result = {};
	
	SECURITY_ATTRIBUTES secAttribs = {};
	secAttribs.nLength = sizeof(SECURITY_ATTRIBUTES);
	secAttribs.bInheritHandle = true;
	secAttribs.lpSecurityDescriptor = NULL;
	
	if (!CreatePipe(&result.stdHandleOutRead, &result.stdHandleOutWrite, &secAttribs, 0))
	{
		Win32_WriteLine("CreatePipe call for Output failed");
		return result;
	}
	if (!SetHandleInformation(result.stdHandleOutRead, HANDLE_FLAG_INHERIT, 0))
	{
		Win32_WriteLine("SetHandleInformation call for Output failed");
		return result;
	}
	if (!CreatePipe(&result.stdHandleInRead, &result.stdHandleInWrite, &secAttribs, 0))
	{
		Win32_WriteLine("CreatePipe call for Input failed");
		return result;
	}
	if (!SetHandleInformation(result.stdHandleInWrite, HANDLE_FLAG_INHERIT, 0))
	{
		Win32_WriteLine("SetHandleInformation call for Input failed");
		return result;
	}
	
	PROCESS_INFORMATION procInfo = {};
	
	STARTUPINFO startupInfo = {};
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.hStdError = result.stdHandleOutWrite;
	startupInfo.hStdOutput = result.stdHandleOutWrite;
	startupInfo.hStdInput = result.stdHandleInRead;
	startupInfo.dwFlags |= STARTF_USESTDHANDLES;
	startupInfo.dwFlags |= STARTF_USESHOWWINDOW;
	startupInfo.wShowWindow = SW_SHOWMINNOACTIVE;
	
	bool32 createProcessResult = CreateProcess(NULL,
		(LPSTR)commandStr, // command line
		NULL,              // process security attributes
		NULL,              // primary thread security attributes
		true,              // handles are inherited
		0,                 // creation flags
		NULL,              // use parent's environment
		NULL,              // use parent's current directory
		&startupInfo,      // STARTUPINFO pointer
		&procInfo);        // receives PROCESS_INFORMATION
	
	if (!createProcessResult)
	{
		Win32_WriteLine("CreateProcess call failed");
		return result;
	}
	else
	{
		result.isOpen = true;
		result.processHandle = procInfo.hProcess;
		result.threadHandle = procInfo.hThread;
		return result;
	}
}

// +==============================+
// |    Win32_GetProgramStatus    |
// +==============================+
// ProgramStatus_t GetProgramStatus(const ProgramInstance_t* program)
GetProgramStatus_DEFINITION(Win32_GetProgramStatus)
{
	DWORD exitCode;
	if (GetExitCodeProcess(program->processHandle, &exitCode))
	{
		switch (exitCode)
		{
			case STILL_ACTIVE: return ProgramStatus_Running;
			case ERROR_SUCCESS: return ProgramStatus_Finished;
			default:
			{
				Win32_PrintLine("Unknown process exit code %02X", exitCode);
				return ProgramStatus_Error;
			}
		};
	}
	else
	{
		return ProgramStatus_Unknown;
	}
}

// +==============================+
// |   Win32_ReadProgramOutput    |
// +==============================+
// u32 ReadProgramOutput(const ProgramInstance_t* program, char* outputBuffer, u32 outputBufferSize)
ReadProgramOutput_DEFINITION(Win32_ReadProgramOutput)
{
	Assert(program != nullptr);
	Assert(program->isOpen);
	Assert(program->stdHandleOutRead != NULL);
	
	DWORD numBytesAvailable = 0;
	DWORD numBytesLeftThisMessage = 0;
	DWORD numBytesRead = 0;
	if (PeekNamedPipe(program->stdHandleOutRead, nullptr, outputBufferSize, &numBytesRead, &numBytesAvailable, &numBytesLeftThisMessage) != 0 &&
		numBytesAvailable > 0)
	{
		if (numBytesAvailable > outputBufferSize) { numBytesAvailable = outputBufferSize; }
		if (ReadFile(program->stdHandleOutRead, outputBuffer, numBytesAvailable, &numBytesRead, NULL))
		{
			return (u32)numBytesRead;
		}
	}
	
	return 0;
}

// +==============================+
// |   Win32_WriteProgramInput    |
// +==============================+
// u32 WriteProgramInput(const ProgramInstance_t* program, const char* dataPntr, u32 numBytes)
WriteProgramInput_DEFINITION(Win32_WriteProgramInput)
{
	Assert(program != nullptr);
	Assert(program->isOpen);
	Assert(program->stdHandleInWrite != NULL);
	
	DWORD numBytesWritten;
	if (WriteFile(program->stdHandleInWrite, dataPntr, numBytes, &numBytesWritten, NULL))
	{
		return numBytesWritten;
	}
	else
	{
		return 0;
	}
}

// +==============================+
// |  Win32_CloseProgramInstance  |
// +==============================+
// void CloseProgramInstance(ProgramInstance_t* program)
CloseProgramInstance_DEFINITION(Win32_CloseProgramInstance)
{
	if (program == nullptr) return;
	
	if (program->stdHandleOutRead != NULL)
	{
		CloseHandle(program->stdHandleOutRead);
		program->stdHandleOutRead = NULL;
	}
	if (program->stdHandleOutWrite != NULL)
	{
		CloseHandle(program->stdHandleOutWrite);
		program->stdHandleOutWrite = NULL;
	}
	if (program->stdHandleInRead != NULL)
	{
		CloseHandle(program->stdHandleInRead);
		program->stdHandleInRead = NULL;
	}
	if (program->stdHandleInWrite != NULL)
	{
		CloseHandle(program->stdHandleInWrite);
		program->stdHandleInWrite = NULL;
	}
	if (program->processHandle != NULL)
	{
		DWORD exitCode;
		if (GetExitCodeProcess(program->processHandle, &exitCode) &&
			exitCode == STILL_ACTIVE)
		{
			Win32_WriteLine("Force closing program instance");
			TerminateProcess(program->processHandle, 0); //TODO: Should this 0 be a real exit code?
		}
		
		CloseHandle(program->processHandle);
		program->processHandle = NULL;
	}
    if (program->threadHandle != NULL)
    {
		CloseHandle(program->threadHandle);
		program->threadHandle = NULL;
	}
	
	program->isOpen = false;
}

// +==============================+
// |    Win32_CreateNewWindow     |
// +==============================+
// void CreateNewWindow()
CreateNewWindow_DEFINITION(Win32_CreateNewWindow)
{
	char moduleName[256] = {};
	DWORD moduleNameLength = GetModuleFileNameA(0, moduleName, ArrayCount(moduleName));
	
	PROCESS_INFORMATION procInfo = {};
	
	STARTUPINFO startupInfo = {};
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.dwFlags |= STARTF_USESHOWWINDOW;
	startupInfo.wShowWindow = SW_SHOWMINNOACTIVE;
	
	bool32 createProcessResult = CreateProcess(NULL,
		(LPSTR)moduleName, // command line
		NULL,              // process security attributes
		NULL,              // primary thread security attributes
		true,              // handles are inherited
		0,                 // creation flags
		NULL,              // use parent's environment
		NULL,              // use parent's current directory
		&startupInfo,      // STARTUPINFO pointer
		&procInfo);        // receives PROCESS_INFORMATION
	
	if (createProcessResult)
	{
		Win32_WriteLine("Created new window!");
	}
	else
	{
		Win32_WriteLine("Failed to create new window");
	}
}

void ReplaceCharInString(char* strPntr, char oldChar, char newChar)
{
	for (u32 cIndex = 0; strPntr[cIndex] != '\0'; cIndex++)
	{
		if (strPntr[cIndex] == oldChar)
		{
			strPntr[cIndex] = newChar;
		}
	}
}

// +==============================+
// |    Win32_GetAbsolutePath     |
// +==============================+
// char* GetAbsolutePath(MemoryArena_t* arenaPntr, const char* relativePath)
GetAbsolutePath_DEFINITION(Win32_GetAbsolutePath)
{
	char workingDir[256] = {};
	u32 workingDirLength = GetRunningDirectory(workingDir, ArrayCount(workingDir));
	
	char* result = ArenaPrint(arenaPntr, "%s%s", workingDir, relativePath);
	ReplaceCharInString(result, '/', '\\');
	
	return result;
}
