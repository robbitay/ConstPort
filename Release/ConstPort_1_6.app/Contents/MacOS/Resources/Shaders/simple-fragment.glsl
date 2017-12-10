#version 330

uniform vec4 DiffuseColor;
uniform vec4 SecondaryColor;
uniform sampler2D DiffuseTexture;
uniform sampler2D AlphaTexture;
uniform bool DoGrayscaleGradient;
uniform bool UseAlphaTexture;
uniform float CircleRadius;
uniform float CircleInnerRadius;

in vec4 fColor;
in vec2 fTexCoord;
in vec2 fSampleCoord;

out vec4 frag_colour;

void main()
{
	vec4 sampleColor = fColor * texture(DiffuseTexture, fSampleCoord);
	
	if (DoGrayscaleGradient)
	{
		float gray = dot(sampleColor.rgb, vec3(0.299, 0.587, 0.114));
		frag_colour = mix(DiffuseColor.rgba, SecondaryColor.rgba, gray);
		// frag_colour = vec4(fSampleCoord.x, fSampleCoord.y, 0.0, 1.0);
	}
	else
	{
		frag_colour = DiffuseColor * sampleColor;
	}
	
	if (UseAlphaTexture)
	{
		frag_colour.a *= texture(AlphaTexture, fSampleCoord).a;
	}
	
	if (CircleRadius != 0)
	{
		float distFromCenter = length(fTexCoord - vec2(0.5, 0.5)) * 2;
		// frag_colour.rgb = 1 - vec3(distFromCenter);
		float smoothDelta = fwidth(distFromCenter);
		frag_colour.a *= smoothstep(CircleRadius, CircleRadius-smoothDelta, distFromCenter);
		// if (distFromCenter < CircleInnerRadius)
		// {
		// 	frag_colour.a = 0;
		// }
		if (CircleInnerRadius != 0)
		{
			frag_colour.a *= smoothstep(CircleInnerRadius, CircleInnerRadius+smoothDelta, distFromCenter);
		}
	}
}