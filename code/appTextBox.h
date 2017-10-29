/*
File:   appTextBox.h
Author: Taylor Robbins
Date:   10\29\2017
*/

#ifndef _APP_TEXT_BOX_H
#define _APP_TEXT_BOX_H

struct TextBox_t
{
	u32 numChars;
	char* chars;
	
	u32 cursorBegin;
	u32 cursorEnd;
	
	rec drawRec;
	r32 drawOffset;
	r32 leftPadding;
	const Font_t* font;
	u32 maxNumChars;
	
	union
	{
		DynamicColor_t array[6];
		struct
		{
			DynamicColor_t background1;
			DynamicColor_t background2;
			DynamicColor_t text;
			DynamicColor_t border;
			DynamicColor_t cursor1;
			DynamicColor_t cursor2;
			DynamicColor_t selection1;
			DynamicColor_t selection2;
		};
	} colors;
};

#endif //  _APP_TEXT_BOX_H
