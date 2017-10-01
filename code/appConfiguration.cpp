/*
File:   appConfiguration.cpp
Author: Taylor Robbins
Date:   08\02\2017
Description:
	** Handles loading and saving configuration TOML files
	** for various aspects of the application

#included from app.cpp
*/

#include "configurationHelpers.h"

#define GetConfig(parentIndex, Type, tokenName, valuePntr) do                                     \
{                                                                                                 \
	ConfigError_t errorCode = TryGet##Type##Config(&jsonData, parentIndex, tokenName, valuePntr); \
	switch (errorCode)                                                                            \
	{                                                                                             \
		case ConfigError_None: /*DEBUG_PrintLine("Parsed \"%s\" correctly!", tokenName);*/ break; \
		case ConfigError_TokenNotFound:                                                           \
			/*StatusError("Error parsing \"%s\": Token Not Found", tokenName);*/                  \
			parseSuccess = false; break;                                                          \
		case ConfigError_InvalidNumber:                                                           \
			StatusError("Error parsing \"%s\": Invalid Number", tokenName);                       \
			parseSuccess = false; break;                                                          \
		case ConfigError_InvalidBoolean:                                                          \
			StatusError("Error parsing \"%s\": Invalid Boolean", tokenName);                      \
			parseSuccess = false; break;                                                          \
		case ConfigError_ColorDoesntExist:                                                        \
			StatusError("Error parsing \"%s\": Color Doesn't Exist", tokenName);                  \
			parseSuccess = false; break;                                                          \
		case ConfigError_WrongNumberOfArrayElements:                                              \
			StatusError("Error parsing \"%s\": Wrong Number of Array Elements", tokenName);       \
			parseSuccess = false; break;                                                          \
		case ConfigError_NumberOutOfRange:                                                        \
			StatusError("Error parsing \"%s\": Number Out of Range", tokenName);                  \
			parseSuccess = false; break;                                                          \
	};                                                                                            \
} while(0)

#define GetStrConfig(parentIndex, tokenName, valuePntr, memoryArena) do                                      \
{                                                                                                            \
	ConfigError_t errorCode = TryGetStringConfig(&jsonData, parentIndex, tokenName, valuePntr, memoryArena); \
	switch (errorCode)                                                                                       \
	{                                                                                                        \
		case ConfigError_None: /*DEBUG_PrintLine("Parsed \"%s\" correctly!", tokenName);*/ break;            \
		case ConfigError_TokenNotFound:                                                                      \
			/*StatusError("Error parsing \"%s\": Token Not Found", tokenName);*/                             \
			parseSuccess = false; break;                                                                     \
		case ConfigError_InvalidNumber:                                                                      \
			StatusError("Error parsing \"%s\": Invalid Number", tokenName);                                  \
			parseSuccess = false; break;                                                                     \
		case ConfigError_InvalidBoolean:                                                                     \
			StatusError("Error parsing \"%s\": Invalid Boolean", tokenName);                                 \
			parseSuccess = false; break;                                                                     \
		case ConfigError_ColorDoesntExist:                                                                   \
			StatusError("Error parsing \"%s\": Color Doesn't Exist", tokenName);                             \
			parseSuccess = false; break;                                                                     \
		case ConfigError_WrongNumberOfArrayElements:                                                         \
			StatusError("Error parsing \"%s\": Wrong Number of Array Elements", tokenName);                  \
			parseSuccess = false; break;                                                                     \
		case ConfigError_NumberOutOfRange:                                                                   \
			StatusError("Error parsing \"%s\": Number Out of Range", tokenName);                             \
			parseSuccess = false; break;                                                                     \
	};                                                                                                       \
} while(0)

