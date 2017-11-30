/*
File:   appRenderState.cpp
Author: Taylor Robbins
Date:   06\13\2017
Description: 
	** Lots of functions that facilitate drawing different
	** primitives and common things

#included from app.cpp 
*/

void InitializeRenderState()
{
	Assert(renderState != nullptr);
	ClearPointer(renderState);
	
	Vertex_t squareVertices[] =
	{
		{  {0.0f, 0.0f, 0.0f}, NewVec4(NewColor(Color_White)), {0.0f, 0.0f} },
		{  {1.0f, 0.0f, 0.0f}, NewVec4(NewColor(Color_White)), {1.0f, 0.0f} },
		{  {0.0f, 1.0f, 0.0f}, NewVec4(NewColor(Color_White)), {0.0f, 1.0f} },
		
		{  {0.0f, 1.0f, 0.0f}, NewVec4(NewColor(Color_White)), {0.0f, 1.0f} },
		{  {1.0f, 0.0f, 0.0f}, NewVec4(NewColor(Color_White)), {1.0f, 0.0f} },
		{  {1.0f, 1.0f, 0.0f}, NewVec4(NewColor(Color_White)), {1.0f, 1.0f} },
	};
	renderState->squareBuffer = CreateVertexBuffer(squareVertices, ArrayCount(squareVertices));
	
	renderState->gradientTexture = LoadTexture("Resources/Textures/gradient.png", false, false);
	renderState->circleTexture = LoadTexture("Resources/Sprites/circle.png", false, false);
	
	Color_t textureData = {Color_White};
	renderState->dotTexture = CreateTexture((u8*)&textureData, 1, 1);
	
	renderState->viewport = NewRec(0, 0, (r32)RenderScreenSize.x, (r32)RenderScreenSize.y);
	renderState->worldMatrix = Mat4_Identity;
	renderState->viewMatrix = Mat4_Identity;
	renderState->projectionMatrix = Mat4_Identity;
	renderState->doGrayscaleGradient = false;
	renderState->useAlphaTexture = false;
	renderState->sourceRectangle = NewRec(0, 0, 1, 1);
	renderState->depth = 1.0f;
	renderState->circleRadius = 0.0f;
	renderState->circleInnerRadius = 0.0f;
	renderState->color = NewColor(Color_White);
	renderState->secondaryColor = NewColor(Color_White);
}


//+================================================================+
//|                     State Change Functions                     |
//+================================================================+
void RsUpdateShader()
{
	Assert(renderState->boundShader != nullptr);
	
	if (renderState->boundBuffer != nullptr)
	{
		glBindVertexArray(renderState->boundShader->vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, renderState->boundBuffer->id);
		glVertexAttribPointer(renderState->boundShader->locations.positionAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)0);
		glVertexAttribPointer(renderState->boundShader->locations.colorAttrib,    4, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)sizeof(v3));
		glVertexAttribPointer(renderState->boundShader->locations.texCoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)(sizeof(v3)+sizeof(v4)));
	}
	
	if (renderState->boundTexture != nullptr)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderState->boundTexture->id);
		glUniform1i(renderState->boundShader->locations.diffuseTexture, 0);
		glUniform2f(renderState->boundShader->locations.textureSize, (r32)renderState->boundTexture->width, (r32)renderState->boundTexture->height);
	}
	
	if (renderState->boundAlphaTexture != nullptr)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderState->boundAlphaTexture->id);
		glUniform1i(renderState->boundShader->locations.alphaTexture, 1);
		glUniform1i(renderState->boundShader->locations.useAlphaTexture, 1);
	}
	else
	{
		glUniform1i(renderState->boundShader->locations.useAlphaTexture, 0);
	}
	
	if (renderState->boundFrameBuffer != nullptr)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, renderState->boundFrameBuffer->id);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	glUniformMatrix4fv(renderState->boundShader->locations.worldMatrix,      1, GL_FALSE, &renderState->worldMatrix.values[0][0]);
	glUniformMatrix4fv(renderState->boundShader->locations.viewMatrix,       1, GL_FALSE, &renderState->viewMatrix.values[0][0]);
	glUniformMatrix4fv(renderState->boundShader->locations.projectionMatrix, 1, GL_FALSE, &renderState->projectionMatrix.values[0][0]);
	
	glUniform1i(renderState->boundShader->locations.doGrayscaleGradient, renderState->doGrayscaleGradient ? 1 : 0);
	glUniform4f(renderState->boundShader->locations.sourceRectangle, renderState->sourceRectangle.x, renderState->sourceRectangle.y, renderState->sourceRectangle.width, renderState->sourceRectangle.height);
	glUniform1f(renderState->boundShader->locations.circleRadius, renderState->circleRadius);
	glUniform1f(renderState->boundShader->locations.circleInnerRadius, renderState->circleInnerRadius);
	
	v4 colorVec = NewVec4(renderState->color);
	glUniform4f(renderState->boundShader->locations.diffuseColor, colorVec.r, colorVec.g, colorVec.b, colorVec.a);
	colorVec = NewVec4(renderState->secondaryColor);
	glUniform4f(renderState->boundShader->locations.secondaryColor, colorVec.r, colorVec.g, colorVec.b, colorVec.a);
}

