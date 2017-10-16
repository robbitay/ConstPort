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
	ConfigError_t errorCode = TryGet##Type##Config(jsonData, parentIndex, tokenName, valuePntr); \
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
		case ConfigError_ExpectedArray:                                                           \
			StatusError("Error parsing \"%s\": Expected Array", tokenName);                       \
			parseSuccess = false; break;                                                          \
	};                                                                                            \
} while(0)

#define GetStrConfig(parentIndex, tokenName, valuePntr, memoryArena) do                                      \
{                                                                                                            \
	ConfigError_t errorCode = TryGetStringConfig(jsonData, parentIndex, tokenName, valuePntr, memoryArena); \
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
		case ConfigError_ExpectedArray:                                                                      \
			StatusError("Error parsing \"%s\": Expected Array", tokenName);                                  \
			parseSuccess = false; break;                                                                     \
	};                                                                                                       \
} while(0)

void ParseRegexTriggersList(GlobalConfig_t* globalConfig, MemoryArena_t* memArena, JsonData_t* jsonData, i32 parentListIndex)
{
	u32 numTriggers = GetNumChildObjects(jsonData, parentListIndex);
	globalConfig->numTriggers = numTriggers;
	
	if (numTriggers == 0)
	{
		globalConfig->triggers = nullptr;
		return;
	}
	
	globalConfig->triggers = PushArray(memArena, RegexTrigger_t, numTriggers);
	
	for (u32 listIndex = 0; listIndex < numTriggers; listIndex++)
	{
		RegexTrigger_t* triggerPntr = &globalConfig->triggers[listIndex];
		triggerPntr->expression = nullptr;
		triggerPntr->expressionName = nullptr;
		triggerPntr->showOnlyCaptured = false;
		triggerPntr->runPerCharacter = false;
		triggerPntr->numEffects = 0;
		triggerPntr->effects = nullptr;
		triggerPntr->numComPorts = 0;
		triggerPntr->comPorts = nullptr;
		
		i32 triggerObjectIndex = FindChildTokenByIndex(jsonData, parentListIndex, listIndex);
		
		if (triggerObjectIndex == -1)
		{
			DEBUG_PrintLine("Couldn't find child[%u] in \"regex_triggers\" array", listIndex);
			continue;
		}
		else if (jsonData->tokens[triggerObjectIndex].type != JSMN_OBJECT)
		{
			DEBUG_Print("\"regex_triggers\" child[%u] was not an object.", listIndex);
			continue;
		}
		
		bool parseSuccess = true;
		
		GetStrConfig(triggerObjectIndex, "expression",      &triggerPntr->expression, memArena);
		GetStrConfig(triggerObjectIndex, "expression_name", &triggerPntr->expressionName, memArena);
		
		GetConfig(triggerObjectIndex, Bool, "show_only_captured", &triggerPntr->showOnlyCaptured);
		GetConfig(triggerObjectIndex, Bool, "run_per_character",  &triggerPntr->runPerCharacter);
		
		// DEBUG_PrintLine("Trigger[%u]:", listIndex);
		// DEBUG_PrintLine("\tExpression:         \"%s\"", triggerPntr->expression);
		// DEBUG_PrintLine("\tExpression Name:    \"%s\"", triggerPntr->expressionName);
		// DEBUG_PrintLine("\tShow Only Captured: %s", triggerPntr->showOnlyCaptured ? "True" : "False");
		// DEBUG_PrintLine("\tRun Per Character:  %s", triggerPntr->runPerCharacter ? "True" : "False");
		
		// +==============================+
		// |    Parse the Effects List    |
		// +==============================+
		i32 effectsTokenIndex = FindChildTokenByName(jsonData, triggerObjectIndex, "effects");
		if (effectsTokenIndex != -1)
		{
			i32 effectListIndex = GetChildToken(jsonData, effectsTokenIndex);
			if (effectListIndex != -1 && jsonData->tokens[effectListIndex].type == JSMN_ARRAY)
			{
				u32 numEffects = GetNumChildTokens(jsonData, effectListIndex);
				triggerPntr->numEffects = numEffects;
				
				// DEBUG_PrintLine("\t%u Effects:", numEffects);
				if (numEffects > 0)
				{
					triggerPntr->effects = PushArray(memArena, const char*, numEffects);
					
					for (u32 effectIndex = 0; effectIndex < numEffects; effectIndex++)
					{
						const char** effectStrPntr = &triggerPntr->effects[effectIndex];
						
						i32 effectTokenIndex = FindChildTokenByIndex(jsonData, effectListIndex, effectIndex);
						if (effectTokenIndex == -1) { continue; }
						jsmntok_t* effectToken = &jsonData->tokens[effectTokenIndex];
						
						u32 effectStringLength = TokenLength(effectToken);
						char* newString = PushArray(memArena, char, effectStringLength+1);
						memcpy(newString, &jsonData->data[effectToken->start], effectStringLength);
						newString[effectStringLength] = '\0';
						*effectStrPntr = newString;
						
						// DEBUG_PrintLine("\t\tEffect[%u] \"%s\"", effectIndex, newString);
					}
				}
			}
		}
		
		// +==============================+
		// |   Parse the Com Ports List   |
		// +==============================+
		i32 comPortsTokenIndex = FindChildTokenByName(jsonData, triggerObjectIndex, "com_ports");
		if (comPortsTokenIndex != -1)
		{
			i32 comPortsListIndex = GetChildToken(jsonData, comPortsTokenIndex);
			if (comPortsListIndex != -1 && jsonData->tokens[comPortsListIndex].type == JSMN_ARRAY)
			{
				u32 numComPorts = GetNumChildTokens(jsonData, comPortsListIndex);
				triggerPntr->numComPorts = numComPorts;
				
				// DEBUG_PrintLine("\t%u Com Ports:", numComPorts);
				if (numComPorts > 0)
				{
					triggerPntr->comPorts = PushArray(memArena, const char*, numComPorts);
					
					for (u32 comListIndex = 0; comListIndex < numComPorts; comListIndex++)
					{
						const char** comPortStrPntr = &triggerPntr->comPorts[comListIndex];
						
						i32 comPortTokenIndex = FindChildTokenByIndex(jsonData, comPortsListIndex, comListIndex);
						if (comPortTokenIndex == -1) { continue; }
						jsmntok_t* comPortToken = &jsonData->tokens[comPortTokenIndex];
						
						u32 comPortStrLength = TokenLength(comPortToken);
						char* newString = PushArray(memArena, char, comPortStrLength+1);
						memcpy(newString, &jsonData->data[comPortToken->start], comPortStrLength);
						newString[comPortStrLength] = '\0';
						*comPortStrPntr = newString;
						
						// DEBUG_PrintLine("\t\tComPort[%u] \"%s\"", comListIndex, newString);
					}
				}
			}
		}
		
		if (triggerPntr->expression == nullptr && triggerPntr->expressionName == nullptr)
		{
			DEBUG_PrintLine("Neither \"expression\" nor \"expression_name\" were defined for trigger[%u]", listIndex);
		}
		
	}
}

