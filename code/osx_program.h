/*
File:   osx_program.h
Author: Taylor Robbins
Date:   10\05\2017
*/

#ifndef _OSX_PROGRAM_H
#define _OSX_PROGRAM_H

#include <signal.h>

struct ProgramInstance_t
{
	bool isOpen;
	
	pid_t pid;
	int stdInHandle;
	int stdOutHandle;
};

typedef enum
{
	ProgramStatus_Unknown = 0,
	ProgramStatus_Running,
	ProgramStatus_Finished,
	ProgramStatus_Error,
} ProgramStatus_t;

#endif //  _OSX_PROGRAM_H
