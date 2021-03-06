/*
File:   appMenuHandler.h
Author: Taylor Robbins
Date:   06\15\2017
*/

#ifndef _APP_MENU_HANDLER_H
#define _APP_MENU_HANDLER_H

struct Menu_t
{
	LinkHeader_t header;
	
	char name[64];
	bool alive;
	bool show;
	bool allowMovement;
	bool allowResize;
	rec drawRec;
	r32 titleBarSize;
	
	//NOTE: This can be a pointer to anything the update and render functions require
	void* specialPntr;
	
	void* updateFunctionPntr;
	void* renderFunctionPntr;
	
	rec titleBarRec;
	rec usableRec;
};

struct MenuHandler_t
{
	MemoryArena_t* memArena;
	LinkedList_t menuList;
	Texture_t closeTexture;
	
	bool movingMenu;
	v2 grabMenuOffset;
	bool resizingMenuVert;
	bool resizingMenuHor;
	i32 activeMenuIndex;
};

typedef void MenuUpdate_f(MenuHandler_t* menuHandler, Menu_t* menu);
typedef void MenuRender_f(MenuHandler_t* menuHandler, Menu_t* menu);

#endif // _APP_MENU_HANDLER_H