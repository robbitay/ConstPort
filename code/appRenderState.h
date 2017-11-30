/*
File:   appRenderState.h
Author: Taylor Robbins
Date:   06\13\2017
*/

#ifndef _APP_RENDER_STATE_H
#define _APP_RENDER_STATE_H

struct RenderState_t
{
	rec viewport;
	
	VertexBuffer_t squareBuffer;
	Texture_t dotTexture;
	Texture_t gradientTexture;
	Texture_t circleTexture;
	
	const Shader_t* boundShader;
	const Font_t* boundFont;
	const Texture_t* boundTexture;
	const Texture_t* boundAlphaTexture;
	const VertexBuffer_t* boundBuffer;
	const FrameBuffer_t* boundFrameBuffer;
	
	mat4 worldMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	bool doGrayscaleGradient;
	bool useAlphaTexture;
	rec sourceRectangle;
	r32 depth;
	r32 circleRadius;
	r32 circleInnerRadius;
	Color_t color;
	Color_t secondaryColor;
};

#endif // _APP_RENDER_STATE_H