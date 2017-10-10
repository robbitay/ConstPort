/*
File:   appConfigOptions.h
Author: Taylor Robbins
Date:   09\26\2017
*/

#if defined(INCLUDE_GLOBAL_OPTIONS)
	
	#if defined(ConfigurationBool) && defined(ConfigurationInt32) && defined(ConfigurationColor) && defined(ConfigurationString)
	
	ConfigurationInt32 ("main_font_size",              mainFontSize,           16);
	ConfigurationInt32 ("ui_font_size",                uiFontSize,             12);
	ConfigurationInt32 ("line_spacing",                lineSpacing,            2);
	ConfigurationInt32 ("tab_width",                   tabWidth,               4);
	ConfigurationBool  ("elapsed_banner_enabled",      elapsedBannerEnabled,   true);
	ConfigurationInt32 ("elapsed_banner_time",         elapsedBannerTime,      15);
	ConfigurationInt32 ("elapsed_banner_height",       elapsedBannerHeight,    25);
	ConfigurationInt32 ("mark_height",                 markHeight,             1);
	ConfigurationInt32 ("thick_mark_height",           thickMarkHeight,        5);
	ConfigurationInt32 ("status_message_time",         statusMessageTime,      3);
	ConfigurationInt32 ("mouse_click_tolerance",       mouseClickTolerance,    10);
	ConfigurationInt32 ("menu_border_thickness",       menuBorderThickness,    1);
	ConfigurationInt32 ("scrollbar_width",             scrollbarWidth,         12);
	ConfigurationInt32 ("scrollbar_padding",           scrollbarPadding,       3);
	ConfigurationInt32 ("min_scrollbar_height",        minScrollbarHeight,     32);
	ConfigurationInt32 ("min_gutter_width",            minGutterWidth,         40);
	ConfigurationInt32 ("scroll_multiplier",           scrollMultiplier,       40);
	ConfigurationInt32 ("view_speed_divider",          viewSpeedDivider,       4);
	ConfigurationBool  ("show_text_cursor",            showTextCursor,         false);
	ConfigurationBool  ("show_com_name_in_status_bar", showComNameInStatusBar, true);
	ConfigurationBool  ("show_line_numbers",           showLineNumbers,        true);
	ConfigurationBool  ("show_hover_cursor",           showHoverCursor,        false);
	ConfigurationBool  ("highlight_hover_line",        highlightHoverLine,     true);
	ConfigurationInt32 ("rx_tx_led_delay",             rxTxLedDelay,           4);
	ConfigurationBool  ("auto_echo_input",             autoEchoInput,          true);
	
	//Python Related Options
	ConfigurationBool  ("python_script_enabled",   pythonScriptEnabled, false);
	ConfigurationBool  ("auto_run_python",         autoRunPython,       false);
	ConfigurationBool  ("send_com_data_to_python", sendComDataToPython, true);
	ConfigurationBool  ("also_show_com_data",      alsoShowComData,     false);
	ConfigurationBool  ("send_input_to_python",    sendInputToPython,   false);
	ConfigurationBool  ("also_send_input_to_com",  alsoSendInputToCom,  false);
	ConfigurationBool  ("show_python_output",      showPythonOutput,    true);
	ConfigurationString("python_script",           pythonScript,        nullptr);
	
	//Regex Names
	ConfigurationString("generic_count_regex_name",    genericCountRegexName,    nullptr);
	ConfigurationString("mark_line_regex_name",        markLineRegexName,        nullptr);
	ConfigurationString("highlight1_regex_name",       highlight1RegexName,      nullptr);
	ConfigurationString("highlight2_regex_name",       highlight2RegexName,      nullptr);
	ConfigurationString("highlight3_regex_name",       highlight3RegexName,      nullptr);
	ConfigurationString("highlight4_regex_name",       highlight4RegexName,      nullptr);
	ConfigurationString("highlight5_regex_name",       highlight5RegexName,      nullptr);
	ConfigurationString("background_color_regex_name", backgroundColorRegexName, nullptr);
	
	#else
	#error ConfigurationBool, ConfigurationInt32, ConfigurationColor, and ConfigurationString \
	macros must be defined before every inclusion of appConfigOptions.h with INCLUDE_GLOBAL_OPTIONS defined
	#endif
	
	#undef INCLUDE_GLOBAL_OPTIONS
