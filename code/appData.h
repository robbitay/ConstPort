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
	
	Shader_t simpleShader;
	Texture_t testTexture;
	Texture_t dotTexture;
	Texture_t gradientTexture;
	Texture_t scrollBarEndcapTexture;
	Texture_t circleTexure;
	Texture_t glyphTexture;
	Font_t testFont;
	VertexBuffer_t squareBuffer;
	VertexBuffer_t triangleBuffer;
	
	LineList_t lineList;
	
	r32 scrollOffset;
	r32 mouseScrollbarOffset;
	bool startedOnScrollbar;
	
	ComPort_t comPort;
};

#endif // _APP_DATA_H