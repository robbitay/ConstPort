/*
File:   app.cpp
Author: Taylor Robbins
Date:   06\06\2017
Description: 
	** Contains all the exported functions and #includes 
	** the rest of the source code files.
*/

#include "platformInterface.h"
#include "app_version.h"
#include "Colors.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "memoryArena.h"
#include "linkedList.h"
#include "ft2build.h"
#include FT_FREETYPE_H

const PlatformInfo_t* Gl_PlatformInfo = nullptr;
const AppMemory_t*    Gl_AppMemory = nullptr;

#include "appHelpers.cpp"

//+================================================================+
//|                          Includes                              |
//+================================================================+
#include "appDefines.h"
#include "appStructs.h"
#include "lineList.h"
#include "appData.h"


//+================================================================+
//|                       Source Files                             |
//+================================================================+
#include "lineList.cpp"

Texture_t CreateTexture(const u8* bitmapData, i32 width, i32 height, bool pixelated = false, bool repeat = true)
{
	Texture_t result = {};
	
	result.width = width;
	result.height = height;
	
	glGenTextures(1, &result.id);
	glBindTexture(GL_TEXTURE_2D, result.id);
	
	glTexImage2D(
		GL_TEXTURE_2D, 		//bound texture type
		0,					//image level
		GL_RGBA,			//internal format
		width,		        //image width
		width,		        //image height
		0,					//border
		GL_RGBA,			//format
		GL_UNSIGNED_BYTE,	//type
		bitmapData);		//data
	
	
	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pixelated ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pixelated ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	return result;
}

VertexBuffer_t CreateVertexBuffer(const Vertex_t* vertices, u32 numVertices)
{
	VertexBuffer_t result = {};
	result.numVertices = numVertices;
	
	glGenBuffers(1, &result.id);
	glBindBuffer(GL_ARRAY_BUFFER, result.id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_t) * numVertices, vertices, GL_STATIC_DRAW);
	
	return result;
}

Shader_t LoadShader(const PlatformInfo_t* PlatformInfo,
	const char* vertShaderFileName, const char* fragShaderFileName)
{
	Shader_t result = {};
	GLint compiled;
	int logLength;
	char* logBuffer;
	
	FileInfo_t vertexShaderFile = PlatformInfo->ReadEntireFilePntr(vertShaderFileName);
	FileInfo_t fragmentShaderFile = PlatformInfo->ReadEntireFilePntr(fragShaderFileName);
	
	result.vertId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(result.vertId, 1, (const char* const*)&vertexShaderFile.content, NULL);
	glCompileShader(result.vertId);
	
	glGetShaderiv(result.vertId, GL_COMPILE_STATUS, &compiled);
	glGetShaderiv(result.vertId, GL_INFO_LOG_LENGTH, &logLength);
	DEBUG_PrintLine("%s: Compiled %s : %d byte log",
		vertShaderFileName,
		compiled ? "Successfully" : "Unsuccessfully", logLength);
	if (logLength > 0)
	{
		logBuffer = (char*)malloc(logLength);
		glGetShaderInfoLog(result.vertId, logLength, NULL, logBuffer);
		DEBUG_PrintLine("Log: \"%s\"", logBuffer);
		free(logBuffer);
		Assert(false);
	}
	
	result.fragId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(result.fragId, 1, (const char* const*)&fragmentShaderFile.content, NULL);
	glCompileShader(result.fragId);
	
	glGetShaderiv(result.fragId, GL_COMPILE_STATUS, &compiled);
	glGetShaderiv(result.fragId, GL_INFO_LOG_LENGTH, &logLength);
	DEBUG_PrintLine("%s: Compiled %s : %d byte log",
		fragShaderFileName,
		compiled ? "Successfully" : "Unsuccessfully", logLength);
	if (logLength > 0)
	{
		logBuffer = (char*)malloc(logLength);
		glGetShaderInfoLog(result.fragId, logLength, NULL, logBuffer);
		DEBUG_PrintLine("Log: \"%s\"", logBuffer);
		free(logBuffer);
		Assert(false);
	}
	
	PlatformInfo->FreeFileMemoryPntr(&vertexShaderFile);
	PlatformInfo->FreeFileMemoryPntr(&fragmentShaderFile);
	
	result.programId = glCreateProgram();
	glAttachShader(result.programId, result.fragId);
	glAttachShader(result.programId, result.vertId);
	glLinkProgram(result.programId);
	
	glGetProgramiv(result.programId, GL_LINK_STATUS, &compiled);
	glGetProgramiv(result.programId, GL_INFO_LOG_LENGTH, &logLength);
	DEBUG_PrintLine("Shader: Linked %s : %d byte log",
		compiled ? "Successfully" : "Unsuccessfully", logLength);
	if (logLength > 0)
	{
		logBuffer = (char*)malloc(logLength);
		glGetProgramInfoLog(result.programId, logLength, NULL, logBuffer);
		DEBUG_PrintLine("Log: \"%s\"", logBuffer);
		free(logBuffer);
		Assert(false);
	}
	
	result.worldMatrixLocation         = glGetUniformLocation(result.programId, "WorldMatrix");
	result.viewMatrixLocation          = glGetUniformLocation(result.programId, "ViewMatrix");
	result.projectionMatrixLocation    = glGetUniformLocation(result.programId, "ProjectionMatrix");
	result.diffuseTextureLocation      = glGetUniformLocation(result.programId, "DiffuseTexture");
	result.alphaTextureLocation        = glGetUniformLocation(result.programId, "AlphaTexture");
	result.diffuseColorLocation        = glGetUniformLocation(result.programId, "DiffuseColor");
	result.secondaryColorLocation      = glGetUniformLocation(result.programId, "SecondaryColor");
	result.doGrayscaleGradientLocation = glGetUniformLocation(result.programId, "DoGrayscaleGradient");
	result.sourceRectangleLocation     = glGetUniformLocation(result.programId, "SourceRectangle");
	result.useAlphaTextureLocation     = glGetUniformLocation(result.programId, "UseAlphaTexture");
	
	glGenVertexArrays(1, &result.vertexArray);
	glBindVertexArray(result.vertexArray);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	
	return result;
}

