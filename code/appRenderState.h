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
	
	const Shader_t* boundShader;
	const Font_t* boundFont;
	const Texture_t* boundTexture;
	const Texture_t* boundAlphaTexture;
	const VertexBuffer_t* boundBuffer;
	const FrameBuffer_t* boundFrameBuffer;
	mat4 worldMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	
	VertexBuffer_t squareBuffer;
	Texture_t dotTexture;
	Texture_t gradientTexture;
	Texture_t circleTexture;
	
	void Initialize(PlatformInfo_t* PlatformInfo);
	
	//+--------------------------------------+
	//|       State Change Functions         |
	//+--------------------------------------+
	void BindShader(const Shader_t* shaderPntr);
	void UpdateShader();
	void BindFont(const Font_t* fontPntr);
	void BindTexture(const Texture_t* texturePntr);
	void BindAlphaTexture(const Texture_t* texturePntr);
	void DisableAlphaTexture();
	void BindBuffer(const VertexBuffer_t* vertBuffer);
	void BindFrameBuffer(const FrameBuffer_t* frameBuffer);
	void SetWorldMatrix(const mat4& worldMatrix);
	void SetViewMatrix(const mat4& viewMatrix);
	void SetProjectionMatrix(const mat4& projectionMatrix);
	void SetViewport(rec viewport);
	void SetColor(Color_t color);
	void SetSecondaryColor(Color_t color);
	void SetSourceRectangle(rec sourceRectangle);
	void SetGradientEnabled(bool doGradient);
	void SetCircleRadius(float radius, float innerRadius = 0.0f);
	
	//+--------------------------------------+
	//|          Drawing Functions           |
	//+--------------------------------------+
	void DrawTexturedRec(rec rectangle, Color_t color);
	void DrawTexturedRec(rec rectangle, Color_t color, rec sourceRectangle);
	void DrawRectangle(rec rectangle, Color_t color);
	void DrawButton(rec rectangle, Color_t backgroundColor, Color_t borderColor, r32 borderWidth = 1.0f);
	void DrawGradient(rec rectangle, Color_t color1, Color_t color2, Dir2_t direction);
	void DrawLine(v2 p1, v2 p2, r32 thickness, Color_t color);
	void DrawCircle(v2 center, r32 radius, Color_t color);
	void DrawDonut(v2 center, r32 radius, r32 innerRadius, Color_t color);
	void DrawCharacter(u32 charIndex, v2 bottomLeft, Color_t color, r32 scale = 1.0f);
	void DrawString(const char* string, u32 numCharacters, v2 position, Color_t color, r32 scale = 1.0f, Alignment_t alignment = Alignment_Left);
	void DrawString(const char* nullTermString, v2 position, Color_t color, r32 scale = 1.0f, Alignment_t alignment = Alignment_Left);
	void PrintString(v2 position, Color_t color, r32 scale, const char* formatString, ...);
	void DrawFormattedString(const char* string, u32 numCharacters, v2 position, r32 maxWidth, Color_t color, Alignment_t alignment = Alignment_Left, bool preserveWords = true);
	void DrawFormattedString(const char* nullTermString, v2 position, r32 maxWidth, Color_t color, Alignment_t alignment = Alignment_Left, bool preserveWords = true);
};

#endif // _APP_RENDER_STATE_H