void LoadGlobalConfiguration(const PlatformInfo_t* PlatformInfo, GlobalConfig_t* globalConfig, MemoryArena_t* memArena)
{
	TempPushMark();
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
	
	BoundedStrListCreate(&globalConfig->comNameKeys, MAX_USER_PORT_NAMES, MAX_COM_PORT_NAME_LENGTH, TempArena);
	BoundedStrListCreate(&globalConfig->comNameValues, MAX_USER_PORT_NAMES, MAX_USER_PORT_NAME_LENGTH, TempArena);
	
	//+==================================+
	//|       Parse the JSON File        |
	//+==================================+
	FileInfo_t globalConfigFile = PlatformInfo->ReadEntireFile(GLOBAL_CONFIG_FILEPATH);
	char* fileData = (char*)globalConfigFile.content;
	SanatizeFileComments(fileData, globalConfigFile.size);

	jsmn_parser jsonParser;
	jsmn_init(&jsonParser);
	i32 numTokens = jsmn_parse(&jsonParser,
		fileData, globalConfigFile.size,
		nullptr, 0);
	
	jsmn_init(&jsonParser);
	jsmntok_t* jsonTokens = TempArray(jsmntok_t, numTokens);
	
	numTokens = jsmn_parse(&jsonParser,
		fileData, globalConfigFile.size,
		jsonTokens, numTokens);
	
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
	else { DEBUG_PrintLine("%d/%u JSON tokens filled", numTokens, MAX_JSON_TOKENS); }
	
	JsonData_t jsonDataReal = {};
	jsonDataReal.data = fileData;
	jsonDataReal.tokens = jsonTokens;
	jsonDataReal.numTokens = numTokens;
	JsonData_t* jsonData = &jsonDataReal;
	
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
	
	// globalConfig->fontSize            *= GUI_SCALE;
	// globalConfig->lineSpacing         *= GUI_SCALE;
	// globalConfig->elapsedBannerHeight *= GUI_SCALE;
	// globalConfig->markHeight          *= GUI_SCALE;
	// globalConfig->thickMarkHeight     *= GUI_SCALE;
	// globalConfig->mouseClickTolerance *= GUI_SCALE;
	// globalConfig->menuBorderThickness *= GUI_SCALE;
	// globalConfig->scrollbarWidth      *= GUI_SCALE;
	// globalConfig->scrollbarPadding    *= GUI_SCALE;
	// globalConfig->minScrollbarHeight  *= GUI_SCALE;
	// globalConfig->minGutterWidth      *= GUI_SCALE;
	
	// +====================================+
	// | Parse Color Configuration Options  |
	// +====================================+
	i32 colorsTokenIndex = FindChildTokenByName(jsonData, 0, "colors");
	if (colorsTokenIndex == -1)
	{
		DEBUG_WriteLine("Couldn't find \"colors\" object in GlobalConfig.json");
	}
	else
	{
		i32 colorsObjectIndex = GetChildToken(jsonData, colorsTokenIndex);
		
		#define ConfigurationColor( identifier, variableName, defaultValue) GetConfig(colorsObjectIndex, Color, identifier, &globalConfig->colors.variableName)
		
		#define INCLUDE_COLOR_OPTIONS
		#include "appConfigOptions.h"
	}
	
	// +==============================+
	// | Parse COM Port Name Options  |
	// +==============================+
	i32 portNamesTokenIndex = FindChildTokenByName(jsonData, 0, "port_names");
	if (portNamesTokenIndex == -1)
	{
		DEBUG_WriteLine("Couldn't find \"port_names\" object in GlobalConfig.json");
	}
	else
	{
		i32 portNamesArrayIndex = GetChildToken(jsonData, portNamesTokenIndex);
		u32 numChildren = GetNumChildTokens(jsonData, portNamesArrayIndex);
		
		for (u32 childIndex = 0; childIndex < numChildren; childIndex++)
		{
			i32 childKeyTokenIndex = FindChildTokenByIndex(jsonData, portNamesArrayIndex, childIndex);
			if (childKeyTokenIndex == -1) { continue; }
			jsmntok_t* childKeyToken = &jsonData->tokens[childKeyTokenIndex];
			i32 childValueTokenIndex = GetChildToken(jsonData, childKeyTokenIndex);
			if (childValueTokenIndex == -1) { continue; }
			jsmntok_t* childValueToken = &jsonData->tokens[childValueTokenIndex];
			
			const char* keyStrPntr = &jsonData->data[childKeyToken->start];
			char* keyString = DupStrN(keyStrPntr, TokenLength(childKeyToken), TempArena);
			
			const char* valueStrPntr = &jsonData->data[childValueToken->start];
			char* valueString = DupStrN(valueStrPntr, TokenLength(childValueToken), TempArena);
			
			if (strlen(keyString) > MAX_COM_PORT_NAME_LENGTH)
			{
				DEBUG_PrintLine("\"port_names\" item[%u]: Identifier \"%s\" %u char length > max %u chars", childIndex, keyString, strlen(keyString), MAX_COM_PORT_NAME_LENGTH);
			}
			else if (strlen(valueString) > MAX_USER_PORT_NAME_LENGTH)
			{
				DEBUG_PrintLine("\"port_names\" item[%u]: Name \"%s\" %u char length > max %u chars", childIndex, valueString, strlen(valueString), MAX_USER_PORT_NAME_LENGTH);
			}
			else
			{
				DEBUG_PrintLine("User Name[%u]: \"%s\" = \"%s\"", childIndex, keyString, valueString);
				BoundedStrListAdd(&globalConfig->comNameKeys, keyString);
				BoundedStrListAdd(&globalConfig->comNameValues, valueString);
			}
		}
	}
	BoundedStrListSolidify(&globalConfig->comNameKeys,   TempArena, memArena, false);
	BoundedStrListSolidify(&globalConfig->comNameValues, TempArena, memArena, false);
	
	// +==============================+
	// |  Parse Regex Triggers List   |
	// +==============================+
	i32 regexTriggersTokenIndex = FindChildTokenByName(jsonData, 0, "regex_triggers");
	if (regexTriggersTokenIndex == -1)
	{
		DEBUG_WriteLine("Couldn't file \"regex_triggers\" object in GlobalConfig.json");
	}
	else
	{
		i32 regexTriggersListIndex = GetChildToken(jsonData, regexTriggersTokenIndex);
		
		if (regexTriggersListIndex == -1 || jsonTokens[regexTriggersListIndex].type != JSMN_ARRAY)
		{
			DEBUG_WriteLine("\"regex_triggers\" value was not array in GlobalConfig.json");
		}
		else
		{
			ParseRegexTriggersList(globalConfig, memArena, jsonData, regexTriggersListIndex);
		}
	}
	
	PlatformInfo->FreeFileMemory(&globalConfigFile);
	
	if (parseSuccess)
	{
		StatusSuccess("Global Configuration Loaded Successfully!");
	}
	
	TempPopMark();
}