Texture_t LoadTexture(const PlatformInfo_t* PlatformInfo,
	const char* fileName, bool pixelated = false, bool repeat = true)
{
	Texture_t result = {};
	
	FileInfo_t textureFile = PlatformInfo->ReadEntireFilePntr(fileName);
	
	i32 numChannels;
	i32 width, height;
	u8* imageData = stbi_load_from_memory(
		(u8*)textureFile.content, textureFile.size,
		&width, &height, &numChannels, 4);
	
	result = CreateTexture(imageData, width, height, pixelated, repeat);
	
	stbi_image_free(imageData);
	PlatformInfo->FreeFileMemoryPntr(&textureFile);
	
	return result;
}

Font_t LoadFont(const PlatformInfo_t* PlatformInfo,
	const char* fileName, r32 fontSize, i32 bitmapWidth, i32 bitmapHeight,
	u8 firstCharacter, u8 numCharacters)
{
	Font_t result = {};
	
	FileInfo_t fontFile = PlatformInfo->ReadEntireFilePntr(fileName);
	
	result.numChars = numCharacters;
	result.firstChar = firstCharacter;
	result.fontSize = fontSize;
	
	u8* grayscaleData = (u8*)malloc(sizeof(u8) * bitmapWidth * bitmapHeight);
	stbtt_bakedchar* charInfos = (stbtt_bakedchar*)malloc(sizeof(stbtt_bakedchar) * numCharacters);
	
	int bakeResult = stbtt_BakeFontBitmap((u8*)fontFile.content, 
		0, fontSize,
		grayscaleData, bitmapWidth, bitmapHeight, 
		firstCharacter, numCharacters, charInfos);
	DEBUG_PrintLine("STB Bake Result: %d", bakeResult);
	
	for (u8 cIndex = 0; cIndex < numCharacters; cIndex++)
	{
		result.chars[cIndex].position = NewVec2i(
			charInfos[cIndex].x0, 
			charInfos[cIndex].y0);
		result.chars[cIndex].size = NewVec2i(
			charInfos[cIndex].x1 - charInfos[cIndex].x0, 
			charInfos[cIndex].y1 - charInfos[cIndex].y0);
		result.chars[cIndex].offset = NewVec2(
			charInfos[cIndex].xoff, 
			charInfos[cIndex].yoff);
		result.chars[cIndex].advanceX = charInfos[cIndex].xadvance;
	}
	
	u8* bitmapData = (u8*)malloc(sizeof(u8)*4 * bitmapWidth * bitmapHeight);
	
	for (i32 y = 0; y < bitmapHeight; y++)
	{
		for (i32 x = 0; x < bitmapWidth; x++)
		{
			u8 grayscaleValue = grayscaleData[y*bitmapWidth + x];
			
			bitmapData[(y*bitmapWidth+x)*4 + 0] = 255;
			bitmapData[(y*bitmapWidth+x)*4 + 1] = 255;
			bitmapData[(y*bitmapWidth+x)*4 + 2] = 255;
			bitmapData[(y*bitmapWidth+x)*4 + 3] = grayscaleValue; 
		}
	}
	
	result.bitmap = CreateTexture(bitmapData, bitmapWidth, bitmapHeight);
	
	free(grayscaleData);
	free(charInfos);
	free(bitmapData);
	PlatformInfo->FreeFileMemoryPntr(&fontFile);
	
	//Create information about character sizes
	{
		v2 maxSize = Vec2_Zero;
		v2 extendVertical = Vec2_Zero;
		for (u32 cIndex = 0; cIndex < result.numChars; cIndex++)
		{
			FontCharInfo_t* charInfo = &result.chars[cIndex];
			
			if (charInfo->height > maxSize.y)
				maxSize.y = (r32)charInfo->height;
			
			if (-charInfo->offset.y > extendVertical.x)
				extendVertical.x = -charInfo->offset.y;
			
			if (charInfo->offset.y + charInfo->height > extendVertical.y)
				extendVertical.y = charInfo->offset.y + charInfo->height;
			
			if (charInfo->advanceX > maxSize.x)
				maxSize.x = charInfo->advanceX;
		}
		
		result.maxCharWidth = maxSize.x;
		result.maxCharHeight = maxSize.y;
		result.maxExtendUp = extendVertical.x;
		result.maxExtendDown = extendVertical.y;
		result.lineHeight = result.maxExtendDown + result.maxExtendUp;
	}
	
	return result;
}

