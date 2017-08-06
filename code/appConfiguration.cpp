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
		case ConfigError_None: DEBUG_PrintLine("Parsed \"%s\" correctly!", tokenName); break;     \
		case ConfigError_TokenNotFound:                                                           \
			StatusError("Error parsing \"%s\": Token Not Found", tokenName);                      \
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

void LoadGlobalConfiguration(const PlatformInfo_t* PlatformInfo, GlobalConfig_t* globalConfig)
{
	ClearPointer(globalConfig);

	//+================================+
	//|       Set Default Values       |
	//+================================+
	globalConfig->fontSize    = 16;
	globalConfig->lineSpacing = 2;
	globalConfig->tabWidth    = 4;
	
	globalConfig->elapsedBannerEnabled   = true;
	globalConfig->elapsedBannerTime      = 15;
	globalConfig->elapsedBannerHeight    = 25;
	globalConfig->statusMessageTime      = 3;
	globalConfig->mouseClickTolerance    = 10;
	globalConfig->menuBorderThickness    = 1;
	globalConfig->scrollbarWidth         = 12;
	globalConfig->scrollbarPadding       = 3;
	globalConfig->minScrollbarHeight     = 32;
	globalConfig->minGutterWidth         = 40;
	globalConfig->scrollMultiplier       = 40;
	globalConfig->viewSpeedDivider       = 4;
	globalConfig->showTextCursor         = false;
	globalConfig->showComNameInStatusBar = true;
	globalConfig->showLineNumbers        = true;
	globalConfig->showHoverCursor        = true;
	globalConfig->highlightHoverLine     = true;
	globalConfig->rxTxLedDelay           = 4;
	
	globalConfig->colors.background       = NewColor(33, 33, 33, 255);
	globalConfig->colors.foreground       = {0xFFF8F8F2};
	globalConfig->colors.selection1       = {Color_White};
	globalConfig->colors.selection2       = {Color_White};
	globalConfig->colors.uiGray1          = {0xFF494949};
	globalConfig->colors.uiGray2          = {0xFF404040};
	globalConfig->colors.uiGray3          = {0xFF303030};
	globalConfig->colors.uiGray4          = {0xFF101010};
	globalConfig->colors.uiLightGray1     = NewColor(180, 180, 180, 255);
	globalConfig->colors.receiveLed       = NewColor(249, 38, 101, 255);  //Red/Magenta
	globalConfig->colors.transmitLed      = NewColor(174, 129, 255, 255); //Purple
	globalConfig->colors.windowTitleBar   = globalConfig->colors.uiGray3;
	globalConfig->colors.windowBackground = globalConfig->colors.uiGray1;
	globalConfig->colors.windowBorder     = globalConfig->colors.uiLightGray1;
	globalConfig->colors.hoverLine        = globalConfig->colors.uiGray3;
	globalConfig->colors.markColor1       = NewColor(255, 255, 255, 128);
	globalConfig->colors.markColor2       = NewColor(255, 255, 255, 0);
	globalConfig->colors.bannerColor1     = globalConfig->colors.uiGray1;
	globalConfig->colors.bannerColor2     = globalConfig->colors.uiGray3;
	globalConfig->colors.highlight1       = NewColor(117, 113, 94, 255);  //Comment Color
	globalConfig->colors.highlight2       = NewColor(166, 226, 46, 255);  //Green Color
	globalConfig->colors.highlight3       = NewColor(249, 38, 101, 255);  //Red/Magenta
	globalConfig->colors.highlight4       = NewColor(174, 129, 255, 255); //Purple
	globalConfig->colors.highlight5       = NewColor(102, 217, 239, 255); //Light Blue
	
	globalConfig->colors.button               = {Color_White};
	globalConfig->colors.buttonText           = {Color_Black};
	globalConfig->colors.buttonBorder         = {Color_Black};
	globalConfig->colors.buttonHover          = globalConfig->colors.uiLightGray1;
	globalConfig->colors.buttonHoverText      = {Color_Black};
	globalConfig->colors.buttonHoverBorder    = {Color_Black};
	globalConfig->colors.buttonPress          = NewColor(249, 38, 101, 255);  //Red/Magenta
	globalConfig->colors.buttonPressText      = {Color_White};
	globalConfig->colors.buttonPressBorder    = {Color_White};
	globalConfig->colors.buttonSelected       = NewColor(174, 129, 255, 255); //Purple
	globalConfig->colors.buttonSelectedText   = {Color_White};
	globalConfig->colors.buttonSelectedBorder = {Color_White};
	globalConfig->colors.buttonReady          = NewColor(166, 226, 46, 255);  //Green Color
	globalConfig->colors.buttonReadyText      = {Color_White};
	globalConfig->colors.buttonReadyBorder    = {Color_White};

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
	
	//+==================================+
	//|         Integer Options          |
	//+==================================+
	{
		GetConfig(0, Int32, "FontSize",    &globalConfig->fontSize);
		GetConfig(0, Int32, "LineSpacing", &globalConfig->lineSpacing);
		GetConfig(0, Int32, "TabWidth",    &globalConfig->tabWidth);
		
		GetConfig(0, Bool,  "ElapsedBannerEnabled", &globalConfig->elapsedBannerEnabled);
		GetConfig(0, Int32, "ElapsedBannerTime",    &globalConfig->elapsedBannerTime);
		GetConfig(0, Int32, "ElapsedBannerHeight",  &globalConfig->elapsedBannerHeight);
		
		GetConfig(0, Int32, "MarkHeight",      &globalConfig->markHeight);
		GetConfig(0, Int32, "ThickMarkHeight", &globalConfig->thickMarkHeight);
		
		GetConfig(0, Int32, "StatusMessageTime", &globalConfig->statusMessageTime);
		
		GetConfig(0, Int32, "MouseClickTolerance", &globalConfig->mouseClickTolerance);
		GetConfig(0, Int32, "MenuBorderThickness", &globalConfig->menuBorderThickness);
		GetConfig(0, Int32, "ScrollbarWidth",      &globalConfig->scrollbarWidth);
		GetConfig(0, Int32, "ScrollbarPadding",    &globalConfig->scrollbarPadding);
		GetConfig(0, Int32, "MinScrollbarHeight",  &globalConfig->minScrollbarHeight);
		GetConfig(0, Int32, "MinGutterWidth",      &globalConfig->minGutterWidth);
		GetConfig(0, Int32, "ScrollMultiplier",    &globalConfig->scrollMultiplier);
		GetConfig(0, Int32, "ViewSpeedDivider",    &globalConfig->viewSpeedDivider);
		
		GetConfig(0, Bool, "ShowTextCursor",         &globalConfig->showTextCursor);
		GetConfig(0, Bool, "ShowComNameInStatusBar", &globalConfig->showComNameInStatusBar);
		GetConfig(0, Bool, "ShowLineNumbers",        &globalConfig->showLineNumbers);
		GetConfig(0, Bool, "ShowHoverCursor",        &globalConfig->showHoverCursor);
		GetConfig(0, Bool, "HighlightHoverLine",     &globalConfig->highlightHoverLine);
		
		GetConfig(0, Int32, "RxTxLedDelay", &globalConfig->rxTxLedDelay);
	}

	//+==================================+
	//|          Color Options           |
	//+==================================+
	i32 colorsTokenIndex = FindChildTokenByName(&jsonData, 0, "Colors");
	if (colorsTokenIndex == -1)
	{
		DEBUG_WriteLine("Couldn't find \"Colors\" object in GlobalConfig.json");
	}
	else
	{
		i32 colorsObjectIndex = GetChildToken(&jsonData, colorsTokenIndex);
		
		GetConfig(colorsObjectIndex, Color, "Background",       &globalConfig->colors.background);
		GetConfig(colorsObjectIndex, Color, "Foreground",       &globalConfig->colors.foreground);
		GetConfig(colorsObjectIndex, Color, "Selection1",       &globalConfig->colors.selection1);
		GetConfig(colorsObjectIndex, Color, "Selection2",       &globalConfig->colors.selection2);
		GetConfig(colorsObjectIndex, Color, "UiGray1",          &globalConfig->colors.uiGray1);
		GetConfig(colorsObjectIndex, Color, "UiGray2",          &globalConfig->colors.uiGray2);
		GetConfig(colorsObjectIndex, Color, "UiGray3",          &globalConfig->colors.uiGray3);
		GetConfig(colorsObjectIndex, Color, "UiGray4",          &globalConfig->colors.uiGray4);
		GetConfig(colorsObjectIndex, Color, "UiLightGray1",     &globalConfig->colors.uiLightGray1);
		GetConfig(colorsObjectIndex, Color, "ReceiveLed",       &globalConfig->colors.receiveLed);
		GetConfig(colorsObjectIndex, Color, "TransmitLed",      &globalConfig->colors.transmitLed);
		GetConfig(colorsObjectIndex, Color, "UiLightGray1",     &globalConfig->colors.uiLightGray1);
		GetConfig(colorsObjectIndex, Color, "WindowTitleBar",   &globalConfig->colors.windowTitleBar);
		GetConfig(colorsObjectIndex, Color, "WindowBackground", &globalConfig->colors.windowBackground);
		GetConfig(colorsObjectIndex, Color, "WindowBorder",     &globalConfig->colors.windowBorder);
		GetConfig(colorsObjectIndex, Color, "HoverLine",        &globalConfig->colors.hoverLine);
		GetConfig(colorsObjectIndex, Color, "MarkColor1",       &globalConfig->colors.markColor1);
		GetConfig(colorsObjectIndex, Color, "MarkColor2",       &globalConfig->colors.markColor2);
		GetConfig(colorsObjectIndex, Color, "BannerColor1",     &globalConfig->colors.bannerColor1);
		GetConfig(colorsObjectIndex, Color, "BannerColor2",     &globalConfig->colors.bannerColor2);
		GetConfig(colorsObjectIndex, Color, "Highlight1",       &globalConfig->colors.highlight1);
		GetConfig(colorsObjectIndex, Color, "Highlight2",       &globalConfig->colors.highlight2);
		GetConfig(colorsObjectIndex, Color, "Highlight3",       &globalConfig->colors.highlight3);
		GetConfig(colorsObjectIndex, Color, "Highlight4",       &globalConfig->colors.highlight4);
		GetConfig(colorsObjectIndex, Color, "Highlight5",       &globalConfig->colors.highlight5);
		
		GetConfig(colorsObjectIndex, Color, "Button",               &globalConfig->colors.button);
		GetConfig(colorsObjectIndex, Color, "ButtonText",           &globalConfig->colors.buttonText);
		GetConfig(colorsObjectIndex, Color, "ButtonBorder",         &globalConfig->colors.buttonBorder);
		GetConfig(colorsObjectIndex, Color, "ButtonHover",          &globalConfig->colors.buttonHover);
		GetConfig(colorsObjectIndex, Color, "ButtonHoverText",      &globalConfig->colors.buttonHoverText);
		GetConfig(colorsObjectIndex, Color, "ButtonHoverBorder",    &globalConfig->colors.buttonHoverBorder);
		GetConfig(colorsObjectIndex, Color, "ButtonPress",          &globalConfig->colors.buttonPress);
		GetConfig(colorsObjectIndex, Color, "ButtonPressText",      &globalConfig->colors.buttonPressText);
		GetConfig(colorsObjectIndex, Color, "ButtonPressBorder",    &globalConfig->colors.buttonPressBorder);
		GetConfig(colorsObjectIndex, Color, "ButtonSelected",       &globalConfig->colors.buttonSelected);
		GetConfig(colorsObjectIndex, Color, "ButtonSelectedText",   &globalConfig->colors.buttonSelectedText);
		GetConfig(colorsObjectIndex, Color, "ButtonSelectedBorder", &globalConfig->colors.buttonSelectedBorder);
		GetConfig(colorsObjectIndex, Color, "ButtonReady",          &globalConfig->colors.buttonReady);
		GetConfig(colorsObjectIndex, Color, "ButtonReadyText",      &globalConfig->colors.buttonReadyText);
		GetConfig(colorsObjectIndex, Color, "ButtonReadyBorder",    &globalConfig->colors.buttonReadyBorder);
	}

	PlatformInfo->FreeFileMemoryPntr(&globalConfigFile);
	
	if (parseSuccess)
	{
		StatusSuccess("Global Configuration Loaded Successfully!");
	}
}