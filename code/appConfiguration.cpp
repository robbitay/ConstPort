/*
File:   appConfiguration.cpp
Author: Taylor Robbins
Date:   08\02\2017
Description: 
	** Handles loading and saving configuration TOML files
	** for various aspects of the application

#included from app.cpp
*/

//+================================================================+
//|                     JSON Helper Functions                      |
//+================================================================+
#define TokenLength(tokenPntr) ((tokenPntr)->end - (tokenPntr)->start)
#define TokenIsNamed(fileData, tokenPntr, compareString) (strncmp(compareString, &fileData[tokenPntr->start], TokenLength(tokenPntr)) == 0)
#define TryGetTokenAsNumber(fileData, tokenPntr, valueOut) TryParseInt32(&(fileData)[(tokenPntr)->start], TokenLength(tokenPntr), valueOut)

i32 FindChildTokenByName(const char* fileData, jsmntok_t* jsonTokens, i32 numTokens, i32 parentIndex, const char* name)
{
	for (i32 tIndex = 0; tIndex < numTokens; tIndex++)
	{
		jsmntok_t* token = &jsonTokens[tIndex];
		if (token->parent == parentIndex)
		{
			// DEBUG_PrintLine("Checking \"%.*s\"", TokenLength(token), &fileData[token->start]);
			if (TokenIsNamed(fileData, token, name))
			{
				return tIndex;
			}
		}
	}
	
	return -1;
}

i32 FindChildTokenByIndex(jsmntok_t* jsonTokens, i32 numTokens, i32 parentIndex, i32 childIndex)
{
	i32 numItemsFound = 0;
	
	for (i32 tIndex = 0; tIndex < numTokens; tIndex++)
	{
		jsmntok_t* token = &jsonTokens[tIndex];
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

i32 GetChildToken(jsmntok_t* jsonTokens, i32 numTokens, i32 parentIndex)
{
	for (i32 tIndex = 0; tIndex < numTokens; tIndex++)
	{
		jsmntok_t* token = &jsonTokens[tIndex];
		if (token->parent == parentIndex)
		{
			return tIndex;
		}
	}
	
	return -1;
}




//+================================================================+
//|                        Global Functions                        |
//+================================================================+
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

bool TryParseColor(const char* charPntr, i32 numChars, Color_t* colorOut)
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
		
		return true;
	}
	else
	{
		for (i32 colorIndex = 0; colorIndex < NUM_COLORS; colorIndex++)
		{
			u32 colorValue = GetColorByIndex(colorIndex);
			if (strncmp(GetColorName(colorValue), charPntr, numChars) == 0)
			{
				colorOut->value = colorValue;
				return true;
			}
		}
	}
	
	return false;
}

