#version 400

uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform vec4 SourceRectangle;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;

out vec3 fPosition;
out vec4 fColor;
out vec2 fTexCoord;

void main()
{
	fColor = inColor;
	fTexCoord = SourceRectangle.xy + (inTexCoord * SourceRectangle.zw);
	mat4 transformMatrix = ProjectionMatrix * (ViewMatrix * WorldMatrix);
	gl_Position = transformMatrix * vec4(inPosition, 1.0);
	fPosition = gl_Position.xyz;
}