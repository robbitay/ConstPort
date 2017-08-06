/*
File:   appConfiguration.h
Author: Taylor Robbins
Date:   08\02\2017
*/

#ifndef _APP_CONFIGURATION_H
#define _APP_CONFIGURATION_H

struct GlobalConfig_t
{
	struct
	{
		Color_t background;
		Color_t highlight1;
		Color_t highlight2;
		Color_t highlight3;
		Color_t highlight4;
		Color_t highlight5;
	} colors;
	
	bool elapsedBannerEnabled;
	i32 elapsedBannerTime;
};

#endif // _APP_CONFIGURATION_H