void LoadGlobalConfiguration(const PlatformInfo_t* PlatformInfo, GlobalConfig_t* globalConfig, MemoryArena_t* memArena)
{
	ClearPointer(globalConfig);
	
	globalConfig->memArena = memArena;
	
	//+================================+
	//|       Set Default Values       |
	//+================================+
	#define ConfigurationBool(  identifier, variableName, defaultValue) globalConfig->variableName = defaultValue;
	#define ConfigurationInt32( identifier, variableName, defaultValue) globalConfig->variableName = defaultValue;
	#define ConfigurationColor( identifier, variableName, defaultValue) globalConfig->variableName = defaultValue;
	#define ConfigurationString(identifier, variableName, defaultValue) globalConfig->variableName = defaultValue;
	
	#define INCLUDE_GLOBAL_OPTIONS
	#include "appConfigOptions.h"
	
	#define ConfigurationColor(identifier, variableName, defaultValue) globalConfig->colors.variableName = defaultValue;
	
	#define INCLUDE_COLOR_OPTIONS
	#include "appConfigOptions.h"
	
	for (u8 comIndex = ComPort_1; comIndex < NumComPorts; comIndex++)
	{
		globalConfig->comPortNames[comIndex] = GetComPortReadableName((ComPortIndex_t)comIndex);
	}

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
	
	JsonData_t jsonData = {};
	jsonData.data = fileData;
	jsonData.tokens = jsonTokens;
	jsonData.numTokens = numTokens;
	
	bool parseSuccess = true;
	
	// +==============================+
	// | Parse Configuration Options  |
	// +==============================+
	#define ConfigurationBool(  identifier, variableName, defaultValue) GetConfig(0, Bool, identifier, &globalConfig->variableName)
	#define ConfigurationInt32( identifier, variableName, defaultValue) GetConfig(0, Int32, identifier, &globalConfig->variableName)
	#define ConfigurationColor( identifier, variableName, defaultValue) GetConfig(0, Color, identifier, &globalConfig->variableName)
	#define ConfigurationString(identifier, variableName, defaultValue) GetStrConfig(0, identifier, &globalConfig->variableName, memArena)
	
	#define INCLUDE_GLOBAL_OPTIONS
	#include "appConfigOptions.h"
	
	// +====================================+
	// | Parse Color Configuration Options  |
	// +====================================+
	i32 colorsTokenIndex = FindChildTokenByName(&jsonData, 0, "colors");
	if (colorsTokenIndex == -1)
	{
		DEBUG_WriteLine("Couldn't find \"colors\" object in GlobalConfig.json");
	}
	else
	{
		i32 colorsObjectIndex = GetChildToken(&jsonData, colorsTokenIndex);
		
		#define ConfigurationColor( identifier, variableName, defaultValue) GetConfig(colorsObjectIndex, Color, identifier, &globalConfig->colors.variableName)
		
		#define INCLUDE_COLOR_OPTIONS
		#include "appConfigOptions.h"
	}
	
	// +==============================+
	// | Parse COM Port Name Options  |
	// +==============================+
	i32 portNamesTokenIndex = FindChildTokenByName(&jsonData, 0, "port_names");
	if (portNamesTokenIndex == -1)
	{
		DEBUG_WriteLine("Couldn't find \"port_names\" object in GlobalConfig.json");
	}
	else
	{
		i32 portNamesIndex = GetChildToken(&jsonData, portNamesTokenIndex);
		
		for (u8 comIndex = ComPort_1; comIndex < NumComPorts; comIndex++)
		{
			const char* comName = GetComPortReadableName((ComPortIndex_t)comIndex);
			
			const char* assignedName = comName;
			GetStrConfig(portNamesIndex, comName, &assignedName, memArena);
			
			globalConfig->comPortNames[comIndex] = assignedName;
		}
	}

	PlatformInfo->FreeFileMemoryPntr(&globalConfigFile);
	
	if (parseSuccess)
	{
		StatusSuccess("Global Configuration Loaded Successfully!");
	}
}

void DisposeGlobalConfig(GlobalConfig_t* globalConfig)
{
	DEBUG_WriteLine("Disposing of old strings in GlobalConfig");
	
	#define ConfigurationBool(  identifier, variableName, defaultValue) 
	#define ConfigurationInt32( identifier, variableName, defaultValue) 
	#define ConfigurationColor( identifier, variableName, defaultValue) 
	#define ConfigurationString(identifier, variableName, defaultValue) do       \
	{                                                                            \
		if (globalConfig->variableName != nullptr)                               \
		{                                                                        \
			ArenaPop(globalConfig->memArena, (char*)globalConfig->variableName); \
		}                                                                        \
	} while(0)
	
	#define INCLUDE_GLOBAL_OPTIONS
	#include "appConfigOptions.h"
	
	for (u8 comIndex = ComPort_1; comIndex < NumComPorts; comIndex++)
	{
		if (globalConfig->comPortNames[comIndex] != nullptr)
		{
			ArenaPop(globalConfig->memArena, (char*)globalConfig->comPortNames[comIndex]);
			globalConfig->comPortNames[comIndex] = nullptr;
		}
	}
	
	ClearPointer(globalConfig);
}