Texture_t FreeTypeTestGlyph(const char* fontFilePath, i32 glyphIndex)
{
	Texture_t result = {};
	
	FT_Library       fontLibrary;
	FT_Face          fontFace;
	FT_Bitmap        bitmap;
	FT_GlyphSlot     currentGlyph;
	FT_Glyph_Metrics glyphMetrics;
	
	if (FT_Init_FreeType(&fontLibrary) != 0) 
	{
		Assert(false);
		return result;
	}
	
	if (FT_New_Face(fontLibrary, fontFilePath, 0, &fontFace) != 0)
	{
		Assert(false);
		return result;
	}
	
	if (FT_Set_Char_Size(fontFace, 0, 832, 300, 300) != 0)
	{
		Assert(false);
		return result;
	}
	
	FT_Set_Transform(fontFace, NULL, NULL);
	
	if (FT_Load_Glyph(fontFace, glyphIndex, FT_LOAD_DEFAULT) != 0)
	{
		Assert(false);
		return result;	
	}
	
	currentGlyph = fontFace->glyph;
	
	if (currentGlyph->format != FT_GLYPH_FORMAT_BITMAP)
	{
		if (FT_Render_Glyph(fontFace->glyph, FT_RENDER_MODE_LCD) != 0)
		{
			Assert(false);
			return result;
		}
	}
	
	bitmap = currentGlyph->bitmap;
	glyphMetrics = currentGlyph->metrics;
	
	DEBUG_PrintLine("Glyph %d is %ux%u", glyphIndex, bitmap.width, bitmap.rows);
	
	i32 bitmapWidth = bitmap.width / 3;
	i32 bitmapHeight = bitmap.rows;
	i32 textureWidth = max(bitmapWidth, bitmapHeight);
	i32 textureHeight = max(bitmapWidth, bitmapHeight);
	u8* bitmapData = (u8*)malloc(sizeof(u8)*4 * textureWidth * textureHeight);
	
	for (i32 y = 0; y < textureHeight; y++)
	{
		for (i32 x = 0; x < textureWidth; x++)
		{
			if (x < bitmapWidth && y < bitmapHeight)
			{
				u8 red =   bitmap.buffer[y*bitmap.pitch + x*3 + 0];
				u8 green = bitmap.buffer[y*bitmap.pitch + x*3 + 1];
				u8 blue =  bitmap.buffer[y*bitmap.pitch + x*3 + 2];
				// if (red != green || green != blue)
				// {
				// 	Assert(false);
				// }
				
				bitmapData[(y*textureWidth + x)*4 + 0] = red;
				bitmapData[(y*textureWidth + x)*4 + 1] = green;
				bitmapData[(y*textureWidth + x)*4 + 2] = blue;
				bitmapData[(y*textureWidth + x)*4 + 3] = 0xFF;
			}
			else
			{
				bitmapData[(y*textureWidth + x)*4 + 0] = 0xFF;
				bitmapData[(y*textureWidth + x)*4 + 1] = 0xFF;
				bitmapData[(y*textureWidth + x)*4 + 2] = 0xFF;
				bitmapData[(y*textureWidth + x)*4 + 3] = 0x00;
			}
		}
	}
	
	result = CreateTexture(bitmapData, textureWidth, textureHeight, true, false);
	
	free(bitmapData);
	
	return result;
}

inline void SetWorldMatrix(const Shader_t& shader, const Matrix4_t& worldMatrix)
{
	glUniformMatrix4fv(shader.worldMatrixLocation, 1, GL_FALSE, &worldMatrix.values[0][0]);
}
inline void SetViewMatrix(const Shader_t& shader, const Matrix4_t& viewMatrix)
{
	glUniformMatrix4fv(shader.viewMatrixLocation, 1, GL_FALSE, &viewMatrix.values[0][0]);
}
inline void SetProjectionMatrix(const Shader_t& shader, const Matrix4_t& projectionMatrix)
{
	glUniformMatrix4fv(shader.projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix.values[0][0]);
}
inline void SetTexture(const Shader_t& shader, const Texture_t& texture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glUniform1i(shader.diffuseTextureLocation, 0);
}
inline void SetColor(const Shader_t& shader, Color_t color)
{
	v4 colorVec = ColorToVec4(color);
	glUniform4f(shader.diffuseColorLocation, colorVec.r, colorVec.g, colorVec.b, colorVec.a);
}
inline void SetSecondaryColor(const Shader_t& shader, Color_t color)
{
	v4 colorVec = ColorToVec4(color);
	glUniform4f(shader.secondaryColorLocation, colorVec.r, colorVec.g, colorVec.b, colorVec.a);
}
inline void SetGrayscaleDoGradient(const Shader_t& shader, bool doGradient)
{
	glUniform1i(shader.doGrayscaleGradientLocation, doGradient ? 1 : 0);
}
inline void SetSourceRectangle(const Shader_t& shader, rec sourceRectangle)
{
	glUniform4f(shader.sourceRectangleLocation, sourceRectangle.x, sourceRectangle.y, sourceRectangle.width, sourceRectangle.height);
}
inline void EnableAlphaTexture(const Shader_t& shader, const Texture_t& texture)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glUniform1i(shader.alphaTextureLocation, 1);
	glUniform1i(shader.useAlphaTextureLocation, 1);
}
inline void DisableAlphaTexture(const Shader_t& shader)
{
	glUniform1i(shader.useAlphaTextureLocation, 0);
}
inline void BindBuffer(const Shader_t& shader, const VertexBuffer_t& vertBuffer)
{
	glBindVertexArray(shader.vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer.id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)sizeof(v3));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)(sizeof(v3)+sizeof(v4)));
}

