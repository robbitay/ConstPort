/*
File:   appDefines.h
Author: Taylor Robbins
Date:   06\08\2017
*/

#ifndef _APP_DEFINES_H
#define _APP_DEFINES_H

#define SCROLLBAR_WIDTH                12 //px
#define MIN_SCROLLBAR_HEIGHT           32 //px
#define SCROLLBAR_PADDING              3  //px
#define SCROLL_SPEED_DIVIDER           4  //divisor
#define MIN_GUTTER_WIDTH               40 //px
#define TAB_WIDTH                      4  //num spaces
#define SCROLL_MULTIPLIER              40 //divisor
#define MENU_BORDER_WIDTH              1  //px
#define MENU_DEFAULT_TITLEBAR_SIZE     22 //px
#define LINE_SPACING                   2  //px
#define CONTEXT_MENU_PADDING           5  //px
#define MARK_SIZE                      1  //px
#define THICK_MARK_SIZE                3  //px
#define MOUSE_CLICK_TOLERANCE          10 //Distance
#define MIN_SECONDS_STATUS_BAR         1  //seconds
#define MAIN_MENU_BUTTON_PADDING       10  //px
#define MAIN_MENU_HEIGHT               (40+MAIN_MENU_BUTTON_PADDING*2) //px
#define COM_MENU_TAB_PADDING           5
#define COM_MENU_TAB_HEIGHT            35
#define COM_MENU_OUTER_PADDING         5
#define COM_MENU_INNER_PADDING         10
#define CONNECT_BUTTON_WIDTH           80
#define CONNECT_BUTTON_HEIGHT          25
#define RX_TX_LED_SIZE                 8

#define Color_Background               NewColor(33, 33, 33, 255)
#define Color_Foreground               {0xFFF8F8F2}
#define Color_UiGray1                  {0xFF494949}
#define Color_UiGray2                  {0xFF404040}
#define Color_UiGray3                  {0xFF303030}
#define Color_UiGray4                  {0xFF101010}
#define Color_UiLightGray1             NewColor(180, 180, 180, 255)
#define Color_Highlight1               NewColor(117, 113, 94, 255)  //Comment Color
#define Color_Highlight2               NewColor(166, 226, 46, 255)  //Green Color
#define Color_Highlight3               NewColor(249, 38, 101, 255)  //Red/Magenta
#define Color_Highlight4               NewColor(174, 129, 255, 255) //Purple
#define Color_Highlight5               NewColor(102, 217, 239, 255) //Light Blue

#define Color_WindowTitleBar           Color_UiGray3
#define Color_WindowBackground         Color_UiGray1
#define Color_WindowBorder             Color_UiLightGray1
#define Color_MarkColor                NewColor(255, 255, 255, 128)

#endif // _APP_DEFINES_H