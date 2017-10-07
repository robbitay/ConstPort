/*
File:   appDefines.h
Author: Taylor Robbins
Date:   06\08\2017
*/

#ifndef _APP_DEFINES_H
#define _APP_DEFINES_H

#if OSX_COMPILATION
#define GUI_SCALE 2
#else
#define GUI_SCALE 1
#endif

// #define MIN_GUTTER_WIDTH           40 //px
#define MENU_DEFAULT_TITLEBAR_SIZE GUI_SCALE*22 //px
#define CONTEXT_MENU_PADDING       GUI_SCALE*5  //px
#define MAIN_MENU_BUTTON_PADDING   GUI_SCALE*10 //px
#define MAIN_MENU_HEIGHT           GUI_SCALE*(40+MAIN_MENU_BUTTON_PADDING*2) //px
#define COM_MENU_TAB_PADDING       GUI_SCALE*5  //px
#define COM_MENU_TAB_HEIGHT        GUI_SCALE*35 //px
#define COM_MENU_OUTER_PADDING     GUI_SCALE*5  //px
#define COM_MENU_INNER_PADDING     GUI_SCALE*10 //px
#define CONNECT_BUTTON_WIDTH       GUI_SCALE*80 //px
#define CONNECT_BUTTON_HEIGHT      GUI_SCALE*25 //px
#define RX_TX_LED_SIZE             GUI_SCALE*8  //px
#define CLEAR_BUTTON_WIDTH         GUI_SCALE*60 //px
#define CLEAR_BUTTON_HEIGHT        GUI_SCALE*20 //px
#define SAVE_BUTTON_WIDTH          GUI_SCALE*120 //px
#define SAVE_BUTTON_HEIGHT         GUI_SCALE*20 //px
#define MIN_BANNER_HEIGHT          GUI_SCALE*2  //px
#define BANNER_EXPAND_SPEED        0.003f //percentage
#define STATUS_MESSAGE_BUFFER_SIZE 128//bytes

#define TRANSIENT_MAX_NUMBER_MARKS 16
#define INPUT_ARENA_SIZE           1024*1024

#define MAX_JSON_TOKENS            2048
#define GLOBAL_CONFIG_FILEPATH     "Resources/Configuration/GlobalConfig.json"
#define OUTPUT_FILE_NAME           "Output.txt"

#endif // _APP_DEFINES_H