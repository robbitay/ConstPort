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
	
	bool elapsedBannerEnabled;
	i32  elapsedBannerTime;
	i32  elapsedBannerHeight;
	
	i32  markHeight;
	i32  thickMarkHeight;
	
	i32 statusMessageTime;
	
	struct
	{
		Color_t background;
		Color_t foreground;
		Color_t uiGray1;
		Color_t uiGray2;
		Color_t uiGray3;
		Color_t uiGray4;
		Color_t uiLightGray1;
		Color_t windowTitleBar;
		Color_t windowBackground;
		Color_t windowBorder;
		Color_t markColor1;
		Color_t markColor2;
		Color_t bannerColor1;
		Color_t bannerColor2;
		Color_t highlight1;
		Color_t highlight2;
		Color_t highlight3;
		Color_t highlight4;
		Color_t highlight5;
	} colors;
};

#endif // _APP_CONFIGURATION_H
