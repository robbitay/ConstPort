/*
File:   appConfiguration.h
Author: Taylor Robbins
Date:   08\02\2017
*/

#ifndef _APP_CONFIGURATION_H
#define _APP_CONFIGURATION_H

struct RegexTrigger_t
{
	const char* expression;
	const char* expressionName;
	const char* replaceStr;
	
	bool showOnlyCaptured;
	bool runPerCharacter;
	
	u32 numEffects;
	const char** effects;
	
	u32 numComPorts;
	const char** comPorts;
};

struct GlobalConfig_t
{
	MemoryArena_t* memArena;
	
	#define ConfigurationBool(  identifier, variableName, defaultValue) bool        variableName;
	#define ConfigurationInt32( identifier, variableName, defaultValue) i32         variableName;
	#define ConfigurationColor( identifier, variableName, defaultValue) Color_t     variableName;
	#define ConfigurationString(identifier, variableName, defaultValue) const char* variableName;
	
	#define INCLUDE_GLOBAL_OPTIONS
	#include "appConfigOptions.h"
	
	struct
	{
		#define ConfigurationColor(identifier, variableName, defaultValue) Color_t variableName;
		
		#define INCLUDE_COLOR_OPTIONS
		#include "appConfigOptions.h"
	} colors;
	
	BoundedStrList_t comNameKeys;
	BoundedStrList_t comNameValues;
	
	u32 numTriggers;
	RegexTrigger_t* triggers;
};

#endif // _APP_CONFIGURATION_H
