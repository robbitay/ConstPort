/*
File:   appConfigOptions.h
Author: Taylor Robbins
Date:   09\26\2017
*/

#if defined(INCLUDE_GLOBAL_OPTIONS)
	
	#if defined(ConfigurationBool) && defined(ConfigurationInt32) && defined(ConfigurationColor) && defined(ConfigurationString)
	
	ConfigurationInt32 ("FontSize",                fontSize,               16);
	ConfigurationInt32 ("LineSpacing",             lineSpacing,            2);
	ConfigurationInt32 ("TabWidth",                tabWidth,               4);
	ConfigurationBool  ("ElapsedBannerEnabled",    elapsedBannerEnabled,   true);
	ConfigurationInt32 ("ElapsedBannerTime",       elapsedBannerTime,      15);
	ConfigurationInt32 ("ElapsedBannerHeight",     elapsedBannerHeight,    25);
	ConfigurationInt32 ("MarkHeight",              markHeight,             1);
	ConfigurationInt32 ("ThickMarkHeight",         thickMarkHeight,        5);
	ConfigurationInt32 ("StatusMessageTime",       statusMessageTime,      3);
	ConfigurationInt32 ("MouseClickTolerance",     mouseClickTolerance,    10);
	ConfigurationInt32 ("MenuBorderThickness",     menuBorderThickness,    1);
	ConfigurationInt32 ("ScrollbarWidth",          scrollbarWidth,         12);
	ConfigurationInt32 ("ScrollbarPadding",        scrollbarPadding,       3);
	ConfigurationInt32 ("MinScrollbarHeight",      minScrollbarHeight,     32);
	ConfigurationInt32 ("MinGutterWidth",          minGutterWidth,         40);
	ConfigurationInt32 ("ScrollMultiplier",        scrollMultiplier,       40);
	ConfigurationInt32 ("ViewSpeedDivider",        viewSpeedDivider,       4);
	ConfigurationBool  ("ShowTextCursor",          showTextCursor,         false);
	ConfigurationBool  ("ShowComNameInStatusBar",  showComNameInStatusBar, true);
	ConfigurationBool  ("ShowLineNumbers",         showLineNumbers,        true);
	ConfigurationBool  ("ShowHoverCursor",         showHoverCursor,        false);
	ConfigurationBool  ("HighlightHoverLine",      highlightHoverLine,     true);
	ConfigurationInt32 ("RxTxLedDelay",            rxTxLedDelay,           4);
	
	//Regex Names
	ConfigurationString("GenericCountRegexName",    genericCountRegexName,    nullptr);
	ConfigurationString("MarkLineRegexName",        markLineRegexName,        nullptr);
	ConfigurationString("Highlight1RegexName",      highlight1RegexName,      nullptr);
	ConfigurationString("Highlight2RegexName",      highlight2RegexName,      nullptr);
	ConfigurationString("Highlight3RegexName",      highlight3RegexName,      nullptr);
	ConfigurationString("Highlight4RegexName",      highlight4RegexName,      nullptr);
	ConfigurationString("Highlight5RegexName",      highlight5RegexName,      nullptr);
	ConfigurationString("BackgroundColorRegexName", backgroundColorRegexName, nullptr);
	
	#else
	#error ConfigurationBool, ConfigurationInt32, ConfigurationColor, and ConfigurationString \
	macros must be defined before every inclusion of appConfigOptions.h with INCLUDE_GLOBAL_OPTIONS defined
	#endif
	
	#undef INCLUDE_GLOBAL_OPTIONS
#endif // INCLUDE_GLOBAL_OPTIONS