inline void DrawTexture(AppData_t* appData, const Texture_t& texture, rec rectangle, Color_t color)
{
	SetSourceRectangle(appData->simpleShader, NewRectangle(0, 0, 1, 1));
	SetTexture(appData->simpleShader, texture);
	SetColor(appData->simpleShader, color);
	m4 worldMatrix = Mat4Mult(
		Matrix4Translate(NewVec3(rectangle.x, rectangle.y, 0.0f)),       //Position
		Matrix4Scale(NewVec3(rectangle.width, rectangle.height, 1.0f))); //Scale
	SetWorldMatrix(appData->simpleShader, worldMatrix);
	BindBuffer(appData->simpleShader, appData->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, appData->squareBuffer.numVertices);
}

inline void DrawRectangle(AppData_t* appData, rec rectangle, Color_t color)
{
	SetSourceRectangle(appData->simpleShader, NewRectangle(0, 0, 1, 1));
	SetTexture(appData->simpleShader, appData->dotTexture);
	SetColor(appData->simpleShader, color);
	m4 worldMatrix = Mat4Mult(
		Matrix4Translate(NewVec3(rectangle.x, rectangle.y, 0.0f)),       //Position
		Matrix4Scale(NewVec3(rectangle.width, rectangle.height, 1.0f))); //Scale
	SetWorldMatrix(appData->simpleShader, worldMatrix);
	BindBuffer(appData->simpleShader, appData->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, appData->squareBuffer.numVertices);
}

inline void DrawGradient(AppData_t* appData, rec rectangle, Color_t color1, Color_t color2, Direction2D_t direction)
{
	SetSourceRectangle(appData->simpleShader, NewRectangle(0, 0, 1, 1));
	SetTexture(appData->simpleShader, appData->gradientTexture);
	SetColor(appData->simpleShader, color1);
	SetSecondaryColor(appData->simpleShader, color2);
	SetGrayscaleDoGradient(appData->simpleShader, true);
	m4 worldMatrix = Matrix4_Identity;
	switch (direction)
	{
		case Direction2D_Right:
		default:
		{
			worldMatrix = Mat4Mult(
				Matrix4Translate(NewVec3(rectangle.x, rectangle.y, 0.0f)),
				Matrix4Scale(NewVec3(rectangle.width, rectangle.height, 1.0f)));
		} break;
		
		case Direction2D_Left:
		{
			worldMatrix = Mat4Mult(
				Matrix4Translate(NewVec3(rectangle.x + rectangle.width, rectangle.y, 0.0f)),
				Matrix4Scale(NewVec3(-rectangle.width, rectangle.height, 1.0f)));
		} break;
		
		case Direction2D_Down:
		{
			worldMatrix = Matrix4Multiply(
				Matrix4Translate(NewVec3(rectangle.x + rectangle.width, rectangle.y, 0.0f)),
				Matrix4RotateZ(ToRadians(90)),
				Matrix4Scale(NewVec3(rectangle.height, rectangle.width, 1.0f)));
		} break;
		
		case Direction2D_Up:
		{
			worldMatrix = Matrix4Multiply(
				Matrix4Translate(NewVec3(rectangle.x + rectangle.width, rectangle.y + rectangle.height, 0.0f)),
				Matrix4RotateZ(ToRadians(90)),
				Matrix4Scale(NewVec3(-rectangle.height, rectangle.width, 1.0f)));
		} break;
	};
	
	SetWorldMatrix(appData->simpleShader, worldMatrix);
	BindBuffer(appData->simpleShader, appData->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, appData->squareBuffer.numVertices);
	
	SetGrayscaleDoGradient(appData->simpleShader, false);
}

inline void DrawCircle(AppData_t* appData, v2 center, r32 radius, Color_t color)
{
	EnableAlphaTexture(appData->simpleShader, appData->circleTexure);
	DrawRectangle(appData, NewRectangle(center.x - radius, center.y - radius, radius*2, radius*2), color);
	DisableAlphaTexture(appData->simpleShader);
}

inline void DrawTexturePart(AppData_t* appData, Texture_t texture, rec rectangle, Color_t color, rec sourceRectangle)
{
	rec realSourceRec = NewRectangle(
		sourceRectangle.x / (r32)texture.width,
		sourceRectangle.y / (r32)texture.height,
		sourceRectangle.width / (r32)texture.width,
		sourceRectangle.height / (r32)texture.height);
	
	SetSourceRectangle(appData->simpleShader, realSourceRec);
	SetTexture(appData->simpleShader, texture);
	SetColor(appData->simpleShader, color);
	m4 worldMatrix = Mat4Mult(
		Matrix4Translate(NewVec3(rectangle.x, rectangle.y, 0.0f)),       //Position
		Matrix4Scale(NewVec3(rectangle.width, rectangle.height, 1.0f))); //Scale
	SetWorldMatrix(appData->simpleShader, worldMatrix);
	BindBuffer(appData->simpleShader, appData->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, appData->squareBuffer.numVertices);
}

inline u32 GetCharIndex(const Font_t& font, char character)
{
	u32 cIndex = 0;
	if (character >= font.firstChar && character - font.firstChar < font.numChars)
	{
		cIndex = character - font.firstChar;
	}
	else
	{
		cIndex = 127 - font.firstChar;
	}
	
	return cIndex;
}