#endif // INCLUDE_GLOBAL_OPTIONS

#if defined(INCLUDE_COLOR_OPTIONS)
	
	#if defined(ConfigurationColor)
	
	ConfigurationColor("background",             background,           NewColor(33, 33, 33, 255));
	ConfigurationColor("foreground",             foreground,           {0xFFF8F8F2});
	ConfigurationColor("highlight_1",            highlight1,           NewColor(117, 113, 94, 255));  //Comment Color
	ConfigurationColor("highlight_2",            highlight2,           NewColor(166, 226, 46, 255));  //Green Color
	ConfigurationColor("highlight_3",            highlight3,           NewColor(249, 38, 101, 255));  //Red/Magenta
	ConfigurationColor("highlight_4",            highlight4,           NewColor(174, 129, 255, 255)); //Purple
	ConfigurationColor("highlight_5",            highlight5,           NewColor(102, 217, 239, 255)); //Light Blue
	ConfigurationColor("selection_1",            selection1,           {Color_White});
	ConfigurationColor("selection_2",            selection2,           {Color_White});
	ConfigurationColor("regex_match_background", regexMatchBackground, {Color_Black});
	ConfigurationColor("ui_gray_1",              uiGray1,              {0xFF494949});
	ConfigurationColor("ui_gray_2",              uiGray2,              {0xFF404040});
	ConfigurationColor("ui_gray_3",              uiGray3,              {0xFF303030});
	ConfigurationColor("ui_gray_4",              uiGray4,              {0xFF101010});
	ConfigurationColor("ui_light_gray_1",        uiLightGray1,         NewColor(180, 180, 180, 255));
	ConfigurationColor("receive_led",            receiveLed,           globalConfig->colors.highlight3);
	ConfigurationColor("transmit_led",           transmitLed,          globalConfig->colors.highlight4);
	ConfigurationColor("window_title_bar",       windowTitleBar,       globalConfig->colors.uiGray3);
	ConfigurationColor("window_background",      windowBackground,     globalConfig->colors.uiGray1);
	ConfigurationColor("window_border",          windowBorder,         globalConfig->colors.uiLightGray1);
	ConfigurationColor("hover_line",             hoverLine,            globalConfig->colors.uiGray3);
	ConfigurationColor("mark_color_1",           markColor1,           NewColor(255, 255, 255, 128));
	ConfigurationColor("mark_color_2",           markColor2,           NewColor(255, 255, 255, 0));
	ConfigurationColor("banner_color_1",         bannerColor1,         globalConfig->colors.uiGray1);
	ConfigurationColor("banner_color_2",         bannerColor2,         globalConfig->colors.uiGray3);
	ConfigurationColor("button",                 button,               {Color_White});
	ConfigurationColor("button_text",            buttonText,           {Color_Black});
	ConfigurationColor("button_border",          buttonBorder,         {Color_Black});
	ConfigurationColor("button_hover",           buttonHover,          globalConfig->colors.uiLightGray1);
	ConfigurationColor("button_hover_text",      buttonHoverText,      {Color_Black});
	ConfigurationColor("button_hover_border",    buttonHoverBorder,    {Color_Black});
	ConfigurationColor("button_press",           buttonPress,          globalConfig->colors.highlight3);
	ConfigurationColor("button_press_text",      buttonPressText,      {Color_White});
	ConfigurationColor("button_press_border",    buttonPressBorder,    {Color_White});
	ConfigurationColor("button_selected",        buttonSelected,       globalConfig->colors.highlight4);
	ConfigurationColor("button_selected_text",   buttonSelectedText,   {Color_White});
	ConfigurationColor("button_selected_border", buttonSelectedBorder, {Color_White});
	ConfigurationColor("button_ready",           buttonReady,          globalConfig->colors.highlight2);
	ConfigurationColor("button_ready_text",      buttonReadyText,      {Color_White});
	ConfigurationColor("button_ready_border",    buttonReadyBorder,    {Color_White});
	
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