void RsBindShader(const Shader_t* shaderPntr)
{
	renderState->boundShader = shaderPntr;
	
	glUseProgram(shaderPntr->programId);
}

void RsBindFont(const Font_t* fontPntr)
{
	renderState->boundFont = fontPntr;
}

void RsBindTexture(const Texture_t* texturePntr)
{
	renderState->boundTexture = texturePntr;
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderState->boundTexture->id);
	glUniform1i(renderState->boundShader->locations.diffuseTexture, 0);
	glUniform2f(renderState->boundShader->locations.textureSize, (r32)renderState->boundTexture->width, (r32)renderState->boundTexture->height);
}

void RsBindAlphaTexture(const Texture_t* texturePntr)
{
	renderState->boundAlphaTexture = texturePntr;
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderState->boundAlphaTexture->id);
	glUniform1i(renderState->boundShader->locations.alphaTexture, 1);
	glUniform1i(renderState->boundShader->locations.useAlphaTexture, 1);
}
void RsDisableAlphaTexture()
{
	renderState->boundAlphaTexture = nullptr;
	
	glUniform1i(renderState->boundShader->locations.useAlphaTexture, 0);
}

void RsBindBuffer(const VertexBuffer_t* vertBufferPntr)
{
	renderState->boundBuffer = vertBufferPntr;
	
	glBindVertexArray(renderState->boundShader->vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertBufferPntr->id);
	glVertexAttribPointer(renderState->boundShader->locations.positionAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)0);
	glVertexAttribPointer(renderState->boundShader->locations.colorAttrib,    4, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)sizeof(v3));
	glVertexAttribPointer(renderState->boundShader->locations.texCoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)(sizeof(v3)+sizeof(v4)));
}

void RsBindFrameBuffer(const FrameBuffer_t* frameBuffer)
{
	renderState->boundFrameBuffer = frameBuffer;
	
	if (frameBuffer == nullptr)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->id);
	}
}

void RsSetWorldMatrix(const mat4& worldMatrix)
{
	renderState->worldMatrix = worldMatrix;
	glUniformMatrix4fv(renderState->boundShader->locations.worldMatrix, 1, GL_FALSE, &worldMatrix.values[0][0]);
}
void RsSetViewMatrix(const mat4& viewMatrix)
{
	renderState->viewMatrix = viewMatrix;
	glUniformMatrix4fv(renderState->boundShader->locations.viewMatrix, 1, GL_FALSE, &viewMatrix.values[0][0]);
}
void RsSetProjectionMatrix(const mat4& projectionMatrix)
{
	renderState->projectionMatrix = projectionMatrix;
	glUniformMatrix4fv(renderState->boundShader->locations.projectionMatrix, 1, GL_FALSE, &projectionMatrix.values[0][0]);
}

void RsSetViewport(rec viewport)
{
	renderState->viewport = viewport;
	
	glViewport(
		(i32)renderState->viewport.x, 
		(i32)(RenderScreenSize.y - renderState->viewport.height - renderState->viewport.y), 
		(i32)renderState->viewport.width, 
		(i32)renderState->viewport.height
	);
	
	mat4 projMatrix;
	projMatrix = Mat4Scale(NewVec3(2.0f/viewport.width, -2.0f/viewport.height, 1.0f));
	projMatrix = Mat4Multiply(projMatrix, Mat4Translate(NewVec3(-viewport.width/2.0f, -viewport.height/2.0f, 0.0f)));
	projMatrix = Mat4Multiply(projMatrix, Mat4Translate(NewVec3(-renderState->viewport.x, -renderState->viewport.y, 0.0f)));
	RsSetProjectionMatrix(projMatrix);
}

void RsSetColor(Color_t color)
{
	renderState->color = color;
	
	v4 colorVec = NewVec4(color);
	glUniform4f(renderState->boundShader->locations.diffuseColor, colorVec.r, colorVec.g, colorVec.b, colorVec.a);
}
void RsSetSecondaryColor(Color_t color)
{
	renderState->secondaryColor = color;
	
	v4 colorVec = NewVec4(color);
	glUniform4f(renderState->boundShader->locations.secondaryColor, colorVec.r, colorVec.g, colorVec.b, colorVec.a);
}

