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
	rec connectRec;
	rec diconnectRec;
	
	u32 numComListItems;
	i32 comListSelectedIndex;
	r32 comListHeight;
	r32 comListScroll;
	r32 comListScrollGoto;
};

#endif //  _APP_COM_MENU_H
