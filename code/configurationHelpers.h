/*
File:   configurationHelpers.h
Author: Taylor Robbins
Date:   08\13\2017
*/

/*
File:   configurationHelpers.h
Author: Taylor Robbins
Date:   08\06\2017
	** This file is included by both platform and application layers
*/

#ifndef _CONFIGURATION_HELPERS
#define _CONFIGURATION_HELPERS

#define JSMN_PARENT_LINKS
#include "jsmn.h"
#include "jsmn.c"

struct JsonData_t
{
	const char* data;
	jsmntok_t* tokens;
	i32 numTokens;
};

typedef enum
{
	ConfigError_None = 0,
	
	ConfigError_TokenNotFound,
	ConfigError_InvalidNumber,
	ConfigError_InvalidBoolean,
	ConfigError_ColorDoesntExist,
	ConfigError_WrongNumberOfArrayElements,
	ConfigError_NumberOutOfRange,
	ConfigError_ExpectedArray,
} ConfigError_t;

// +------------------------------------------------------------------+
// |                        Parsing Functions                         |
// +------------------------------------------------------------------+
bool IsHexString(const char* charPntr, i32 numChars)
{
	for (i32 cIndex = 0; cIndex < numChars; cIndex++)
	{
		if (charPntr[cIndex] != '0' &&
			charPntr[cIndex] != '1' &&
			charPntr[cIndex] != '2' &&
			charPntr[cIndex] != '3' &&
			charPntr[cIndex] != '4' &&
			charPntr[cIndex] != '5' &&
			charPntr[cIndex] != '6' &&
			charPntr[cIndex] != '7' &&
			charPntr[cIndex] != '8' &&
			charPntr[cIndex] != '9' &&
			charPntr[cIndex] != 'A' &&
			charPntr[cIndex] != 'B' &&
			charPntr[cIndex] != 'C' &&
			charPntr[cIndex] != 'D' &&
			charPntr[cIndex] != 'E' &&
			charPntr[cIndex] != 'F' &&
			charPntr[cIndex] != 'a' &&
			charPntr[cIndex] != 'b' &&
			charPntr[cIndex] != 'c' &&
			charPntr[cIndex] != 'd' &&
			charPntr[cIndex] != 'e' &&
			charPntr[cIndex] != 'f')
		{
			return false;
		}
	}

	return true;
}

u8 HexCharValue(char c)
{
	u8 result = 0x00;

	if (c >= '0' && c <= '9')
	{
		result = c - '0';
	}
	else if (c >= 'A' && c <= 'F')
	{
		result = (c - 'A') + 10;
	}
	else if (c >= 'a' && c <= 'f')
	{
		result = (c - 'a') + 10;
	}

	return result;
}

u8 ByteFromHexChars(const char* charPntr)
{
	return 16*HexCharValue(charPntr[0]) + HexCharValue(charPntr[1]);
}

u8* BytesFromMixedHexString(MemoryArena_t* arenaPntr, const char* strPntr, u32 strLength, u32* numBytesOut)
{
	u32 resultCount = 0;
	u8* result = nullptr;
	*numBytesOut = 0;
	
	//Run this code twice. Once to count the number of bytes and allocate space, then once to fill the space with data
	for (u8 round = 0; round < 2; round++)
	{
		resultCount = 0;
		u32 fromIndex = 0;
		u32 toIndex = 0;
		while (fromIndex < strLength)
		{
			char c = strPntr[fromIndex];
			char nextChar = '\0';
			if (fromIndex+1 < strLength) { nextChar = strPntr[fromIndex+1]; }
			
			if (IsCharClassHexChar(c) && IsCharClassHexChar(nextChar))
			{
				u8 value = ByteFromHexChars(&strPntr[fromIndex]);
				if (result != nullptr) { result[toIndex] = value; }
				toIndex++;
				fromIndex += 2;
			}
			else if (IsCharClassHexChar(c))
			{
				u8 value = HexCharValue(c);
				if (result != nullptr) { result[toIndex] = value; }
				toIndex++;
				fromIndex++;
			}
			else { fromIndex++; } //Skip characters that aren't hex representations
		}
		
		if (round == 0) //Sizing round
		{
			if (toIndex == 0) //No valid hex characters found
			{
				*numBytesOut = 0;
				return nullptr;
			}
			
			result = PushArray(arenaPntr, u8, toIndex);
			*numBytesOut = toIndex;
		}
	}
	
	return result;
}