bool TryGetColorFromJsonArray(const char* fileData, jsmntok_t* jsonTokens, i32 numTokens, i32 arrayTokenIndex, Color_t* colorOut)
{
	jsmntok_t* arrayTokenPntr = &jsonTokens[arrayTokenIndex];
	
	if (arrayTokenPntr->size != 3 && arrayTokenPntr->size != 4)
	{
		DEBUG_PrintLine("Expected 3-4 items in color array. Found %d", arrayTokenPntr->size);
		return false;
	}
	
	//Find all the child items
	i32 childTokenIndex[4] = {};
	childTokenIndex[0] = FindChildTokenByIndex(jsonTokens, numTokens, arrayTokenIndex, 0);
	childTokenIndex[1] = FindChildTokenByIndex(jsonTokens, numTokens, arrayTokenIndex, 1);
	childTokenIndex[2] = FindChildTokenByIndex(jsonTokens, numTokens, arrayTokenIndex, 2);
	childTokenIndex[3] = FindChildTokenByIndex(jsonTokens, numTokens, arrayTokenIndex, 3);
	
	if (childTokenIndex[0] == -1 || childTokenIndex[1] == -1 || childTokenIndex[2] == -1 ||
		(arrayTokenPntr->size == 4 && childTokenIndex[3] == -1))
	{
		DEBUG_WriteLine("Couldn't find all child items in color array.");
		return false;
	}
	
	//Make sure we can parse them as numbers
	//and that they are in range of a uint8
	i32 values[4] = {};
	
	if (!TryGetTokenAsNumber(fileData, &jsonTokens[childTokenIndex[0]], &values[0]) ||
			values[0] < 0 || values[0] > 255)
	{
		DEBUG_WriteLine("Couldn't parse first item in array as uint8");
		return false;
	}
	else if (!TryGetTokenAsNumber(fileData, &jsonTokens[childTokenIndex[1]], &values[1]) ||
			values[1] < 0 || values[1] > 255)
	{
		DEBUG_WriteLine("Couldn't parse second item in array as uint8");
		return false;
	}
	else if (!TryGetTokenAsNumber(fileData, &jsonTokens[childTokenIndex[2]], &values[2]) ||
			values[2] < 0 || values[2] > 255)
	{
		DEBUG_WriteLine("Couldn't parse third item in array as uint8");
		return false;
	}
	else if (childTokenIndex[3] != -1 &&
		(!TryGetTokenAsNumber(fileData, &jsonTokens[childTokenIndex[3]], &values[3]) ||
		values[3] < 0 || values[3] > 255))
	{
		DEBUG_WriteLine("Couldn't parse fourth item in array as uint8");
		return false;
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
	
	return true;
}

#define GetColorConfig(parentObjectIndex, tokenName, colorPntr, defaultValue) do                          \
{                                                                                                         \
	i32 tokenIndex = FindChildTokenByName(fileData, jsonTokens, numTokens, parentObjectIndex, tokenName); \
	if (tokenIndex == -1)                                                                                 \
	{                                                                                                     \
		DEBUG_PrintLine("Couldn't find \"%s\" token in Colors object", tokenName);                        \
		*colorPntr = defaultValue;                                                                        \
	}                                                                                                     \
	else                                                                                                  \
	{                                                                                                     \
		jsmntok_t* tokenPntr = &jsonTokens[tokenIndex];                                                   \
		i32 valueTokenIndex = GetChildToken(jsonTokens, numTokens, tokenIndex);                           \
		jsmntok_t* valueTokenPntr = &jsonTokens[valueTokenIndex];                                         \
		char* valueStringPntr = &fileData[valueTokenPntr->start];                                         \
		if (valueTokenPntr->type == JSMN_ARRAY)                                                           \
		{                                                                                                 \
			if (!TryGetColorFromJsonArray(fileData, jsonTokens, numTokens, valueTokenIndex, colorPntr))   \
			{                                                                                             \
				DEBUG_PrintLine("Couldn't parse array as color for \"%s\": \"%.*s\"",                     \
					tokenName, TokenLength(valueTokenPntr), valueStringPntr);                             \
				*colorPntr = defaultValue;                                                                \
			}                                                                                             \
		}                                                                                                 \
		else                                                                                              \
		{                                                                                                 \
			if (!TryParseColor(valueStringPntr, TokenLength(valueTokenPntr), colorPntr))                  \
			{                                                                                             \
				DEBUG_PrintLine("Couldn't parse value as color for \"%s\": \"%.*s\"",                     \
					tokenName, TokenLength(valueTokenPntr), valueStringPntr);                             \
				*colorPntr = defaultValue;                                                                \
			}                                                                                             \
		}                                                                                                 \
	}                                                                                                     \
} while (0)

void LoadGlobalConfiguration(const PlatformInfo_t* PlatformInfo, GlobalConfig_t* globalConfig)
{
	ClearPointer(globalConfig);
	
	//+================================+
	//|       Set Default Values       |
	//+================================+
	globalConfig->colors.background = Color_Background;
	globalConfig->colors.highlight1 = Color_Highlight1;
	globalConfig->colors.highlight2 = Color_Highlight2;
	globalConfig->colors.highlight3 = Color_Highlight3;
	globalConfig->colors.highlight4 = Color_Highlight4;
	globalConfig->colors.highlight5 = Color_Highlight5;
	
	//+==================================+
	//|       Parse the JSON File        |
	//+==================================+
	FileInfo_t globalConfigFile = PlatformInfo->ReadEntireFilePntr(GLOBAL_CONFIG_FILEPATH);
	char* fileData = (char*)globalConfigFile.content;
	SanatizeFileComments(fileData, globalConfigFile.size);
	
	jsmn_parser jsonParser;
	jsmntok_t jsonTokens[MAX_JSON_TOKENS];
	jsmn_init(&jsonParser);
	i32 numTokens = jsmn_parse(&jsonParser,
		fileData, globalConfigFile.size,
		jsonTokens, ArrayCount(jsonTokens));
	
	if (numTokens < 0)
	{
		StatusError("JSON Parsing Error %d in GlobalConfig.json", numTokens);
		return;
	}
	else if (numTokens == 0)
	{
		StatusError("No JSON Objects Found in GlobalConfig.json");
		return;
	}
	else if (jsonTokens[0].type != JSMN_OBJECT)
	{
		StatusError("Top Item Was Not Object in GlobalConfig.json");
		return;
	}
	
	//+==================================+
	//|            UI Colors             |
	//+==================================+
	i32 colorsTokenIndex = FindChildTokenByName(fileData, jsonTokens, numTokens, 0, "Colors");
	if (colorsTokenIndex == -1)
	{
		DEBUG_WriteLine("Couldn't find \"Colors\" object in GlobalConfig.json");
	}
	else
	{
		i32 colorsObjectIndex = GetChildToken(jsonTokens, numTokens, colorsTokenIndex);
		
		GetColorConfig(colorsObjectIndex, "Background", &globalConfig->colors.background, Color_Background);
		GetColorConfig(colorsObjectIndex, "Highlight1", &globalConfig->colors.highlight1, Color_Highlight1);
		GetColorConfig(colorsObjectIndex, "Highlight2", &globalConfig->colors.highlight2, Color_Highlight2);
		GetColorConfig(colorsObjectIndex, "Highlight3", &globalConfig->colors.highlight3, Color_Highlight3);
		GetColorConfig(colorsObjectIndex, "Highlight4", &globalConfig->colors.highlight4, Color_Highlight4);
		GetColorConfig(colorsObjectIndex, "Highlight5", &globalConfig->colors.highlight5, Color_Highlight5);
	}
	
	PlatformInfo->FreeFileMemoryPntr(&globalConfigFile);
	
	StatusSuccess("Global Configuration Loaded Successfully!");
}