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
	ConfigurationInt32 ("rx_tx_led_ring_size",         rxTxLedRingSize,        2);
	ConfigurationBool  ("circular_rx_led",             circularRxLed,          true);
	ConfigurationBool  ("circular_tx_led",             circularTxLed,          false);
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
	
	// 0x75715EFF - Comment
	// 0xA6E22EFF - Green
	// 0xF92665FF - Red/Magenta
	// 0xAE81FFFF - Purple
	// 0x66D9EFFF - Light Blue
	// 0x494949FF - Ui Gray 1
	// 0x404040FF - Ui Gray 2
	// 0x303030FF - Ui Gray 3
	// 0x101010FF - Ui Gray 4
	// 0xB4B4B4FF - Ui Light Gray 1
	ConfigurationColor("text_default",           textDefault,          {0xF8F8F2FF});
	ConfigurationColor("text_background",        textBackground,       {0x212121FF});
	ConfigurationColor("hover_background",       hoverBackground,      {0x000000FF});
	ConfigurationColor("selection_1",            selection1,           {0xFFFFFFFF});
	ConfigurationColor("selection_2",            selection2,           {0x808080FF});
	ConfigurationColor("banner_1",               banner1,              {0x494949FF});
	ConfigurationColor("banner_2",               banner2,              {0x303030FF});
	ConfigurationColor("banner_text",            bannerText,           {0xF8F8F2FF});
	ConfigurationColor("line_mark_hover",        lineMarkHover,        {0xA6E22EFF}); //Green
	ConfigurationColor("line_mark_1",            lineMark1,            {0xFFFFFFFF});
	ConfigurationColor("line_mark_2",            lineMark2,            {0xFFFFFF80});
	ConfigurationColor("hover_cursor_1",         hoverCursor1,         {0xFFFFFFFF});
	ConfigurationColor("hover_cursor_2",         hoverCursor2,         {0xFFFFFF80});
	ConfigurationColor("end_cursor_1",           endCursor1,           {0xFFFFFFFF});
	ConfigurationColor("end_cursor_2",           endCursor2,           {0xFFFFFF80});
	
	ConfigurationColor("ui_text",                uiText,               {0xF8F8F2FF});
	ConfigurationColor("debug_message",          debugMessage,         {0x75715EFF}); //Comment
	ConfigurationColor("success_message",        successMessage,       {0xA6E22EFF}); //Green
	ConfigurationColor("info_message",           infoMessage,          {0x66D9EFFF}); //Light Blue
	ConfigurationColor("error_message",          errorMessage,         {0xF92665FF}); //Red/Magenta
	ConfigurationColor("line_numbers",           lineNumbers,          {0xF8F8F2FF});
	ConfigurationColor("scrollbar_outline",      scrollbarOutline,     {0x000000FF});
	ConfigurationColor("scrollbar_1",            scrollbar1,           {0x494949FF});
	ConfigurationColor("scrollbar_2",            scrollbar2,           {0x303030FF});
	ConfigurationColor("scrollbar_back_1",       scrollbarBack1,       {0x101010FF});
	ConfigurationColor("scrollbar_back_2",       scrollbarBack2,       {0x101010FF});
	ConfigurationColor("gutter_1",               gutter1,              {0x494949FF});
	ConfigurationColor("gutter_2",               gutter2,              {0x303030FF});
	ConfigurationColor("status_bar_1",           statusBar1,           {0x303030FF});
	ConfigurationColor("status_bar_2",           statusBar2,           {0x303030FF});
	ConfigurationColor("main_menu_1",            mainMenu1,            {0x494949FF});
	ConfigurationColor("main_menu_2",            mainMenu2,            {0x303030FF});
	
	ConfigurationColor("window_outline",         windowOutline,        {0xFFFFFFFF});
	ConfigurationColor("window_titlebar",        windowTitlebar,       {0x101010FF});
	ConfigurationColor("window_background_1",    windowBackground1,    {0x494949FF});
	ConfigurationColor("window_background_2",    windowBackground2,    {0x303030FF});
	ConfigurationColor("window_exit_back",       windowExitBack,       {0xFF0000FF});
	ConfigurationColor("window_exit_icon",       windowExitIcon,       {0xFFFFFFFF});
	ConfigurationColor("context_menu_text",      contextMenuText,      {0xF8F8F2FF});
	
	ConfigurationColor("receive_led",            receiveLed,           {0x000000FF});
	ConfigurationColor("receive_led_active",     receiveLedActive,     {0xF92665FF}); //Red/Magenta
	ConfigurationColor("receive_led_ring",       receiveLedRing,       {0xF92665FF}); //Red/Magenta
	ConfigurationColor("transmit_led",           transmitLed,          {0x000000FF});
	ConfigurationColor("transmit_led_active",    transmitLedActive,    {0xAE81FFFF}); //Purple
	ConfigurationColor("transmit_led_ring",      transmitLedRing,      {0xAE81FFFF}); //Purple
	
	ConfigurationColor("button",                 button,               {0xFFFFFFFF});
	ConfigurationColor("button_text",            buttonText,           {0x000000FF});
	ConfigurationColor("button_border",          buttonBorder,         {0x000000FF});
	
	ConfigurationColor("button_hover",           buttonHover,          {0xB4B4B4FF}); //UiLightGray1
	ConfigurationColor("button_hover_text",      buttonHoverText,      {0xFFFFFFFF});
	ConfigurationColor("button_hover_border",    buttonHoverBorder,    {0xFFFFFFFF});
	
	ConfigurationColor("button_press",           buttonPress,          {0xFF0604FF}); //Red
	ConfigurationColor("button_press_text",      buttonPressText,      {0xFFFFFFFF});
	ConfigurationColor("button_press_border",    buttonPressBorder,    {0xFFFFFFFF});
	
	ConfigurationColor("button_selected",        buttonSelected,       {0x02AEFFFF}); //Turquoise
	ConfigurationColor("button_selected_text",   buttonSelectedText,   {0x000000FF});
	ConfigurationColor("button_selected_border", buttonSelectedBorder, {0x000000FF});
	
	ConfigurationColor("button_ready",           buttonReady,          {0xCFFF01FF}); //Yellow
	ConfigurationColor("button_ready_text",      buttonReadyText,      {0x000000FF});
	ConfigurationColor("button_ready_border",    buttonReadyBorder,    {0x000000FF});
	
	ConfigurationColor("button_disabled",        buttonDisabled,       {0x494949FF});
	ConfigurationColor("button_disabled_text",   buttonDisabledText,   {0x000000FF});
	ConfigurationColor("button_disabled_border", buttonDisabledBorder, {0x000000FF});
	
	ConfigurationColor("main_menu_button",              mainMenuButton,            {0x494949FF});
	ConfigurationColor("main_menu_button_icon",         mainMenuButtonIcon,        {0xB4B4B4FF});
	ConfigurationColor("main_menu_button_border",       mainMenuButtonBorder,      {0xB4B4B4FF});
	ConfigurationColor("main_menu_button_hover",        mainMenuButtonHover,       {0x494949FF});
	ConfigurationColor("main_menu_button_hover_icon",   mainMenuButtonHoverIcon,   {0x66D9EFFF}); //Light Blue
	ConfigurationColor("main_menu_button_hover_border", mainMenuButtonHoverBorder, {0x66D9EFFF}); //Light Blue
	ConfigurationColor("main_menu_button_press",        mainMenuButtonPress,       {0x494949FF});
	ConfigurationColor("main_menu_button_press_icon",   mainMenuButtonPressIcon,   {0xF92665FF}); //Red/Magenta
	ConfigurationColor("main_menu_button_press_border", mainMenuButtonPressBorder, {0xF92665FF}); //Red/Magenta
	
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