ConfigError_t TryParseColor(const char* charPntr, i32 numChars, Color_t* colorOut)
{
	if ((numChars == 6 || numChars == 8) && IsHexString(charPntr, numChars))
	{
		colorOut->red   = ByteFromHexChars(&charPntr[0]);
		colorOut->green = ByteFromHexChars(&charPntr[2]);
		colorOut->blue  = ByteFromHexChars(&charPntr[4]);
		colorOut->alpha = 0xFF;
		if (numChars == 8)
		{
			colorOut->alpha = ByteFromHexChars(&charPntr[6]);
		}

		return ConfigError_None;
	}
	else
	{
		for (i32 colorIndex = 0; colorIndex < NUM_COLORS; colorIndex++)
		{
			u32 colorValue = GetColorByIndex(colorIndex);
			if (strncmp(GetColorName(colorValue), charPntr, numChars) == 0)
			{
				colorOut->value = colorValue;
				return ConfigError_None;
			}
		}
		
		return ConfigError_ColorDoesntExist;
	}
}

//Undoes escaped characters in jsonString in-place.
//Returns the number of characters the string shrunk by
i32 UnescapeJsonString(char* jsonString, i32 strLength)
{
	i32 writePos = 0;
	for (i32 cIndex = 0; cIndex < strLength; cIndex++)
	{
		Assert(writePos <= cIndex);
		
		char writeCharacter = jsonString[cIndex];
		if (jsonString[cIndex] == '\\' && cIndex+1 < strLength)
		{
			char nextChar = jsonString[cIndex+1];
			if (nextChar == 'b')
			{
				writeCharacter = '\b';
				cIndex++;
			}
			else if (nextChar == 'f')
			{
				writeCharacter = '\f';
				cIndex++;
			}
			else if (nextChar == 'n')
			{
				writeCharacter = '\n';
				cIndex++;
			}
			else if (nextChar == 'r')
			{
				writeCharacter = '\r';
				cIndex++;
			}
			else if (nextChar == 't')
			{
				writeCharacter = '\t';
				cIndex++;
			}
			else if (nextChar == '"')
			{
				writeCharacter = '\"';
				cIndex++;
			}
			else if (nextChar == '\\')
			{
				writeCharacter = '\\';
				cIndex++;
			}
			else
			{
				printf("Unknown JSON string escape sequence: \"\\%c\"", nextChar);
			}
		}
		
		jsonString[writePos] = writeCharacter;
		writePos += 1;
	}
	
	return strLength - writePos;
}

// +------------------------------------------------------------------+
// |                      JSON Helper Functions                       |
// +------------------------------------------------------------------+
#define TokenLength(tokenPntr) ((tokenPntr)->end - (tokenPntr)->start)
#define TokenIsNamed(jsonData, tokenPntr, compareString) (TokenLength(tokenPntr) == strlen(compareString) && strncmp(compareString, &(jsonData)->data[tokenPntr->start], TokenLength(tokenPntr)) == 0)
#define TryGetTokenAsNumber(jsonData, tokenPntr, valueOut) TryParseI32(&(jsonData)->data[(tokenPntr)->start], TokenLength(tokenPntr), valueOut)
#define TryGetTokenAsBoolean(jsonData, tokenPntr, valueOut) TryParseBool(&(jsonData)->data[(tokenPntr)->start], TokenLength(tokenPntr), valueOut)

i32 FindChildTokenByName(JsonData_t* jsonData, i32 parentIndex, const char* name)
{
	for (i32 tIndex = 0; tIndex < jsonData->numTokens; tIndex++)
	{
		jsmntok_t* token = &jsonData->tokens[tIndex];
		if (token->parent == parentIndex)
		{
			if (TokenIsNamed(jsonData, token, name))
			{
				return tIndex;
			}
		}
	}

	return -1;
}

u32 GetNumChildTokens(JsonData_t* jsonData, i32 parentIndex)
{
	u32 result = 0;

	for (i32 tIndex = 0; tIndex < jsonData->numTokens; tIndex++)
	{
		jsmntok_t* token = &jsonData->tokens[tIndex];
		if (token->parent == parentIndex)
		{
			result++;
		}
	}
	
	return result;
}

