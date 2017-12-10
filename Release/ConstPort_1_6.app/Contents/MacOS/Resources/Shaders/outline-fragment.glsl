#version 330

uniform vec4      DiffuseColor;
uniform vec4      SecondaryColor;
uniform sampler2D DiffuseTexture;
uniform sampler2D AlphaTexture;
uniform bool      UseAlphaTexture;
uniform vec2      TextureSize;

in vec4 fColor;
in vec2 fTexCoord;

out vec4 frag_colour; //layout(location = 0)

void main()
{
	vec2 pixelSize = vec2(1.0 / TextureSize.x, 1.0 / TextureSize.y);
	vec4 sampleColor = texture(DiffuseTexture, fTexCoord);
	
	if (false) //Simple blur
	{
		sampleColor = sampleColor * 4;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2(-1,-2)*pixelSize) * 1;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 0,-2)*pixelSize) * 2;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 1,-2)*pixelSize) * 1;
		
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2(-2,-1)*pixelSize) * 1;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2(-1,-1)*pixelSize) * 2;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 0,-1)*pixelSize) * 4;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 1,-1)*pixelSize) * 2;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 2,-1)*pixelSize) * 1;
		
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2(-2, 0)*pixelSize) * 2;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2(-1, 0)*pixelSize) * 4;
		// sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 0, 0)*pixelSize) * 8;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 1, 0)*pixelSize) * 4;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 2, 0)*pixelSize) * 2;
		
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2(-2, 1)*pixelSize) * 1;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2(-1, 1)*pixelSize) * 2;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 0, 1)*pixelSize) * 4;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 1, 1)*pixelSize) * 2;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 2, 1)*pixelSize) * 1;
		
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2(-1, 2)*pixelSize) * 1;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 0, 2)*pixelSize) * 2;
		sampleColor = sampleColor + texture(DiffuseTexture, fTexCoord + vec2( 1, 2)*pixelSize) * 1;
		sampleColor = sampleColor / 44;
	}
	
	frag_colour = fColor * DiffuseColor * sampleColor;
	
	if (UseAlphaTexture)
	{
		frag_colour.a *= texture(AlphaTexture, fTexCoord).a;
	}
	
	vec4 outlineColor = vec4(frag_colour.rgb, 1.0);
	vec4 innerColor = frag_colour * vec4(1, 1, 1, SecondaryColor.a);
	if (frag_colour.a >= 0.5)
	{
		// if (texture(DiffuseTexture, fTexCoord + vec2(-1,-2)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		// else if (texture(DiffuseTexture, fTexCoord + vec2( 0,-2)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		// else if (texture(DiffuseTexture, fTexCoord + vec2( 1,-2)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		// else if (texture(DiffuseTexture, fTexCoord + vec2(-2,-1)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		if (texture(DiffuseTexture, fTexCoord + vec2(-1,-1)*pixelSize).a < 0.5)
			frag_colour = outlineColor;
		else if (texture(DiffuseTexture, fTexCoord + vec2( 0,-1)*pixelSize).a < 0.5)
			frag_colour = outlineColor;
		else if (texture(DiffuseTexture, fTexCoord + vec2( 1,-1)*pixelSize).a < 0.5)
			frag_colour = outlineColor;
		// else if (texture(DiffuseTexture, fTexCoord + vec2( 2,-1)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		// else if (texture(DiffuseTexture, fTexCoord + vec2(-2, 0)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		else if (texture(DiffuseTexture, fTexCoord + vec2(-1, 0)*pixelSize).a < 0.5)
			frag_colour = outlineColor;
		// else if (texture(DiffuseTexture, fTexCoord + vec2( 0, 0)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		else if (texture(DiffuseTexture, fTexCoord + vec2( 1, 0)*pixelSize).a < 0.5)
			frag_colour = outlineColor;
		// else if (texture(DiffuseTexture, fTexCoord + vec2( 2, 0)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		// else if (texture(DiffuseTexture, fTexCoord + vec2(-2, 1)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		else if (texture(DiffuseTexture, fTexCoord + vec2(-1, 1)*pixelSize).a < 0.5)
			frag_colour = outlineColor;
		else if (texture(DiffuseTexture, fTexCoord + vec2( 0, 1)*pixelSize).a < 0.5)
			frag_colour = outlineColor;
		else if (texture(DiffuseTexture, fTexCoord + vec2( 1, 1)*pixelSize).a < 0.5)
			frag_colour = outlineColor;
		// else if (texture(DiffuseTexture, fTexCoord + vec2( 2, 1)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		// else if (texture(DiffuseTexture, fTexCoord + vec2(-1, 2)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		// else if (texture(DiffuseTexture, fTexCoord + vec2( 0, 2)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		// else if (texture(DiffuseTexture, fTexCoord + vec2( 1, 2)*pixelSize).a < 0.5)
		// 	frag_colour = outlineColor;
		else
			frag_colour = innerColor;
	}
	else
	{
		frag_colour = innerColor;
	}
}