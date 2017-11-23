/*
File:   osx_program.cpp
Author: Taylor Robbins
Date:   10\10\2017
Description: 
	** Holds the OSX program interface functions for starting,
	** talking to, and stopping an external program instance 
*/

// +==============================+
// |   OSX_CloseProgramInstance   |
// +==============================+
// void CloseProgramInstancePntr(ProgramInstance_t* program)
CloseProgramInstance_DEFINITION(OSX_CloseProgramInstance)
{
	Assert(program != nullptr);
	
	if (program->isOpen == false) { return; }
	
	if (program->stdInHandle != -1)
	{
		close(program->stdInHandle);
	}
	if (program->stdOutHandle != -1)
	{
		close(program->stdOutHandle);
	}
	if (program->pid > 0)
	{
		kill(program->pid, SIGKILL);
	}
	
	ClearPointer(program);
}

// +==============================+
// |   OSX_StartProgramInstance   |
// +==============================+
// ProgramInstance_t StartProgramInstancePntr(const char* commandStr)
StartProgramInstance_DEFINITION(OSX_StartProgramInstance)
{
	ProgramInstance_t result = {};
	
	OSX_PrintLine("Running \"%s\"", commandStr);
	result.pid = popen2(commandStr, &result.stdInHandle, &result.stdOutHandle);
	
	if (result.pid > 0)
	{
		OSX_PrintLine("Program started, PID = %d", result.pid);
		fcntl(result.stdInHandle,  F_SETFL, FNDELAY);
		fcntl(result.stdOutHandle, F_SETFL, FNDELAY);
		result.isOpen = true;
	}
	else
	{
		OSX_PrintLine("Failed to start program: errno=%s", GetErrnoName(errno));
	}
	
	return result;
}

// +==============================+
// |     OSX_GetProgramStatus     |
// +==============================+
// ProgramStatus_t GetProgramStatusPntr(const ProgramInstance_t* program)
GetProgramStatus_DEFINITION(OSX_GetProgramStatus)
{
	Assert(program != nullptr);
	
	//TODO: Ask the OS about it's status
	
	if (program->isOpen || program->pid <= 0)
	{
		if (kill(program->pid, 0) != 0) //TODO: Fix me?
		{
			return ProgramStatus_Finished;
		}
		else
		{
			return ProgramStatus_Running;
		}
	}
	else
	{
		return ProgramStatus_Finished;
	}
}

// +==============================+
// |    OSX_ReadProgramOutput     |
// +==============================+
// u32 ReadProgramOutputPntr(const ProgramInstance_t* program, char* outputBuffer, u32 outputBufferSize)
ReadProgramOutput_DEFINITION(OSX_ReadProgramOutput)
{
	Assert(program != nullptr);
	Assert(outputBuffer != nullptr);
	Assert(program->isOpen == true);
	Assert(program->pid > 0);
	Assert(program->stdOutHandle != -1);
	
	// printf("Reading %u bytes from file %d\n", 1, program->stdOutHandle);
	i64 readResult = read(program->stdOutHandle, outputBuffer, outputBufferSize);
	
	if (readResult == -1)
	{
		switch (errno)
		{
			case EAGAIN: return 0;
			default: printf("read call failed: %s\n", GetErrnoName(errno)); return -1;
		};
	}
	
	return readResult;
}

// +==============================+
// |    OSX_WriteProgramInput     |
// +==============================+
// u32 WriteProgramInputPntr(const ProgramInstance_t* program, const char* dataPntr, u32 numBytes)
WriteProgramInput_DEFINITION(OSX_WriteProgramInput)
{
	Assert(program != nullptr);
	Assert(program->isOpen);
	Assert(program->pid > 0);
	Assert(program->stdInHandle != -1);
	
	i64 writeResult = write(program->stdInHandle, dataPntr, numBytes);
	
	if (writeResult == -1)
	{
		printf("write call failed %s\n", GetErrnoName(errno));
	}
	
	return (u32)writeResult;
}

// +==============================+
// |     OSX_CreateNewWindow      |
// +==============================+
// void OSX_CreateNewWindow()
CreateNewWindow_DEFINITION(OSX_CreateNewWindow)
{
	char tempBuffer[256];
	snprintf(tempBuffer, ArrayCount(tempBuffer)-1, "open -n \"%s/../../../ConstPort.app\"", WorkingDirectory);
	
	pid_t pid = popen2(tempBuffer, nullptr, nullptr);
	
	if (pid > 0)
	{
		OSX_PrintLine("New window created, PID = %d", pid);
	}
	else
	{
		OSX_PrintLine("Failed to create new window: errno=%s", GetErrnoName(errno));
	}
}

// +==============================+
// |     OSX_GetAbsolutePath      |
// +==============================+
// char* OSX_GetAbsolutePath(MemoryArena_t* arenaPntr, const char* relativePath)
GetAbsolutePath_DEFINITION(OSX_GetAbsolutePath)
{
	//TODO: Do better at detecting absolute paths
	if (relativePath[0] != '/')
	{
		return ArenaPrint(arenaPntr, "%s/%s", WorkingDirectory, relativePath);
	}
	else
	{
		return ArenaString(arenaPntr, NtStr(relativePath));
	}
	return nullptr;
}
