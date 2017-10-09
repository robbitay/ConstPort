/*
File:   appDefines.h
Author: Taylor Robbins
Date:   06\08\2017
*/

#ifndef _APP_DEFINES_H
#define _APP_DEFINES_H

#if OSX_COMPILATION
#define GUI_SCALE 2.0f
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
#define CONNECT_BUTTON_WIDTH       80 //px
#define CONNECT_BUTTON_HEIGHT      25 //px
#define RX_TX_LED_SIZE             8  //px
#define CLEAR_BUTTON_WIDTH         60 //px
#define CLEAR_BUTTON_HEIGHT        20 //px
#define SAVE_BUTTON_WIDTH          120 //px
#define SAVE_BUTTON_HEIGHT         20 //px
#define MIN_BANNER_HEIGHT          2  //px
#define BANNER_EXPAND_SPEED        0.003f //percentage
#define STATUS_MESSAGE_BUFFER_SIZE 128//bytes

#define TRANSIENT_MAX_NUMBER_MARKS 16
#define INPUT_ARENA_SIZE           1024*1024

#define MAX_JSON_TOKENS            2048
#define GLOBAL_CONFIG_FILEPATH     "Resources/Configuration/GlobalConfig.json"
#define OUTPUT_FILE_NAME           "Output.txt"
#define MAX_USER_PORT_NAMES        24
#define MAX_USER_PORT_NAME_LENGTH  40

#endif // _APP_DEFINES_H