void RsSetSourceRectangle(rec sourceRectangle)
{
	renderState->sourceRectangle = sourceRectangle;
	
	glUniform4f(renderState->boundShader->locations.sourceRectangle, sourceRectangle.x, sourceRectangle.y, sourceRectangle.width, sourceRectangle.height);
}

void RsSetGradientEnabled(bool doGradient)
{
	renderState->doGrayscaleGradient = doGradient;
	
	glUniform1i(renderState->boundShader->locations.doGrayscaleGradient, doGradient ? 1 : 0);
}

void RsSetCircleRadius(r32 radius, r32 innerRadius = 0.0f)
{
	renderState->circleRadius = radius;
	renderState->circleInnerRadius = innerRadius;
	
	glUniform1f(renderState->boundShader->locations.circleRadius, radius);
	glUniform1f(renderState->boundShader->locations.circleInnerRadius, innerRadius);
}

void RsSetDepth(r32 depth)
{
	renderState->depth = depth;
}

void RsBegin(const Shader_t* startShader, const Font_t* startFont, rec viewport)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.1f);
	
	RsBindShader(startShader);
	RsBindFont(startFont);
	RsBindTexture(&renderState->dotTexture);
	RsBindBuffer(&renderState->squareBuffer);
	RsBindFrameBuffer(nullptr);
	RsDisableAlphaTexture();
	
	RsSetWorldMatrix(Matrix4_Identity);
	RsSetViewMatrix(Matrix4_Identity);
	RsSetProjectionMatrix(Matrix4_Identity);
	RsSetViewport(viewport);
	RsSetColor(NewColor(Color_White));
	RsSetSecondaryColor(NewColor(Color_White));
	RsSetSourceRectangle(NewRec(0, 0, 1, 1));
	RsSetGradientEnabled(false);
	RsSetCircleRadius(0.0f, 0.0f);
	RsSetDepth(1.0f);
}

