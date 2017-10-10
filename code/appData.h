/*
File:   appData.h
Author: Taylor Robbins
Date:   06\07\2017
*/

#ifndef _APP_DATA_H
#define _APP_DATA_H

struct AppData_t
{
	MemoryArena_t mainHeap;
	MemoryArena_t inputArena;
	MemoryArena_t tempArena;
	u32 appInitTempHighWaterMark;
	
	RenderState_t renderState;
	MenuHandler_t menuHandler;
	GlobalConfig_t globalConfig;
	
	Shader_t simpleShader;
	Shader_t outlineShader;
	Texture_t testTexture;
	Texture_t scrollBarEndcapTexture;
	Font_t mainFont;
	Font_t uiFont;
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
	BoundedStrList_t availablePorts;
	
	UiElements_t uiElements;
	
	TextLocation_t hoverLocation;
	TextLocation_t selectionStart;
	TextLocation_t selectionEnd;
	
	u32 rxTxShiftCountdown;
	u8 rxShiftRegister;
	u8 txShiftRegister;
	
	RealTime_t statusMessageTime;
	StatusMessage_t statusMessageType;
	char statusMessage[STATUS_MESSAGE_BUFFER_SIZE];
	
	RegexList_t regexList;
	
	u32 genericCounter;
	
	bool writeToFile;
	OpenFile_t outputFile;
	
	bool showDebugMenu;
	
	ProgramInstance_t programInstance;
	
	bool buttonHandled[Buttons_NumButtons];
};

#endif // _APP_DATA_H