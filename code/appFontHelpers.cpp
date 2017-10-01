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

inline v2 MeasureString(const Font_t* font, const char* string, u32 numChars)
{
	v2 currentPos = Vec2_Zero;
	for (u32 cIndex = 0; cIndex < numChars; cIndex++)
	{
		if (string[cIndex] == '\t')
		{
			u32 spaceIndex = GetFontCharIndex(font, ' ');
			currentPos.x += font->chars[spaceIndex].advanceX * GC->tabWidth;
		}
		else if (string[cIndex] == '\r' || 
			string[cIndex] == 0x01 || string[cIndex] == 0x02 || string[cIndex] == 0x03 || string[cIndex] == 0x04 || string[cIndex] == 0x05)
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

inline v2 MeasureString(const Font_t* font, const char* nullTermString)
{
	return MeasureString(font, nullTermString, (u32)strlen(nullTermString));
}

//TODO: Handle this more correctly with line wrapping
inline i32 GetStringIndexForLocation(const Font_t* font, const char* nullTermString, v2 relativePos)
{
	i32 result = 0;
	
	if (relativePos.y > MeasureString(font, nullTermString).y + GC->lineSpacing)
	{
		result = (i32)strlen(nullTermString);
		return result;
	}
	
	v2 lastStringSize = Vec2_Zero;
	for (i32 cIndex = 0; nullTermString[cIndex] != '\0'; cIndex++)
	{
		v2 stringSize = MeasureString(font, nullTermString, cIndex+1);
		if (stringSize.x > relativePos.x || nullTermString[cIndex+1] == '\0')
		{
			if (Abs32(relativePos.x - lastStringSize.x) < Abs32(relativePos.x - stringSize.x))
			{
				result = cIndex;
			}
			else
			{
				result = cIndex+1;
			}
			
			break;
		}
		
		lastStringSize = stringSize;
	}
	
	return result;
}

u32 FindNextFormatChunk(const Font_t* font, const char* string, u32 numCharacters, r32 maxWidth, bool preserveWords)
{
	r32 textWidth = 0;
	bool doBackwardsSearch = false;
	u32 cIndex;
	for (cIndex = 0; cIndex < numCharacters; cIndex++)
	{
		if (string[cIndex] == '\r' || string[cIndex] == '\n')
		{
			break;
		}
		else
		{
			r32 nextCharWidth = MeasureString(font, &string[cIndex], 1).x;
			
			if (textWidth + nextCharWidth > maxWidth)
			{
				doBackwardsSearch = preserveWords;
				break;
			}
			textWidth += nextCharWidth;
		}
	}
	
	if (doBackwardsSearch)
	{
		u32 searchStartIndex = cIndex;
		u32 wordBoundIndex = 0;
		bool foundSpace = false;
		for (cIndex = searchStartIndex; cIndex > 0; cIndex--)
		{
			char c = string[cIndex-1];
			if (IsCharClassWhitespace(c))
			{
				foundSpace = true;
				break;
			}
			else if (wordBoundIndex == 0 && IsCharClassAlphaNumeric(c) == false)
			{
				wordBoundIndex = cIndex;
			}
		}
		
		if (foundSpace)
		{
			return cIndex;
		}
		else if (wordBoundIndex != 0)
		{
			return wordBoundIndex-1;
		}
		else
		{
			return searchStartIndex;
		}
	}
	else
	{
		return cIndex;
	}
}

v2 MeasureFormattedString(const Font_t* font, const char* string, u32 numCharacters, r32 maxWidth, bool preserveWords)
{
	u32 cIndex = 0;
	
	r32 maxChunkWidth = 0;
	u32 numLines = 0;
	while (cIndex < numCharacters)
	{
		u32 numChars = FindNextFormatChunk(font, &string[cIndex], numCharacters - cIndex, maxWidth, preserveWords);
		if (numChars == 0) { numChars = 1; }
		
		while (numChars > 1 && IsCharClassWhitespace(string[cIndex + numChars-1]))
		{
			numChars--;
		}
		
		r32 chunkWidth = MeasureString(font, &string[cIndex], numChars).x;
		if (chunkWidth > maxChunkWidth) { maxChunkWidth = chunkWidth; }
		
		if (cIndex+numChars < numCharacters && string[cIndex+numChars] == '\r')
		{
			numChars++;
		}
		if (cIndex+numChars < numCharacters && string[cIndex+numChars] == '\n')
		{
			numChars++;
		}
		while (cIndex+numChars < numCharacters && string[cIndex+numChars] == ' ')
		{
			numChars++;
		}
		
		numLines++;
		cIndex += numChars;
	}
	
	return NewVec2(maxChunkWidth, numLines * font->lineHeight);
}

v2 MeasureFormattedString(const Font_t* font, const char* nullTermString, r32 maxWidth, bool preserveWords)
{
	u32 numCharacters = (u32)strlen(nullTermString);
	return MeasureFormattedString(font, nullTermString, numCharacters, maxWidth, preserveWords);
}