#if defined(INCLUDE_COLOR_OPTIONS)
	
	#if defined(ConfigurationColor)
	
	ConfigurationColor("Background",           background,           NewColor(33, 33, 33, 255));
	ConfigurationColor("Foreground",           foreground,           {0xFFF8F8F2});
	ConfigurationColor("Highlight1",           highlight1,           NewColor(117, 113, 94, 255));  //Comment Color
	ConfigurationColor("Highlight2",           highlight2,           NewColor(166, 226, 46, 255));  //Green Color
	ConfigurationColor("Highlight3",           highlight3,           NewColor(249, 38, 101, 255));  //Red/Magenta
	ConfigurationColor("Highlight4",           highlight4,           NewColor(174, 129, 255, 255)); //Purple
	ConfigurationColor("Highlight5",           highlight5,           NewColor(102, 217, 239, 255)); //Light Blue
	ConfigurationColor("Selection1",           selection1,           {Color_White});
	ConfigurationColor("Selection2",           selection2,           {Color_White});
	ConfigurationColor("UiGray1",              uiGray1,              {0xFF494949});
	ConfigurationColor("UiGray2",              uiGray2,              {0xFF404040});
	ConfigurationColor("UiGray3",              uiGray3,              {0xFF303030});
	ConfigurationColor("UiGray4",              uiGray4,              {0xFF101010});
	ConfigurationColor("UiLightGray1",         uiLightGray1,         NewColor(180, 180, 180, 255));
	ConfigurationColor("ReceiveLed",           receiveLed,           globalConfig->colors.highlight3);
	ConfigurationColor("TransmitLed",          transmitLed,          globalConfig->colors.highlight4);
	ConfigurationColor("WindowTitleBar",       windowTitleBar,       globalConfig->colors.uiGray3);
	ConfigurationColor("WindowBackground",     windowBackground,     globalConfig->colors.uiGray1);
	ConfigurationColor("WindowBorder",         windowBorder,         globalConfig->colors.uiLightGray1);
	ConfigurationColor("HoverLine",            hoverLine,            globalConfig->colors.uiGray3);
	ConfigurationColor("MarkColor1",           markColor1,           NewColor(255, 255, 255, 128));
	ConfigurationColor("MarkColor2",           markColor2,           NewColor(255, 255, 255, 0));
	ConfigurationColor("BannerColor1",         bannerColor1,         globalConfig->colors.uiGray1);
	ConfigurationColor("BannerColor2",         bannerColor2,         globalConfig->colors.uiGray3);
	ConfigurationColor("Button",               button,               {Color_White});
	ConfigurationColor("ButtonText",           buttonText,           {Color_Black});
	ConfigurationColor("ButtonBorder",         buttonBorder,         {Color_Black});
	ConfigurationColor("ButtonHover",          buttonHover,          globalConfig->colors.uiLightGray1);
	ConfigurationColor("ButtonHoverText",      buttonHoverText,      {Color_Black});
	ConfigurationColor("ButtonHoverBorder",    buttonHoverBorder,    {Color_Black});
	ConfigurationColor("ButtonPress",          buttonPress,          globalConfig->colors.highlight3);
	ConfigurationColor("ButtonPressText",      buttonPressText,      {Color_White});
	ConfigurationColor("ButtonPressBorder",    buttonPressBorder,    {Color_White});
	ConfigurationColor("ButtonSelected",       buttonSelected,       globalConfig->colors.highlight4);
	ConfigurationColor("ButtonSelectedText",   buttonSelectedText,   {Color_White});
	ConfigurationColor("ButtonSelectedBorder", buttonSelectedBorder, {Color_White});
	ConfigurationColor("ButtonReady",          buttonReady,          globalConfig->colors.highlight2);
	ConfigurationColor("ButtonReadyText",      buttonReadyText,      {Color_White});
	ConfigurationColor("ButtonReadyBorder",    buttonReadyBorder,    {Color_White});
	
	#else
	#error ConfigurationColor macro must be defined before every inclusion of appConfigOptions.h \
	with INCLUDE_COLOR_OPTIONS defined
	#endif
	
	#undef INCLUDE_COLOR_OPTIONS
#endif // INCLUDE_COLOR_OPTIONS

//Undefine all of the macros
#ifdef ConfigurationBool
	#undef ConfigurationBool
#endif

#ifdef ConfigurationInt32
	#undef ConfigurationInt32
#endif

#ifdef ConfigurationColor
	#undef ConfigurationColor
#endif

#ifdef ConfigurationString
	#undef ConfigurationString
#endif

