/*
File:   appStructs.h
Author: Taylor Robbins
Date:   06\08\2017
*/

#ifndef _APP_STRUCTS_H
#define _APP_STRUCTS_H

struct Texture_t
{
	GLuint id;
	
	union
	{
		v2i size;
		struct { i32 width, height; };
	};
};

struct FontCharInfo_t
{
	union
	{
		v2i position;
		struct { i32 x, y; };
	};
	union
	{
		v2i size;
		struct { i32 width, height; };
	};
	v2 offset;
	r32 advanceX;
};

struct Font_t
{
	Texture_t bitmap;
	r32 fontSize;
	r32 maxCharWidth;
	r32 maxCharHeight;
	r32 maxExtendUp;
	r32 maxExtendDown;
	r32 lineHeight;
	
	u8 firstChar;
	u8 numChars;
	FontCharInfo_t chars[256];
};

struct Shader_t
{
	GLuint vertId;
	GLuint fragId;
	GLuint programId;
	
	GLuint vertexArray;
	
	struct
	{
		GLint positionAttrib;
		GLint colorAttrib;
		GLint texCoordAttrib;
		
		GLint worldMatrix;
		GLint viewMatrix;
		GLint projectionMatrix;
		GLint diffuseTexture;
		GLint alphaTexture;
		GLint diffuseColor;
		GLint secondaryColor;
		GLint doGrayscaleGradient;
		GLint useAlphaTexture;
		GLint sourceRectangle;
		GLint textureSize;
		GLint circleRadius;
		GLint circleInnerRadius;
	} locations;
};

struct VertexBuffer_t
{
	GLuint id;
	
	u32 numVertices;
};

struct Vertex_t
{
	union
	{
		v3 position;
		struct { r32 x, y, z; };
	};
	union
	{
		v4 color;
		struct { r32 r, g, b, a; };
	};
	union
	{
		v2 texCoord;
		struct { r32 tX, tY; };
	};
};

struct FrameBuffer_t
{
	GLuint id;
	GLuint depthBuffer;
	const Texture_t* renderTexture;
};

typedef enum
{
	Alignment_Left,
	Alignment_Center,
	Alignment_Right,
} Alignment_t;

#endif // _APP_STRUCTS_H