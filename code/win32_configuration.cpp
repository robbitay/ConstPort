/*
File:   win32_configuration.cpp
Author: Taylor Robbins
Date:   08\06\2017
Description: 
	** Handles loading PlatformConfiguration.json file and parsing
	** the values into PlatformConfig_t structure 
*/

#include "configurationHelpers.h"

struct PlatformConfig_t
{
	i32 windowWidth;
	i32 windowHeight;
	i32 minWindowWidth;
	i32 minWindowHeight;
	bool forceAspectRatio;
	Vec2i_t aspectRatio;
	bool allowResizeWindow;
	i32 antiAliasingSamples;
	bool topmostWindow;
	bool showConsoleWindow;
	
	i32 permanantMemorySize;
	i32 transientMemorySize;
};

#define GetConfig(parentIndex, Type, tokenName, valuePntr) do                                     \
{                                                                                                 \
	ConfigError_t errorCode = TryGet##Type##Config(&jsonData, parentIndex, tokenName, valuePntr); \
	switch (errorCode)                                                                            \
	{                                                                                             \
		case ConfigError_None: /*Win32_PrintLine("Parsed \"%s\" correctly!", tokenName);*/ break; \
		case ConfigError_TokenNotFound:                                                           \
			Win32_PrintLine("Error parsing \"%s\": Token Not Found", tokenName);                  \
			parseSuccess = false; break;                                                          \
		case ConfigError_InvalidNumber:                                                           \
			Win32_PrintLine("Error parsing \"%s\": Invalid Number", tokenName);                   \
			parseSuccess = false; break;                                                          \
		case ConfigError_InvalidBoolean:                                                          \
			Win32_PrintLine("Error parsing \"%s\": Invalid Boolean", tokenName);                  \
			parseSuccess = false; break;                                                          \
		case ConfigError_ColorDoesntExist:                                                        \
			Win32_PrintLine("Error parsing \"%s\": Color Doesn't Exist", tokenName);              \
			parseSuccess = false; break;                                                          \
		case ConfigError_WrongNumberOfArrayElements:                                              \
			Win32_PrintLine("Error parsing \"%s\": Wrong Number of Array Elements", tokenName);   \
			parseSuccess = false; break;                                                          \
		case ConfigError_NumberOutOfRange:                                                        \
			Win32_PrintLine("Error parsing \"%s\": Number Out of Range", tokenName);              \
			parseSuccess = false; break;                                                          \
		case ConfigError_ExpectedArray:                                                           \
			Win32_PrintLine("Error parsing \"%s\": Expected Array", tokenName);                   \
			parseSuccess = false; break;                                                          \
	};                                                                                            \
} while(0)

void LoadGlobalConfiguration(PlatformConfig_t* platformConfig)
{
	ClearPointer(platformConfig);

	//+================================+
	//|       Set Default Values       |
	//+================================+
	platformConfig->windowWidth         = 500;
	platformConfig->windowHeight        = 400;
	platformConfig->minWindowWidth      = 350;
	platformConfig->minWindowHeight     = 400;
	platformConfig->forceAspectRatio    = false;
	platformConfig->aspectRatio         = NewVec2i(16, 9);
	platformConfig->allowResizeWindow   = true;
	platformConfig->antiAliasingSamples = 4;
	platformConfig->topmostWindow       = false;
	platformConfig->showConsoleWindow   = false;
	platformConfig->permanantMemorySize = 64;
	platformConfig->transientMemorySize = 1024;

	//+==================================+
	//|       Parse the JSON File        |
	//+==================================+
	FileInfo_t configFile = Win32_ReadEntireFile("Resources/Configuration/PlatformConfig.json");
	char* fileData = (char*)configFile.content;
	SanatizeFileComments(fileData, configFile.size);

	jsmn_parser jsonParser;
	jsmntok_t jsonTokens[1024];
	jsmn_init(&jsonParser);
	i32 numTokens = jsmn_parse(&jsonParser,
		fileData, configFile.size,
		jsonTokens, ArrayCount(jsonTokens));

	if (numTokens < 0)
	{
		Win32_PrintLine("JSON Parsing Error %d in GlobalConfig.json", numTokens);
		return;
	}
	else if (numTokens == 0)
	{
		Win32_WriteLine("No JSON Objects Found in GlobalConfig.json");
		return;
	}
	else if (jsonTokens[0].type != JSMN_OBJECT)
	{
		Win32_WriteLine("Top Item Was Not Object in GlobalConfig.json");
		return;
	}
	
	JsonData_t jsonData = {};
	jsonData.data = fileData;
	jsonData.tokens = jsonTokens;
	jsonData.numTokens = numTokens;
	
	bool parseSuccess = true;
	
	//+==================================+
	//|      Configuration Options       |
	//+==================================+
	{
		GetConfig(0, Int32, "WindowWidth",         &platformConfig->windowWidth);
		GetConfig(0, Int32, "WindowHeight",        &platformConfig->windowHeight);
		GetConfig(0, Int32, "MinWindowWidth",      &platformConfig->minWindowWidth);
		GetConfig(0, Int32, "MinWindowHeight",     &platformConfig->minWindowHeight);
		GetConfig(0, Bool,  "ForceAspectRatio",    &platformConfig->forceAspectRatio);
		GetConfig(0, Vec2i, "AspectRatio",         &platformConfig->aspectRatio);
		GetConfig(0, Bool,  "AllowResizeWindow",   &platformConfig->allowResizeWindow);
		GetConfig(0, Int32, "AntiAliasingSamples", &platformConfig->antiAliasingSamples);
		GetConfig(0, Bool,  "TopmostWindow",       &platformConfig->topmostWindow);
		GetConfig(0, Bool,  "ShowConsoleWindow",   &platformConfig->showConsoleWindow);
		
		GetConfig(0, Int32, "PermanantMemorySize", &platformConfig->permanantMemorySize);
		GetConfig(0, Int32, "TransientMemorySize", &platformConfig->transientMemorySize);
	}

	Win32_FreeFileMemory(&configFile);
	
	if (parseSuccess)
	{
		Win32_WriteLine("Platform Configuration Loaded Successfully!");
	}
	else
	{
		Win32_WriteLine("Errors while loading Platform Configuration!");
	}
}
