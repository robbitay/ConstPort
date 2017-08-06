/*
File:   appConfiguration.h
Author: Taylor Robbins
Date:   08\02\2017
*/

#ifndef _APP_CONFIGURATION_H
#define _APP_CONFIGURATION_H

struct GlobalConfig_t
{
	i32 fontSize;
	i32 lineSpacing;
	i32  tabWidth;
	
	bool elapsedBannerEnabled;
	i32  elapsedBannerTime;
	i32  elapsedBannerHeight;
	i32  markHeight;
	i32  thickMarkHeight;
	i32  statusMessageTime;
	i32  mouseClickTolerance;
	i32  menuBorderThickness;
	i32  scrollbarWidth;
	i32  scrollbarPadding;
	i32  minScrollbarHeight;
	i32  minGutterWidth;
	i32  scrollMultiplier;
	i32  viewSpeedDivider;
	bool showTextCursor;
	bool showComNameInStatusBar;
	bool showLineNumbers;
	bool showHoverCursor;
	bool highlightHoverLine;
	i32  rxTxLedDelay;
	
	struct
	{
		Color_t background;
		Color_t foreground;
		Color_t selection1;
		Color_t selection2;
		Color_t uiGray1;
		Color_t uiGray2;
		Color_t uiGray3;
		Color_t uiGray4;
		Color_t uiLightGray1;
		Color_t receiveLed;
		Color_t transmitLed;
		Color_t windowTitleBar;
		Color_t windowBackground;
		Color_t windowBorder;
		Color_t hoverLine;
		Color_t markColor1;
		Color_t markColor2;
		Color_t bannerColor1;
		Color_t bannerColor2;
		Color_t highlight1;
		Color_t highlight2;
		Color_t highlight3;
		Color_t highlight4;
		Color_t highlight5;
		
		Color_t button;
		Color_t buttonText;
		Color_t buttonBorder;
		Color_t buttonHover;
		Color_t buttonHoverText;
		Color_t buttonHoverBorder;
		Color_t buttonPress;
		Color_t buttonPressText;
		Color_t buttonPressBorder;
		Color_t buttonSelected;
		Color_t buttonSelectedText;
		Color_t buttonSelectedBorder;
		Color_t buttonReady;
		Color_t buttonReadyText;
		Color_t buttonReadyBorder;
	} colors;
};

#endif // _APP_CONFIGURATION_H
