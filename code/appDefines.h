/*
File:   appDefines.h
Author: Taylor Robbins
Date:   06\08\2017
*/

#ifndef _APP_DEFINES_H
#define _APP_DEFINES_H

#if OSX_COMPILATION
	#if DOUBLE_RESOLUTION
	#define GUI_SCALE 2.0f
	#else
	#define GUI_SCALE 1.0f
	#endif
#else
#define GUI_SCALE 1.0f
#endif

// #define MIN_GUTTER_WIDTH           40 //px
#define MENU_DEFAULT_TITLEBAR_SIZE 22 //px
#define CONTEXT_MENU_PADDING       5  //px
#define MAIN_MENU_BUTTON_PADDING   10 //px
#define MAIN_MENU_HEIGHT           (40+MAIN_MENU_BUTTON_PADDING*2) //px
#define COM_MENU_TAB_PADDING       5  //px
#define COM_MENU_TAB_HEIGHT        35 //px
#define COM_MENU_OUTER_PADDING     5  //px
#define COM_MENU_INNER_PADDING     10 //px
#define RX_TX_LED_SIZE             8 //px
#define CLEAR_BUTTON_WIDTH         60 //px
#define CLEAR_BUTTON_HEIGHT        20 //px
#define SAVE_BUTTON_WIDTH          120 //px
#define SAVE_BUTTON_HEIGHT         20 //px
#define MIN_BANNER_HEIGHT          2  //px
#define BANNER_EXPAND_SPEED        0.003f //percentage
#define PYTHON_ICON_SIZE           16 //px
#define DEFAULT_MESSAGE_WIDTH      200 //px
#define POPUP_MESSAGE_PADDING      4 //px
#define POPUP_MAX_WIDTH            200 //px

#define COM_MENU_OPEN_SPEED        0.025f //per frame anim
#define COM_LIST_ITEM_PADDING      4
#define REFRESH_BUTTON_SIZE        16 //px

#define STATUS_MESSAGE_BUFFER_SIZE 128//bytes
#define POPUP_MESSAGE_BUFFER_SIZE  512//bytes
#define MAX_INPUT_HISTORY          8
#define INPUT_TEXT_BUFFER_SIZE     256//bytes
#define INPUT_ARENA_SIZE           32*1024 //bytes
#define TRANSIENT_MAX_NUMBER_MARKS 16
#define MAX_JSON_TOKENS            2048
#define MAX_USER_PORT_NAMES        24
#define MAX_USER_PORT_NAME_LENGTH  40 //bytes
#define MAX_COM_READ_LOOPS         5
#define FILE_SEND_MAX_CHUNK_SIZE   32
#define MAX_SAVED_LINES            4095 //lines TODO: Make this into a GlobalConfig option

#define GLOBAL_CONFIG_FILEPATH     "Resources/Configuration/GlobalConfig.json"
#define OUTPUT_FILE_NAME           "Output.txt"

#define REAL_LOGO_HEIGHT 75
#define ABOUT_INFO_TEXT_PADDING 20
#define ABOUT_INFO_FORMAT_STRING "Const Port is an application written by me, Taylor Robbins. "                  \
"It's primarily been developed for my own purposes at work while developing embedded firmware applications.\n\n" \
"I release it under an open source license in the hope that others might also find it useful.\n\n"               \
"If you find any bugs or have any suggestions you can email me at robbitay@gmail.com.\n\n"                       \
"Latest releases can be found on the development blog:\nhttps://www.SilTutorials.com/blog\n\n"                   \
"More info on the HMN project page:\nhttps://ConstPort.Handmade.Network/\n\n"                                    \
"Application Version %u.%u (build %u)\nPlatform Version %u.%u (build %u)\n"

#endif // _APP_DEFINES_H