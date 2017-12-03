/*
File:   appComMenu.h
Author: Taylor Robbins
Date:   11\28\2017
*/

#ifndef _APP_COM_MENU_H
#define _APP_COM_MENU_H

struct ComMenu_t
{
	r32 animPercent;
	bool open;
	
	rec drawRec;
	rec comListRec;
	rec baudRatesRec;
	rec numBitsRec;
	rec parityRec;
	rec stopBitsRec;
	rec connectRec;
	rec disconnectRec;
	rec refreshRec;
	
	u32 numComListItems;
	i32 comListOpenIndex;
	i32 comListSelectedIndex;
	r32 comListHeight;
	r32 comListScroll;
	r32 comListScrollGoto;
	
	u32 baudRateSelection;
	r32 baudRatesHeight;
	r32 baudRateScroll;
	r32 baudRateScrollGoto;
	
	u32 numBitsSelection;
	u32 paritySelection;
	u32 stopBitsSelection;
};

#endif //  _APP_COM_MENU_H