inline void DrawCharacter(AppData_t* appData, const Font_t& font, u32 charIndex, v2 bottomLeft, Color_t color, r32 scale = 1.0f)
{
	const FontCharInfo_t* charInfo = &font.chars[charIndex];
	
	rec sourceRectangle = NewRectangle((r32)charInfo->x, (r32)charInfo->y, (r32)charInfo->width, (r32)charInfo->height);
	rec drawRectangle = NewRectangle(
		bottomLeft.x + scale*charInfo->offset.x, 
		bottomLeft.y + scale*charInfo->offset.y, 
		scale*charInfo->width, 
		scale*charInfo->height);
	
	DrawTexturePart(appData, font.bitmap, drawRectangle, color, sourceRectangle);
}

inline void DrawString(AppData_t* appData, const Font_t& font, const char* string, u32 numCharacters, v2 position, Color_t color, r32 scale = 1.0f)
{
	v2 currentPos = position;
	for (u32 cIndex = 0; cIndex < numCharacters; cIndex++)
	{
		if (string[cIndex] == '\t')
		{
			u32 spaceIndex = GetCharIndex(font, ' ');
			currentPos.x += font.chars[spaceIndex].advanceX * TAB_WIDTH;
		}
		else if (string[cIndex] == '\r')
		{
			//Don't do anything
		}
		else
		{
			u32 charIndex = GetCharIndex(font, string[cIndex]);
			DrawCharacter(appData, font, charIndex, currentPos, color, scale);
			currentPos.x += font.chars[charIndex].advanceX * scale;
		}
	}
}

inline void DrawString(AppData_t* appData, const Font_t& font, const char* string, v2 position, Color_t color, r32 scale = 1.0f)
{
	DrawString(appData, font, string, (u32)strlen(string), position, color, scale);
}

inline void PrintString(AppData_t* appData, const Font_t& font, v2 position, Color_t color, r32 scale, const char* formatString, ...)
{
	char printBuffer[256] = {};
	va_list args;
	
	va_start(args, formatString);
	size_t length = vsnprintf(printBuffer, 256-1, formatString, args);
	va_end(args);
	
	DrawString(appData, font, printBuffer, position, color, scale);
}

inline v2 MeasureString(const Font_t& font, const char* string)
{
	v2 currentPos = Vec2_Zero;
	for (i32 cIndex = 0; cIndex < string[cIndex] != '\0'; cIndex++)
	{
		if (string[cIndex] == '\t')
		{
			u32 spaceIndex = GetCharIndex(font, ' ');
			currentPos.x += font.chars[spaceIndex].advanceX * TAB_WIDTH;
		}
		else if (string[cIndex] == '\r')
		{
			//Don't do anything
		}
		else
		{
			u32 charIndex = GetCharIndex(font, string[cIndex]);
			const FontCharInfo_t* charInfo = &font.chars[charIndex];
			currentPos.x += charInfo->advanceX;
		}
	}
	
	return NewVec2(currentPos.x, font.lineHeight);
}

inline v2 MeasureLine(const Font_t& font, const Line_t* line)
{
	return MeasureString(font, line->chars);
}

inline void DrawLine(AppData_t* appData, const Line_t* line, v2 startPos)
{
	//TODO: Add cool formatting stuff?
	
	DrawString(appData, appData->testFont, line->chars, startPos, line->color, 1.0f);
}

//+================================================================+
//|                       App Get Version                          |
//+================================================================+
AppGetVersion_DEFINITION(App_GetVersion)
{
	Version_t version = {
		APP_VERSION_MAJOR,
		APP_VERSION_MINOR,
		APP_VERSION_BUILD,
	};
	
	if (resetApplication != nullptr)
	{
		*resetApplication = true;
	}
	
	return version;
}

//+================================================================+
//|                       App Initialize                           |
//+================================================================+
AppInitialize_DEFINITION(App_Initialize)
{
	Gl_PlatformInfo = PlatformInfo;
	Gl_AppMemory = AppMemory;
	DEBUG_WriteLine("Initializing Game...");
	
	AppData_t* appData = (AppData_t*)AppMemory->permanantPntr;
	ClearPointer(appData);
	
	void* arenaBase = (void*)(appData+1);
	u32 arenaSize = AppMemory->permanantSize - sizeof(AppData_t);
	InitializeMemoryArenaHeap(&appData->memArena, arenaBase, arenaSize);
	
	Vertex_t suareVertices[] =
	{
		{  {0.0f, 0.0f, 0.0f}, ColorToVec4({Color_White}), {0.0f, 0.0f} },
		{  {1.0f, 0.0f, 0.0f}, ColorToVec4({Color_White}), {1.0f, 0.0f} },
		{  {0.0f, 1.0f, 0.0f}, ColorToVec4({Color_White}), {0.0f, 1.0f} },
		
		{  {0.0f, 1.0f, 0.0f}, ColorToVec4({Color_White}), {0.0f, 1.0f} },
		{  {1.0f, 0.0f, 0.0f}, ColorToVec4({Color_White}), {1.0f, 0.0f} },
		{  {1.0f, 1.0f, 0.0f}, ColorToVec4({Color_White}), {1.0f, 1.0f} },
	};
	appData->squareBuffer = CreateVertexBuffer(suareVertices, ArrayCount(suareVertices));
	
	Vertex_t triangleVertices[] =
	{
		{  {0.0f, 1.0f, 0.0f}, ColorToVec4({Color_White}), {0.0f, 1.0f} },
		{  {0.5f, 0.0f, 0.0f}, ColorToVec4({Color_White}), {0.5f, 0.0f} },
		{  {1.0f, 1.0f, 0.0f}, ColorToVec4({Color_White}), {1.0f, 1.0f} },
	};
	appData->triangleBuffer = CreateVertexBuffer(triangleVertices, ArrayCount(triangleVertices));
	
	appData->simpleShader = LoadShader(PlatformInfo,
		"Resources/Shaders/simple-vertex.glsl",
		"Resources/Shaders/simple-fragment.glsl");
	
	appData->testTexture = LoadTexture(PlatformInfo,
		"Resources/Sprites/test.png");
	appData->gradientTexture = LoadTexture(PlatformInfo,
		"Resources/Textures/gradient.png");
	appData->scrollBarEndcapTexture = LoadTexture(PlatformInfo,
		"Resources/Sprites/scrollBarEndcap.png", false, false);
	appData->circleTexure = LoadTexture(PlatformInfo,
		"Resources/Sprites/circle.png", false, false);
	
	appData->glyphTexture = FreeTypeTestGlyph("Resources/Fonts/consola.ttf", 5);
	
	appData->testFont = LoadFont(PlatformInfo,
		"Resources/Fonts/consola.ttf", 16,
		1024, 1024, ' ', 96);
	
	FileInfo_t testFile = PlatformInfo->ReadEntireFilePntr("test.txt");
	CreateLineList(&appData->lineList, &appData->memArena, "");//(const char*)testFile.content);
	PlatformInfo->FreeFileMemoryPntr(&testFile);
	
	Color_t dotTexture = {Color_White};
	appData->dotTexture = CreateTexture((u8*)&dotTexture, 1, 1);
	
	DEBUG_WriteLine("Initialization Done!");
}

