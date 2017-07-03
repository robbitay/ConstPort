/*
File:   appData.h
Author: Taylor Robbins
Date:   06\07\2017
*/

#ifndef _APP_DATA_H
#define _APP_DATA_H

struct AppData_t
{
	MemoryArena_t memArena;
	RenderState_t renderState;
	MenuHandler_t menuHandler;
	
	Shader_t simpleShader;
	Shader_t outlineShader;
	Texture_t testTexture;
	Texture_t scrollBarEndcapTexture;
	Font_t testFont;
	FrameBuffer_t frameBuffer;
	Texture_t frameTexture;
	
	LineList_t lineList;
	
	r32 scrollOffset;
	r32 scrollOffsetGoto;
	
	r32 mouseScrollbarOffset;
	bool startedOnScrollbar;
	
	ComPort_t comMenuOptions;
	ComPort_t comPort;
	
	//NOTE: This list will not contain the comPort that is
	//		currently connected.
	u32 numComPortsAvailable;
	bool availableComPorts[NumComPorts];
	
	UiElements_t uiElements;
	
	
	TextLocation_t hoverLocation;
	TextLocation_t selectionStart;
	TextLocation_t selectionEnd;
};

#endif // _APP_DATA_H