// +--------------------------------------------------------------+
// |                      Drawing Functions                       |
// +--------------------------------------------------------------+
void RsClearColorBuffer(Color_t clearColor)
{
	v4 clearColorVec = NewVec4(clearColor);
	glClearColor(clearColorVec.x, clearColorVec.y, clearColorVec.z, clearColorVec.w);
	glClear(GL_COLOR_BUFFER_BIT);
}
void RsClearDepthBuffer(r32 clearDepth)
{
	glClearDepth(clearDepth);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void RsDrawTexturedRec(rec rectangle, Color_t color)
{
	RsBindTexture(renderState->boundTexture);
	RsSetSourceRectangle(NewRec(0, 0, 1, 1));
	RsSetColor(color);
	mat4 worldMatrix = Mat4Multiply(
		Mat4Translate(NewVec3(rectangle.x, rectangle.y, renderState->depth)), //Position
		Mat4Scale(NewVec3(rectangle.width, rectangle.height, 1.0f))); //Scale
	RsSetWorldMatrix(worldMatrix);
	RsBindBuffer(&renderState->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, renderState->squareBuffer.numVertices);
}
void RsDrawTexturedRec(rec rectangle, Color_t color, rec sourceRectangle)
{
	rec realSourceRec = NewRec(
		sourceRectangle.x / (r32)renderState->boundTexture->width,
		sourceRectangle.y / (r32)renderState->boundTexture->height,
		sourceRectangle.width / (r32)renderState->boundTexture->width,
		sourceRectangle.height / (r32)renderState->boundTexture->height);
	RsSetSourceRectangle(realSourceRec);
	RsBindTexture(renderState->boundTexture);
	RsSetColor(color);
	mat4 worldMatrix = Mat4Multiply(
		Mat4Translate(NewVec3(rectangle.x, rectangle.y, renderState->depth)), //Position
		Mat4Scale(NewVec3(rectangle.width, rectangle.height, 1.0f))); //Scale
	RsSetWorldMatrix(worldMatrix);
	RsBindBuffer(&renderState->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, renderState->squareBuffer.numVertices);
}

void RsDrawRectangle(rec rectangle, Color_t color)
{
	RsBindTexture(&renderState->dotTexture);
	RsSetSourceRectangle(NewRec(0, 0, 1, 1));
	RsSetColor(color);
	mat4 worldMatrix = Mat4Multiply(
		Mat4Translate(NewVec3(rectangle.x, rectangle.y, renderState->depth)), //Position
		Mat4Scale(NewVec3(rectangle.width, rectangle.height, 1.0f))); //Scale
	RsSetWorldMatrix(worldMatrix);
	RsBindBuffer(&renderState->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, renderState->squareBuffer.numVertices);
}

void RsDrawButton(rec rectangle, Color_t backgroundColor, Color_t borderColor, r32 borderWidth = 1.0f)
{
	RsDrawRectangle(rectangle, backgroundColor);
	
	RsDrawRectangle(NewRec(rectangle.x, rectangle.y, rectangle.width, borderWidth), borderColor);
	RsDrawRectangle(NewRec(rectangle.x, rectangle.y, borderWidth, rectangle.height), borderColor);
	RsDrawRectangle(NewRec(rectangle.x, rectangle.y + rectangle.height - borderWidth, rectangle.width, borderWidth), borderColor);
	RsDrawRectangle(NewRec(rectangle.x + rectangle.width - borderWidth, rectangle.y, borderWidth, rectangle.height), borderColor);
}

void RsDrawGradient(rec rectangle, Color_t color1, Color_t color2, Dir2_t direction)
{
	RsBindTexture(&renderState->gradientTexture);
	RsSetSourceRectangle(NewRec(0, 0, 1, 1));
	RsSetColor(color1);
	RsSetSecondaryColor(color2);
	RsSetGradientEnabled(true);
	
	mat4 worldMatrix = Mat4_Identity;
	switch (direction)
	{
		case Dir2_Right:
		default:
		{
			worldMatrix = Mat4Multiply(
				Mat4Translate(NewVec3(rectangle.x, rectangle.y, renderState->depth)),
				Mat4Scale(NewVec3(rectangle.width, rectangle.height, 1.0f)));
		} break;
		
		case Dir2_Left:
		{
			worldMatrix = Mat4Multiply(
				Mat4Translate(NewVec3(rectangle.x + rectangle.width, rectangle.y, renderState->depth)),
				Mat4Scale(NewVec3(-rectangle.width, rectangle.height, 1.0f)));
		} break;
		
		case Dir2_Down:
		{
			worldMatrix = Mat4Multiply(
				Mat4Translate(NewVec3(rectangle.x + rectangle.width, rectangle.y, renderState->depth)),
				Mat4RotateZ(ToRadians(90)),
				Mat4Scale(NewVec3(rectangle.height, rectangle.width, 1.0f)));
		} break;
		
		case Dir2_Up:
		{
			worldMatrix = Mat4Multiply(
				Mat4Translate(NewVec3(rectangle.x + rectangle.width, rectangle.y + rectangle.height, renderState->depth)),
				Mat4RotateZ(ToRadians(90)),
				Mat4Scale(NewVec3(-rectangle.height, rectangle.width, 1.0f)));
		} break;
	};
	RsSetWorldMatrix(worldMatrix);
	
	RsBindBuffer(&renderState->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, renderState->squareBuffer.numVertices);
	
	RsSetGradientEnabled(false);
}

void RsDrawLine(v2 p1, v2 p2, r32 thickness, Color_t color)
{
	RsBindTexture(&renderState->dotTexture);
	RsSetSourceRectangle(NewRec(0, 0, 1, 1));
	RsSetColor(color);
	r32 length = Vec2Length(p2 - p1);
	r32 rotation = AtanR32(p2.y - p1.y, p2.x - p1.x); 
	mat4 worldMatrix = Mat4_Identity;
	worldMatrix = Mat4Multiply(Mat4Translate(NewVec3(0.0f, -0.5f, 0.0f)),              worldMatrix); //Centering
	worldMatrix = Mat4Multiply(Mat4Scale(NewVec3(length, thickness, 1.0f)),            worldMatrix); //Scale
	worldMatrix = Mat4Multiply(Mat4RotateZ(rotation),                                  worldMatrix); //Rotation
	worldMatrix = Mat4Multiply(Mat4Translate(NewVec3(p1.x, p1.y, renderState->depth)), worldMatrix); //Position
	RsSetWorldMatrix(worldMatrix);
	RsBindBuffer(&renderState->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, renderState->squareBuffer.numVertices);
}

void RsDrawCircle(v2 center, r32 radius, Color_t color)
{
	RsSetCircleRadius(1.0f, 0.0f);
	RsDrawRectangle(NewRec(center.x - radius, center.y - radius, radius*2, radius*2), color);
	RsSetCircleRadius(0.0f, 0.0f);
}

void RsDrawDonut(v2 center, r32 radius, r32 innerRadius, Color_t color)
{
	r32 realInnerRadius = ClampR32(innerRadius / radius, 0.0f, 1.0f);
	RsSetCircleRadius(1.0f, realInnerRadius);
	RsDrawRectangle(NewRec(center.x - radius, center.y - radius, radius*2, radius*2), color);
	RsSetCircleRadius(0.0f, 0.0f);
}

void RsDrawCharacter(u32 charIndex, v2 bottomLeft, Color_t color, r32 scale = 1.0f)
{
	const FontCharInfo_t* charInfo = &renderState->boundFont->chars[charIndex];
	
	rec sourceRectangle = NewRec((r32)charInfo->x, (r32)charInfo->y, (r32)charInfo->width, (r32)charInfo->height);
	rec drawRectangle = NewRec(
		bottomLeft.x + scale*charInfo->offset.x, 
		bottomLeft.y + scale*charInfo->offset.y, 
		scale*charInfo->width, 
		scale*charInfo->height);
	
	if (renderState->boundTexture != &renderState->boundFont->bitmap)
	{
		RsBindTexture(&renderState->boundFont->bitmap);
	}
	
	RsDrawTexturedRec(drawRectangle, color, sourceRectangle);
}

void RsDrawString(const char* string, u32 numCharacters, v2 position, Color_t color, r32 scale = 1.0f, Alignment_t alignment = Alignment_Left)
{
	RsBindTexture(&renderState->boundFont->bitmap);
	
	v2 stringSize = MeasureString(renderState->boundFont, string, numCharacters);
	
	v2 currentPos = position;
	switch (alignment)
	{
		case Alignment_Center: currentPos.x -= stringSize.x/2; break;
		case Alignment_Right:  currentPos.x -= stringSize.x; break;
		case Alignment_Left:   break;
	};
	
	for (u32 cIndex = 0; cIndex < numCharacters; cIndex++)
	{
		if (string[cIndex] == '\t')
		{
			u32 spaceIndex = GetFontCharIndex(renderState->boundFont, ' ');
			currentPos.x += renderState->boundFont->chars[spaceIndex].advanceX * GC->tabWidth * scale;
		}
		else if (IsCharClassPrintable(string[cIndex]) == false)
		{
			//Don't do anything
		}
		else
		{
			u32 charIndex = GetFontCharIndex(renderState->boundFont, string[cIndex]);
			RsDrawCharacter(charIndex, currentPos, color, scale);
			currentPos.x += renderState->boundFont->chars[charIndex].advanceX * scale;
		}
	}
}

void RsDrawString(const char* nullTermString, v2 position, Color_t color, r32 scale = 1.0f, Alignment_t alignment = Alignment_Left)
{
	RsDrawString(nullTermString, (u32)strlen(nullTermString), position, color, scale, alignment);
}

void RsPrintString(v2 position, Color_t color, r32 scale, const char* formatString, ...)
{
	char printBuffer[256] = {};
	va_list args;
	
	va_start(args, formatString);
	u32 length = (u32)vsnprintf(printBuffer, 256-1, formatString, args);
	va_end(args);
	
	RsDrawString(printBuffer, length, position, color, scale);
}

void RsDrawFormattedString(const char* string, u32 numCharacters, v2 position, r32 maxWidth, Color_t color, Alignment_t alignment = Alignment_Left, bool preserveWords = true)
{
	u32 cIndex = 0;
	v2 drawPos = position;
	while (cIndex < numCharacters)
	{
		u32 numChars = FindNextFormatChunk(renderState->boundFont, &string[cIndex], numCharacters - cIndex, maxWidth, preserveWords);
		if (numChars == 0) { numChars = 1; }
		
		while (numChars > 1 && IsCharClassWhitespace(string[cIndex + numChars-1]))
		{
			numChars--;
		}
		
		RsDrawString(&string[cIndex], numChars, drawPos, color, 1.0f, alignment);
		
		if (cIndex+numChars < numCharacters && string[cIndex+numChars] == '\r')
		{
			numChars++;
		}
		if (cIndex+numChars < numCharacters && string[cIndex+numChars] == '\n')
		{
			numChars++;
		}
		while (cIndex+numChars < numCharacters && string[cIndex+numChars] == ' ')
		{
			numChars++;
		}
		drawPos.y += renderState->boundFont->lineHeight;
		cIndex += numChars;
	}
}

void RsDrawFormattedString(const char* nullTermString, v2 position, r32 maxWidth, Color_t color, Alignment_t alignment = Alignment_Left, bool preserveWords = true)
{
	u32 numCharacters = (u32)strlen(nullTermString);
	RsDrawFormattedString(nullTermString, numCharacters, position, maxWidth, color, alignment, preserveWords);
}