u32 GetNumChildObjects(JsonData_t* jsonData, i32 parentIndex)
{
	u32 result = 0;

	for (i32 tIndex = 0; tIndex < jsonData->numTokens; tIndex++)
	{
		jsmntok_t* token = &jsonData->tokens[tIndex];
		if (token->parent == parentIndex &&
			token->type == JSMN_OBJECT)
		{
			result++;
		}
	}
	
	return result;
}

i32 FindChildTokenByIndex(JsonData_t* jsonData, i32 parentIndex, i32 childIndex)
{
	i32 numItemsFound = 0;

	for (i32 tIndex = 0; tIndex < jsonData->numTokens; tIndex++)
	{
		jsmntok_t* token = &jsonData->tokens[tIndex];
		if (token->parent == parentIndex)
		{
			if (numItemsFound == childIndex)
			{
				return tIndex;
			}

			numItemsFound++;
		}
	}

	return -1;
}

i32 GetChildToken(JsonData_t* jsonData, i32 parentIndex)
{
	for (i32 tIndex = 0; tIndex < jsonData->numTokens; tIndex++)
	{
		jsmntok_t* token = &jsonData->tokens[tIndex];
		if (token->parent == parentIndex)
		{
			return tIndex;
		}
	}

	return -1;
}

//TODO: Add support for multiline comments
void SanatizeFileComments(char* fileData, u32 fileSize)
{
	bool insideString = false;
	bool insideComment = false;

	for (u32 cIndex = 0; cIndex < fileSize; cIndex++)
	{
		char c = fileData[cIndex];
		char lastChar = '\0';
		if (cIndex > 0) lastChar = fileData[cIndex-1];
		char nextChar = '\0';
		if (cIndex < fileSize-1) nextChar = fileData[cIndex+1];

		if (c == '\n' || c == '\r')
		{
			insideString = false;
			insideComment = false;
		}
		else if (insideComment)
		{
			//Replace characters inside comment with spaces
			fileData[cIndex] = ' ';
		}
		else if (c == '\"')
		{
			if (!insideString)
			{
				insideString = true;
			}
			else if (lastChar == '\\')
			{
				//Escaped quote, disregard
			}
			else
			{
				insideString = false;
			}
		}
		else if (c == '/' && nextChar == '/' && !insideString)
		{
			insideComment = true;
			fileData[cIndex] = ' ';
		}
	}
}

ConfigError_t TryGetColorFromJsonArray(JsonData_t* jsonData, i32 arrayTokenIndex, Color_t* colorOut)
{
	jsmntok_t* arrayTokenPntr = &jsonData->tokens[arrayTokenIndex];

	if (arrayTokenPntr->size != 3 && arrayTokenPntr->size != 4)
	{
		return ConfigError_WrongNumberOfArrayElements;
	}

	//Find all the child items
	i32 childTokenIndex[4] = {};
	childTokenIndex[0] = FindChildTokenByIndex(jsonData, arrayTokenIndex, 0);
	childTokenIndex[1] = FindChildTokenByIndex(jsonData, arrayTokenIndex, 1);
	childTokenIndex[2] = FindChildTokenByIndex(jsonData, arrayTokenIndex, 2);
	childTokenIndex[3] = FindChildTokenByIndex(jsonData, arrayTokenIndex, 3);

	if (childTokenIndex[0] == -1 || childTokenIndex[1] == -1 || childTokenIndex[2] == -1 ||
		(arrayTokenPntr->size == 4 && childTokenIndex[3] == -1))
	{
		return ConfigError_WrongNumberOfArrayElements;
	}

	//Make sure we can parse them as numbers
	//and that they are in range of a uint8
	i32 values[4] = {};

	if (!TryGetTokenAsNumber(jsonData, &jsonData->tokens[childTokenIndex[0]], &values[0]))
	{
		return ConfigError_InvalidNumber;
	}
	else if (values[0] < 0 || values[0] > 255)
	{
		return ConfigError_NumberOutOfRange;
	}
	else if (!TryGetTokenAsNumber(jsonData, &jsonData->tokens[childTokenIndex[1]], &values[1]))
	{
		return ConfigError_InvalidNumber;
	}
	else if (values[1] < 0 || values[1] > 255)
	{
		return ConfigError_NumberOutOfRange;
	}
	else if (!TryGetTokenAsNumber(jsonData, &jsonData->tokens[childTokenIndex[2]], &values[2]))
	{
		return ConfigError_InvalidNumber;
	}
	else if (values[2] < 0 || values[2] > 255)
	{
		return ConfigError_NumberOutOfRange;
	}
	else if (childTokenIndex[3] != -1 &&
		!TryGetTokenAsNumber(jsonData, &jsonData->tokens[childTokenIndex[3]], &values[3]))
	{
		return ConfigError_InvalidNumber;
	}
	else if (childTokenIndex[3] != -1 && (values[3] < 0 || values[3] > 255))
	{
		return ConfigError_NumberOutOfRange;
	}

	//We've parsed all the nubmer correctly. Cast them to uint8
	colorOut->red   = (u8)values[0];
	colorOut->green = (u8)values[1];
	colorOut->blue  = (u8)values[2];
	colorOut->alpha = 255;
	if (childTokenIndex[3] != -1)
	{
		colorOut->alpha = (u8)values[3];
	}

	return ConfigError_None;
}

