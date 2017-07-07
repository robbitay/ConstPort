#ifndef _APP_UI_HANDLER_H
#define _APP_UI_HANDLER_H

enum
{
	Button_ComPort = 0,
	Button_ConsoleSettings,
	Button_Settings,	
	Button_Help,
	
	NumMainMenuButtons,
	
} MainMenuButtons_t;

struct UiElements_t
{
	v2 screenSize;
	v2 mousePos;
	i32 markIndex;
	r32 mouseScrollbarOffset;
	
	r32 lineHeight;
	v2  fileSize;
	v2  scrollOffset;
	v2  scrollOffsetGoto;
	v2  maxScrollOffset;
	v2  scrollPercent;
	
	bool followingEndOfFile;
	bool startedOnScrollbar;
	bool mouseInMenu;
	
	rec mainMenuRec;
	rec statusBarRec;
	rec scrollBarGutterRec;
	rec gutterRec;
	rec viewRec;
	rec scrollBarRec;
	rec buttonRecs[NumMainMenuButtons];
	rec rxLedRec;
	rec txLedRec;
	rec clearButtonRec;
	
	char contextStringBuffer[256];
	Texture_t buttonBaseTexture;
	Texture_t buttonDarkenTexture;
	Texture_t buttonHighlightTexture;
	Texture_t buttonTextures[NumMainMenuButtons];
};

#endif // _APP_UI_HANDLER_H