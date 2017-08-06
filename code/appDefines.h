/*
File:   appDefines.h
Author: Taylor Robbins
Date:   06\08\2017
*/

#ifndef _APP_DEFINES_H
#define _APP_DEFINES_H

#define SCROLLBAR_WIDTH            12 //px
#define MIN_SCROLLBAR_HEIGHT       32 //px
#define SCROLLBAR_PADDING          3  //px
#define SCROLL_SPEED_DIVIDER       4  //divisor
#define MIN_GUTTER_WIDTH           40 //px
#define TAB_WIDTH                  4  //num spaces
#define SCROLL_MULTIPLIER          40 //divisor
#define MENU_BORDER_WIDTH          1  //px
#define MENU_DEFAULT_TITLEBAR_SIZE 22 //px
#define LINE_SPACING               2  //px
#define CONTEXT_MENU_PADDING       5  //px
#define MOUSE_CLICK_TOLERANCE      10 //Distance
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

#define MAX_JSON_TOKENS            1024
#define GLOBAL_CONFIG_FILEPATH     "Resources/Configuration/GlobalConfig.json"

#endif // _APP_DEFINES_H