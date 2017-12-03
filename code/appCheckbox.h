/*
File:   appCheckbox.h
Author: Taylor Robbins
Date:   12\03\2017
*/

#ifndef _APP_CHECKBOX_H
#define _APP_CHECKBOX_H

struct Checkbox_t
{
	rec drawRec;
	Color_t activeColor;
	Alignment_t labelAlignment;
	
	MemoryArena_t* allocArena;
	char* label;
	
	bool enabled;
	bool checked;
	u64  changeTime;
	bool mouseHasLeft;
};

#endif //  _APP_CHECKBOX_H
