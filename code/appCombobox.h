/*
File:   appCombobox.h
Author: Taylor Robbins
Date:   12\03\2017
*/

#ifndef _APP_COMBOBOX_H
#define _APP_COMBOBOX_H

struct Combobox_t
{
	i32 selectedIndex;
	MemoryArena_t* allocArena;
	BoundedStrList_t options;
	
	const Font_t* font;
	rec drawRec;
	rec arrowsRec;
};

#endif //  _APP_COMBOBOX_H
