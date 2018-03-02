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
	MemoryArena_t tempArena;
	u32 appInitTempHighWaterMark;
	
	char* inputArenaBase;
	u32 inputArenaSize;
	
	RenderState_t renderState;
	MenuHandler_t menuHandler;
	GlobalConfig_t globalConfig;
	
	Shader_t simpleShader;
	Shader_t outlineShader;
	Texture_t testTexture;
	Texture_t scrollBarEndcapTexture;
	Texture_t crappyLogo;
	Texture_t pythonIcon;
	Texture_t refreshSprite;
	Texture_t arrowSprite;
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
	
	u64 statusTime;
	u32 statusDuration;
	Color_t statusColor;
	char statusMessage[STATUS_MESSAGE_BUFFER_SIZE];
	
	u64 popupTime;
	u32 popupDuration;
	Color_t popupColor;
	char popupMessage[POPUP_MESSAGE_BUFFER_SIZE];
	bool popupExcused;
	
	RegexList_t regexList;
	
	u32 genericCounter;
	
	bool writeToFile;
	OpenFile_t outputFile;
	
	bool showDebugMenu;
	
	ProgramInstance_t programInstance;
	
	bool buttonHandled[Buttons_NumButtons];
	
	// char lastInputText[INPUT_TEXT_BUFFER_SIZE];
	// u32  lastInputTextLength;
	
	u32 inputHistoryIndex;
	u32 numHistoryItems;
	char inputHistory[MAX_INPUT_HISTORY][INPUT_TEXT_BUFFER_SIZE+1];
	
	const void* activeElement;
	
	TextBox_t inputBox;
	TextBox_t testTextBox;
	Checkbox_t lineWrapCheckbox;
	Checkbox_t hexModeCheckbox;
	Checkbox_t elapsedBannersCheckbox;
	
	ComMenu_t comMenu;
	
	AppThread_t testThread;
	
	u32 droppedFileProgress;
	FileInfo_t droppedFile;
	
	u32 hexLineBreakBeforeCharCount;
	u8* hexLineBreakBeforeChars;
	u32 hexLineBreakAfterCharCount;
	u8* hexLineBreakAfterChars;
	
	u64 lastLineListPush;
	
	bool comRxPaused;
};

#endif // _APP_DATA_H