ConfigError_t TryGetVec2iFromJsonArray(JsonData_t* jsonData, i32 arrayTokenIndex, Vec2i_t* vec2Out)
{
	jsmntok_t* arrayTokenPntr = &jsonData->tokens[arrayTokenIndex];

	if (arrayTokenPntr->size != 2)
	{
		return ConfigError_WrongNumberOfArrayElements;
	}

	//Find all the child items
	i32 childTokenIndex[2] = {};
	childTokenIndex[0] = FindChildTokenByIndex(jsonData, arrayTokenIndex, 0);
	childTokenIndex[1] = FindChildTokenByIndex(jsonData, arrayTokenIndex, 1);

	if (childTokenIndex[0] == -1 || childTokenIndex[1] == -1)
	{
		return ConfigError_WrongNumberOfArrayElements;
	}

	//Make sure we can parse them as numbers
	//and that they are in range of a uint8
	i32 values[2] = {};

	if (!TryGetTokenAsNumber(jsonData, &jsonData->tokens[childTokenIndex[0]], &values[0]))
	{
		return ConfigError_InvalidNumber;
	}
	else if (!TryGetTokenAsNumber(jsonData, &jsonData->tokens[childTokenIndex[1]], &values[1]))
	{
		return ConfigError_InvalidNumber;
	}

	//We've parsed all the nubmer correctly. Cast them to uint8
	vec2Out->x = values[0];
	vec2Out->y = values[1];

	return ConfigError_None;
}

ConfigError_t TryGetColorConfig(JsonData_t* jsonData, i32 parentObjectIndex, const char* tokenName, Color_t* valuePntr)
{
	i32 tokenIndex = FindChildTokenByName(jsonData, parentObjectIndex, tokenName);
	if (tokenIndex != -1)
	{
		jsmntok_t* tokenPntr = &jsonData->tokens[tokenIndex];
		i32 valueTokenIndex = GetChildToken(jsonData, tokenIndex);
		jsmntok_t* valueTokenPntr = &jsonData->tokens[valueTokenIndex];
		const char* valueStringPntr = &jsonData->data[valueTokenPntr->start];
		Color_t parsedValue;
		if (valueTokenPntr->type == JSMN_ARRAY)
		{
			ConfigError_t parseResult = TryGetColorFromJsonArray(jsonData, valueTokenIndex, &parsedValue);
			
			if (parseResult == ConfigError_None)
			{
				*(valuePntr) = parsedValue;
				return ConfigError_None;
			}
			else
			{
				return parseResult;
			}
		}
		else
		{
			ConfigError_t parseResult = TryParseColor(valueStringPntr, TokenLength(valueTokenPntr), &parsedValue);
			
			if (parseResult == ConfigError_None)
			{
				*(valuePntr) = parsedValue;
				return ConfigError_None;
			}
			else
			{
				return parseResult;
			}
		}
	}
	else
	{
		return ConfigError_TokenNotFound;
	}
}

