/*
File:   appDefines.h
Author: Taylor Robbins
Date:   06\08\2017
*/

#ifndef _APP_DEFINES_H
#define _APP_DEFINES_H

#define SCROLLBAR_WIDTH                12
#define MIN_SCROLLBAR_HEIGHT           32
#define SCROLLBAR_PADDING              3
#define SCROLL_SPEED_DIVIDER           4
#define MIN_GUTTER_WIDTH               40
#define TAB_WIDTH                      4
#define SCROLL_MULTIPLIER              40
#define MENU_BORDER_WIDTH              1
#define MENU_DEFAULT_TITLEBAR_SIZE     22
#define LINE_SPACING                   2
#define CONTEXT_MENU_PADDING           5

#define Color_Background               NewColor(33, 33, 33, 255)
#define Color_Foreground               {0xFFF8F8F2}
#define Color_UiGray1                  {0xFF494949}
#define Color_UiGray2                  {0xFF404040}
#define Color_UiGray3                  {0xFF303030}
#define Color_UiGray4                  {0xFF101010}
#define Color_UiLightGray1             NewColor(180, 180, 180, 255)
#define Color_Highlight1               NewColor(166, 226, 46, 255)  //Green Color
#define Color_Highlight2               NewColor(102, 217, 239, 255) //Light Blue
#define Color_Highlight3               NewColor(249, 38, 101, 255)  //Red/Magenta
#define Color_Highlight4               NewColor(174, 129, 255, 255) //Purple

#define Color_WindowTitleBar           Color_UiGray3
#define Color_WindowBackground         Color_UiGray1
#define Color_WindowBorder             Color_UiLightGray1

#endif // _APP_DEFINES_H