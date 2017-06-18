#ifndef _APP_UI_HANDLER_H
#define _APP_UI_HANDLER_H


struct UiElements_t
{
	v2 screenSize;
	v2 mousePos;
	r32 mouseScrollbarOffset;
	
	r32 lineHeight;
	r32 fileHeight;
	r32 scrollOffset;
	r32 scrollOffsetGoto;
	r32 maxScrollOffset;
	r32 scrollPercent;
	
	bool followingEndOfFile;
	bool startedOnScrollbar;
	
	rec statusBarRec;
	rec scrollBarGutterRec;
	rec gutterRec;
	rec viewRec;
	rec scrollBarRec;
	
	char contextStringBuffer[256];
};

#endif // _APP_UI_HANDLER_H