//+================================================================+
//|                         App Update                             |
//+================================================================+
AppUpdate_DEFINITION(App_Update)
{
	Gl_PlatformInfo = PlatformInfo;
	Gl_AppMemory = AppMemory;
	AppData_t* appData = (AppData_t*)AppMemory->permanantPntr;
	
	Color_t color1 = ColorFromHSV((i32)(PlatformInfo->programTime*180) % 360, 1.0f, 1.0f);
	Color_t color2 = ColorFromHSV((i32)(PlatformInfo->programTime*180 + 125) % 360, 1.0f, 1.0f);
	v2 mousePos = AppInput->mousePos;
	v2 mouseStartPos = AppInput->mouseStartPos[MouseButton_Left];
	v2 screenSize = NewVec2((r32)PlatformInfo->screenSize.x, (r32)PlatformInfo->screenSize.y);
	r32 fileHeight = (appData->lineList.numLines * (appData->testFont.lineHeight + 2));
	rec toolbarRec = NewRectangle(
		0, 
		screenSize.y-appData->testFont.lineHeight, 
		screenSize.x - (SCROLLBAR_WIDTH+SCROLLBAR_PADDING*2), 
		appData->testFont.lineHeight);
	rec scrollBarGutterRec = NewRectangle(
		screenSize.x - SCROLLBAR_WIDTH - SCROLLBAR_PADDING*2, 0, 
		SCROLLBAR_WIDTH + SCROLLBAR_PADDING*2, screenSize.y);
	char gutterNumberBuffer[10] = {};
	r32 gutterWidth = 0;
	for (i32 lineIndex = 0; lineIndex < appData->lineList.numLines; lineIndex++)
	{
		ClearArray(gutterNumberBuffer);
		snprintf(gutterNumberBuffer, ArrayCount(gutterNumberBuffer)-1, "%u", lineIndex+1);
		v2 stringSize = MeasureString(appData->testFont, gutterNumberBuffer);
		if (stringSize.x > gutterWidth)
		{
			gutterWidth = stringSize.x;
		}
	}
	gutterWidth += 2;
	rec gutterRec = NewRectangle(0, 0, gutterWidth, screenSize.y - toolbarRec.height);
	rec viewRec = NewRectangle(
		gutterWidth, 0,
		screenSize.x - gutterWidth - scrollBarGutterRec.width, 
		screenSize.y - toolbarRec.height);
	rec scrollBarRec = NewRectangle(
		scrollBarGutterRec.x + SCROLLBAR_PADDING,
		0,
		SCROLLBAR_WIDTH,
		scrollBarGutterRec.height * (viewRec.height / fileHeight));
	r32 maxScrollOffset = fileHeight - viewRec.height;
	r32 scrollPercent = appData->scrollOffset / maxScrollOffset;
	scrollBarRec.y = scrollBarGutterRec.y + (scrollBarGutterRec.height - scrollBarRec.height) * scrollPercent;
	
	if (AppInput->scrollDelta.y != 0)
	{
		appData->scrollOffset += -AppInput->scrollDelta.y * SCROLL_MULTIPLIER;
	}
	
	if (AppInput->buttons[Button_Enter].transCount > 0 && AppInput->buttons[Button_Enter].isDown)
	{
		FileInfo_t testFile = PlatformInfo->ReadEntireFilePntr("test.txt");
		DestroyLineList(&appData->lineList);
		CreateLineList(&appData->lineList, &appData->memArena, (const char*)testFile.content);
		PlatformInfo->FreeFileMemoryPntr(&testFile);
	}
	
	if (AppInput->buttons[Button_Backspace].transCount > 0 && AppInput->buttons[Button_Backspace].isDown)
	{
		if (appData->comPort.isOpen)
		{
			PlatformInfo->CloseComPortPntr(&appData->comPort);
			DEBUG_WriteLine("Closed COM port");
		}
		
		appData->comPort = PlatformInfo->OpenComPortPntr("COM6",
			BaudRate_115200, false, true, Parity_None, 8, StopBits_1);
		
		if (appData->comPort.isOpen)
		{
			DEBUG_WriteLine("COM port opened successfully");
		}
		else
		{
			DEBUG_WriteLine("Couldn't open COM port.");
		}
	}
	
	if (appData->comPort.isOpen)
	{
		bool viewAtEnd = (appData->scrollOffset + viewRec.height) >= (fileHeight - appData->testFont.lineHeight);
		
		i32 readResult = 1;
		while (readResult > 0)
		{
			char buffer[4096] = {};
			readResult = PlatformInfo->ReadComPortPntr(&appData->comPort, buffer, ArrayCount(buffer)-1);
			if (readResult > 0)
			{
				// DEBUG_PrintLine("Read %d bytes \"%.*s\"", readResult, readResult, buffer);
				
				Line_t* lastLine = GetLastLine(&appData->lineList);
				
				for (i32 cIndex = 0; cIndex < readResult; cIndex++)
				{
					char newChar = buffer[cIndex];
					if (newChar == '\n')
					{
						Line_t* finishedLine = lastLine;
						if (finishedLine->numChars >= 4 && strncmp(finishedLine->chars, "lib:", 4) == 0)
						{
							finishedLine->color = Color_Highlight;
						}
						
						lastLine = AddLineToList(&appData->lineList, "");
						lastLine->color = Color_Foreground;
						if (viewAtEnd)
						{
							appData->scrollOffset += 19;//appData->testFont.lineHeight;
						}
					}
					else
					{
						LineAppend(&appData->lineList, lastLine, newChar);
					}
				}
				
				// LineAppend(&appData->lineList, lastLine, ']');
				// LineAppend(&appData->lineList, lastLine, ']');
			}
			else if (readResult < 0)
			{
				DEBUG_PrintLine("COM port read Error!: %d", readResult);
			}
		}
	}
	
	if (AppInput->buttons[Button_Down].isDown)
	{
		appData->scrollOffset += AppInput->buttons[Button_Shift].isDown ? 16 : 5;
	}
	if (AppInput->buttons[Button_Up].isDown)
	{
		appData->scrollOffset -= AppInput->buttons[Button_Shift].isDown ? 16 : 5;
		if (appData->scrollOffset < 0)
			appData->scrollOffset = 0;
	}
	
	//Handle scrollbar interaction with mouse
	if (AppInput->buttons[MouseButton_Left].isDown)
	{
		if (IsInsideRectangle(mouseStartPos, scrollBarGutterRec))
		{
			if (AppInput->buttons[MouseButton_Left].transCount > 0)//Pressed the button down
			{
				appData->mouseScrollbarOffset = mousePos.y - scrollBarRec.y;
				if (IsInsideRectangle(mousePos, scrollBarRec))
				{
					appData->startedOnScrollbar = true;
				}
				else 
				{
					appData->startedOnScrollbar = false;
					if (appData->mouseScrollbarOffset > 0)
					{
						appData->scrollOffset += viewRec.height;
					}
					else
					{
						appData->scrollOffset -= viewRec.height;
					}
				}
			}
			else if (appData->startedOnScrollbar) //holding the button
			{
				r32 newPixelLocation = mousePos.y - appData->mouseScrollbarOffset;
				if (newPixelLocation > scrollBarGutterRec.y + (scrollBarGutterRec.height - scrollBarRec.height))
				{
					newPixelLocation = scrollBarGutterRec.y + (scrollBarGutterRec.height - scrollBarRec.height);
				}
				if (newPixelLocation < 0)
				{
					newPixelLocation = 0;
				}
				
				appData->scrollOffset = (newPixelLocation / (scrollBarGutterRec.height - scrollBarRec.height)) * maxScrollOffset;
			}
		}
	}
	
	fileHeight = (appData->lineList.numLines * (appData->testFont.lineHeight + 2));
	maxScrollOffset = fileHeight - viewRec.height;
	if (appData->scrollOffset < 0)
			appData->scrollOffset = 0;
	if (appData->scrollOffset > maxScrollOffset)
		appData->scrollOffset = maxScrollOffset;
	scrollPercent = appData->scrollOffset / maxScrollOffset;
	scrollBarRec.y = scrollBarGutterRec.y + (scrollBarGutterRec.height - scrollBarRec.height) * scrollPercent;
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	glClearColor((Color_Background.r/255.f), (Color_Background.g/255.f), (Color_Background.b/255.f), 1.0f);
	// glClearColor((200/255.f), (200/255.f), (200/255.f), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	Matrix4_t worldMatrix, viewMatrix, projMatrix;
	
	viewMatrix = Matrix4_Identity;
	projMatrix = Matrix4Scale(NewVec3(2.0f/PlatformInfo->screenSize.x, -2.0f/PlatformInfo->screenSize.y, 1.0f));
	projMatrix = Mat4Mult(projMatrix, Matrix4Translate(NewVec3(-PlatformInfo->screenSize.x/2.0f, -PlatformInfo->screenSize.y/2.0f, 0.0f)));
	SetViewMatrix(appData->simpleShader, viewMatrix);
	SetProjectionMatrix(appData->simpleShader, projMatrix);
	SetGrayscaleDoGradient(appData->simpleShader, false);
	
	glUseProgram(appData->simpleShader.programId);
	
	DrawGradient(appData, gutterRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	// DrawGradient(appData, NewRectangle(0, 0, 300, 300), color1, color2, Direction2D_Right);
	
	{//Items drawn relative to view
		viewMatrix = Matrix4Translate(NewVec3(0, -appData->scrollOffset, 0));
		SetViewMatrix(appData->simpleShader, viewMatrix);
		
		v2 currentPos = NewVec2(gutterWidth + 2, appData->testFont.maxExtendUp);
		for (i32 lineIndex = 0; lineIndex < appData->lineList.numLines; lineIndex++)
		{
			Line_t* linePntr = GetLineAt(&appData->lineList, lineIndex);
			v2 lineSize = MeasureLine(appData->testFont, linePntr);
			rec backRec = NewRectangle(currentPos.x, currentPos.y - appData->testFont.maxExtendUp, lineSize.x, appData->testFont.lineHeight);
			backRec = RectangleInflate(backRec, 1);
			// DrawGradient(appData, backRec, {0x80494949}, {0x80404040}, Direction2D_Down);
			
			PrintString(appData, appData->testFont, NewVec2(0, currentPos.y), {Color_White}, 1.0f, "%u", lineIndex+1);
			
			DrawLine(appData, linePntr, currentPos);
			
			currentPos.y += appData->testFont.lineHeight + 2;
		}
		viewMatrix = Matrix4_Identity;
		SetViewMatrix(appData->simpleShader, viewMatrix);
	}
	
	DrawGradient(appData, toolbarRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	// DrawGradient(appData, NewRectangle(10, 10, 300, 300), color1, color2, Direction2D_Right);
	PrintString(appData, appData->testFont, 
		NewVec2(0, screenSize.y-appData->testFont.maxExtendDown), {Color_White}, 1.0f, 
		"Heap: %u/%u used", appData->memArena.used, appData->memArena.size);
	// PrintString(appData, appData->testFont, 
	// 	NewVec2(0, screenSize.y-appData->testFont.maxExtendDown), {Color_White}, 1.0f, 
	// 	"%u Lines Offset: %f (%fpx long)", appData->lineList.numLines, appData->scrollOffset, fileHeight);
	
	//Draw Scrollbar
	DrawGradient(appData, NewRectangle(scrollBarGutterRec.x - 8, 0, 8, screenSize.y), 
		{Color_TransparentBlack}, {Color_HalfTransparentBlack}, Direction2D_Right);
	DrawGradient(appData, scrollBarGutterRec, Color_Background, Color_UiGray3, Direction2D_Right);
	
	rec centerScrollBarRec = scrollBarRec;
	centerScrollBarRec.y += scrollBarRec.width;
	centerScrollBarRec.height -= 2 * scrollBarRec.width;
	rec startCapRec = NewRectangle(scrollBarRec.x, scrollBarRec.y, scrollBarRec.width, scrollBarRec.width);
	rec endCapRec = NewRectangle(scrollBarRec.x, scrollBarRec.y + scrollBarRec.height - scrollBarRec.width, scrollBarRec.width, scrollBarRec.width);
	endCapRec.y += endCapRec.height;
	endCapRec.height = -endCapRec.height;
	DrawRectangle(appData, RectangleInflate(centerScrollBarRec, 1), Color_UiGray4);
	EnableAlphaTexture(appData->simpleShader, appData->scrollBarEndcapTexture);
	DrawRectangle(appData, RectangleInflate(startCapRec, 1), Color_UiGray4);
	DrawRectangle(appData, RectangleInflate(endCapRec, 1), Color_UiGray4);
	
	DrawGradient(appData, startCapRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	DrawGradient(appData, endCapRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	DisableAlphaTexture(appData->simpleShader);
	DrawGradient(appData, centerScrollBarRec, Color_UiGray1, Color_UiGray3, Direction2D_Right);
	
	// DrawTexture(appData, appData->glyphTexture, NewRectangle(10, 10, 500, 500), {Color_White});
	
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Left], AppInput->mouseMaxDist[MouseButton_Left], {Color_Red});
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Right], AppInput->mouseMaxDist[MouseButton_Right], {Color_Blue});
	// DrawCircle(appData, AppInput->mouseStartPos[MouseButton_Middle], AppInput->mouseMaxDist[MouseButton_Middle], {Color_Green});
	
	// DrawRectangle(appData, scrollBarGutterRec, {Color_Red});
	// DrawRectangle(appData, scrollBarRec, {Color_Blue});
	// DrawRectangle(appData, NewRectangle(0, 0, gutterWidth, screenSize.y), {Color_Orange});
	// DrawRectangle(appData, toolbarRec, {Color_Yellow});
	// DrawRectangle(appData, toolbarRec, {Color_Yellow});
}

//+================================================================+
//|                   App Get Sound Samples                        |
//+================================================================+
AppGetSoundSamples_DEFINITION(App_GetSoundSamples)
{
	Gl_PlatformInfo = PlatformInfo;
	Gl_AppMemory = AppMemory;
	AppData_t* appData = (AppData_t*)AppMemory->permanantPntr;
	
}

//+================================================================+
//|                        App Closing                             |
//+================================================================+
AppClosing_DEFINITION(App_Closing)
{
	Gl_PlatformInfo = PlatformInfo;
	Gl_AppMemory = AppMemory;
	AppData_t* appData = (AppData_t*)AppMemory->permanantPntr;
	
	DEBUG_WriteLine("Application closing!");
	
	
}