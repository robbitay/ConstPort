/*
File:   win32_program.cpp
Author: Taylor Robbins
Date:   09\24\2017
Description: 
	** Handles starting and interacting with external programs. 

#included from win32_main.cpp
*/

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
	
	bool32 success = CreateProcess(NULL,
		(LPSTR)commandStr, // command line
		NULL,              // process security attributes
		NULL,              // primary thread security attributes
		true,              // handles are inherited
		0,                 // creation flags
		NULL,              // use parent's environment
		NULL,              // use parent's current directory
		&startupInfo,      // STARTUPINFO pointer
		&procInfo);        // receives PROCESS_INFORMATION
	
	if (!success)
	{
		Win32_WriteLine("CreateProcess call failed");
		return result;
	}
	else
	{
		CloseHandle(procInfo.hProcess);
		CloseHandle(procInfo.hThread);
		
		result.isOpen = true;
		return result;
	}
}

GetProgramStatus_DEFINITION(Win32_GetProgramStatus)
{
	return ProgramStatus_Running;
}

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
	
	program->isOpen = false;
}


