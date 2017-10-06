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
	HANDLE processHandle;
	HANDLE threadHandle;
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

#endif //  _WIN_32_PROGRAM_H