ConfigError_t TryGetInt32Config(JsonData_t* jsonData, i32 parentObjectIndex, const char* tokenName, i32* valuePntr)
{
	i32 tokenIndex = FindChildTokenByName(jsonData, parentObjectIndex, tokenName);
	if (tokenIndex != -1)
	{
		jsmntok_t* tokenPntr = &jsonData->tokens[tokenIndex];
		i32 valueTokenIndex = GetChildToken(jsonData, tokenIndex);
		jsmntok_t* valueTokenPntr = &jsonData->tokens[valueTokenIndex];
		
		i32 parsedValue;
		if (TryGetTokenAsNumber(jsonData, valueTokenPntr, &parsedValue))
		{
			*(valuePntr) = parsedValue;
			return ConfigError_None;
		}
		else
		{
			return ConfigError_InvalidNumber;
		}
	}
	else
	{
		return ConfigError_TokenNotFound;
	}
}

ConfigError_t TryGetBoolConfig(JsonData_t* jsonData, i32 parentObjectIndex, const char* tokenName, bool* valuePntr)
{
	i32 tokenIndex = FindChildTokenByName(jsonData, parentObjectIndex, tokenName);
	if (tokenIndex != -1)
	{
		jsmntok_t* tokenPntr = &jsonData->tokens[tokenIndex];
		i32 valueTokenIndex = GetChildToken(jsonData, tokenIndex);
		jsmntok_t* valueTokenPntr = &jsonData->tokens[valueTokenIndex];
		
		bool parsedValue;
		if (TryGetTokenAsBoolean(jsonData, valueTokenPntr, &parsedValue))
		{
			*(valuePntr) = parsedValue;
			return ConfigError_None;
		}
		else
		{
			return ConfigError_InvalidBoolean;
		}
	}
	else
	{
		return ConfigError_TokenNotFound;
	}
}

ConfigError_t TryGetVec2iConfig(JsonData_t* jsonData, i32 parentObjectIndex, const char* tokenName, Vec2i_t* valuePntr)
{
	i32 tokenIndex = FindChildTokenByName(jsonData, parentObjectIndex, tokenName);
	if (tokenIndex != -1)
	{
		jsmntok_t* tokenPntr = &jsonData->tokens[tokenIndex];
		i32 valueTokenIndex = GetChildToken(jsonData, tokenIndex);
		jsmntok_t* valueTokenPntr = &jsonData->tokens[valueTokenIndex];
		const char* valueStringPntr = &jsonData->data[valueTokenPntr->start];
		Vec2i_t parsedValue;
		if (valueTokenPntr->type == JSMN_ARRAY)
		{
			ConfigError_t parseResult = TryGetVec2iFromJsonArray(jsonData, valueTokenIndex, &parsedValue);
			
			if (parseResult == ConfigError_None)
			{
				*(valuePntr) = parsedValue;
				return ConfigError_None;
			}
			else
			{
				return parseResult;
			}
		}
		else
		{
			return ConfigError_ExpectedArray;
		}
	}
	else
	{
		return ConfigError_TokenNotFound;
	}
}

ConfigError_t TryGetStringConfig(JsonData_t* jsonData, i32 parentObjectIndex, const char* tokenName, const char** valuePntr, MemoryArena_t* memArena)
{
	i32 tokenIndex = FindChildTokenByName(jsonData, parentObjectIndex, tokenName);
	if (tokenIndex != -1)
	{
		jsmntok_t* tokenPntr = &jsonData->tokens[tokenIndex];
		i32 valueTokenIndex = GetChildToken(jsonData, tokenIndex);
		jsmntok_t* valueTokenPntr = &jsonData->tokens[valueTokenIndex];
		const char* valueStringPntr = &jsonData->data[valueTokenPntr->start];
		u32 valueStringLength = TokenLength(valueTokenPntr);
		
		char* newSpace = PushArray(memArena, char, valueStringLength+1);
		memcpy(newSpace, valueStringPntr, valueStringLength);
		valueStringLength -= UnescapeJsonString(newSpace, valueStringLength);
		newSpace[valueStringLength] = '\0';
		
		*(valuePntr) = newSpace;
		return ConfigError_None;
	}
	else
	{
		return ConfigError_TokenNotFound;
	}
}

#endif // _CONFIGURATION_HELPERS