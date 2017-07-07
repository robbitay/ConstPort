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
	r32 fileHeight;
	r32 scrollOffset;
	r32 scrollOffsetGoto;
	r32 maxScrollOffset;
	r32 scrollPercent;
	
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
	
	char contextStringBuffer[256];
	Texture_t buttonBaseTexture;
	Texture_t buttonDarkenTexture;
	Texture_t buttonHighlightTexture;
	Texture_t buttonTextures[NumMainMenuButtons];
};

#endif // _APP_UI_HANDLER_H