void DisposeGlobalConfig(GlobalConfig_t* globalConfig)
{
	DEBUG_WriteLine("Disposing old GlobalConfig");
	
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
	
	BoundedStrListDestroy(&globalConfig->comNameKeys, globalConfig->memArena);
	BoundedStrListDestroy(&globalConfig->comNameValues, globalConfig->memArena);
	
	for (u32 tIndex = 0; tIndex < globalConfig->numTriggers; tIndex++)
	{
		RegexTrigger_t* trigger = &globalConfig->triggers[tIndex];
		
		if (trigger->expression != nullptr) { ArenaPop(globalConfig->memArena, (char*)trigger->expression); }
		if (trigger->expressionName != nullptr) { ArenaPop(globalConfig->memArena, (char*)trigger->expressionName); }
		
		for (u32 eIndex = 0; eIndex < trigger->numEffects; eIndex++)
		{
			const char* effectStr = trigger->effects[eIndex];
			if (effectStr != nullptr)
			{
				ArenaPop(globalConfig->memArena, (char*)effectStr);
			}
		}
		if (trigger->effects != nullptr) { ArenaPop(globalConfig->memArena, trigger->effects); }
		
		for (u32 eIndex = 0; eIndex < trigger->numComPorts; eIndex++)
		{
			const char* comPortStr = trigger->comPorts[eIndex];
			if (comPortStr != nullptr)
			{
				ArenaPop(globalConfig->memArena, (char*)comPortStr);
			}
		}
		if (trigger->comPorts != nullptr) { ArenaPop(globalConfig->memArena, trigger->comPorts); }
	}
	if (globalConfig->triggers != nullptr) { ArenaPop(globalConfig->memArena, globalConfig->triggers); }
	
	ClearPointer(globalConfig);
}