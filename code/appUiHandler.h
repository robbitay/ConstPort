#ifndef _APP_UI_HANDLER_H
#define _APP_UI_HANDLER_H

enum
{
	Button_ComPort = 0,
	// Button_ConsoleSettings,
	Button_Settings,	
	Button_About,
	
	NumMainMenuButtons,
	
} MainMenuButtons_t;

struct UiElements_t
{
	i32 markIndex;
	r32 mouseScrollbarOffset;
	
	r32 lineHeight;
	i32 firstRenderLine;
	r32 firstRenderLineOffset;
	TextLocation_t mouseTextLocation;
	TextLocation_t mouseTextLineLocation;
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
	rec saveButtonRec;
	rec gotoEndButtonRec;
	rec textInputRec;
	rec sendButtonRec;
	
	const char* contextString;
	Texture_t buttonBaseTexture;
	Texture_t buttonDarkenTexture;
	Texture_t buttonHighlightTexture;
	Texture_t buttonTextures[NumMainMenuButtons];
};

struct Checkbox_t
{
	rec drawRec;
	char label[256];
	Color_t activeColor;
	
	bool enabled;
	bool checked;
	u64  changeTime;
	bool mouseHasLeft;
};

Checkbox_t NewCheckbox(rec drawRec, const char* label, Color_t activeColor)
{
	Checkbox_t result = {};
	
	result.enabled = true;
	result.mouseHasLeft = true;
	result.drawRec = drawRec;
	result.activeColor = activeColor;
	BufferPrint(result.label, label);
	
	return result;
}

#endif // _APP_UI_HANDLER_H