/*
File:   appFontHelpers.cpp
Author: Taylor Robbins
Date:   06\13\2017
Description: 
	** Includes a random collection of functions that are useful 
	** when working with fonts

#included from app.cpp 
*/

inline u32 GetFontCharIndex(const Font_t* font, char character)
{
	u32 cIndex = 0;
	if (character >= font->firstChar && character - font->firstChar < font->numChars)
	{
		cIndex = character - font->firstChar;
	}
	else
	{
		cIndex = 127 - font->firstChar;
	}
	
	return cIndex;
}

inline v2 MeasureString(const Font_t* font, const char* string)
{
	v2 currentPos = Vec2_Zero;
	for (i32 cIndex = 0; cIndex < string[cIndex] != '\0'; cIndex++)
	{
		if (string[cIndex] == '\t')
		{
			u32 spaceIndex = GetFontCharIndex(font, ' ');
			currentPos.x += font->chars[spaceIndex].advanceX * TAB_WIDTH;
		}
		else if (string[cIndex] == '\r')
		{
			//Don't do anything
		}
		else
		{
			u32 charIndex = GetFontCharIndex(font, string[cIndex]);
			const FontCharInfo_t* charInfo = &font->chars[charIndex];
			currentPos.x += charInfo->advanceX;
		}
	}
	
	return NewVec2(currentPos.x, font->lineHeight);
}

inline v2 MeasureLine(const Font_t* font, const Line_t* line)
{
	return MeasureString(font, line->chars);
}