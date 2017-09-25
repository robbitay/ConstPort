/*
File:   win32_program.h
Author: Taylor Robbins
Date:   09\24\2017
*/

#ifndef _WIN_32_PROGRAM_H
#define _WIN_32_PROGRAM_H

struct ProgramInstance_t
{
	bool isOpen;
	HANDLE stdHandleInRead;
	HANDLE stdHandleInWrite;
	HANDLE stdHandleOutRead;
	HANDLE stdHandleOutWrite;
};

typedef enum
{
	ProgramStatus_Unknown = 0,
	ProgramStatus_Running,
	ProgramStatus_Finished,
	ProgramStatus_Error,
} ProgramStatus_t;


#define StartProgramInstance_DEFINITION(functionName) ProgramInstance_t functionName(const char* commandStr)
typedef StartProgramInstance_DEFINITION(StartProgramInstance_f);

#define GetProgramStatus_DEFINITION(functionName) ProgramStatus_t functionName(const ProgramInstance_t* program)
typedef GetProgramStatus_DEFINITION(GetProgramStatus_f);

#define ReadProgramOutput_DEFINITION(functionName) u32 functionName(const ProgramInstance_t* program, char* outputBuffer, u32 outputBufferSize)
typedef ReadProgramOutput_DEFINITION(ReadProgramOutput_f);

#define CloseProgramInstance_DEFINITION(functionName) void functionName(ProgramInstance_t* program)
typedef CloseProgramInstance_DEFINITION(CloseProgramInstance_f);

#endif //  _WIN_32_PROGRAM_H
