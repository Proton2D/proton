// Fragment Shader
#version 450 core

layout(location = 0) out vec4 o_Color;

struct VertexOutput
{
	vec4 Color;
	vec2 TextureCoords;
	float TilingFactor;
};

layout (location = 0) in VertexOutput Input;
layout (location = 3) in flat float v_TextureIndex;

layout (binding = 0) uniform sampler2D u_Textures[32];

void main()
{
	vec4 textureColor = Input.Color;

	switch(int(v_TextureIndex))
	{
		case  0: textureColor *= texture(u_Textures[ 0], Input.TextureCoords * Input.TilingFactor); break;
		case  1: textureColor *= texture(u_Textures[ 1], Input.TextureCoords * Input.TilingFactor); break;
		case  2: textureColor *= texture(u_Textures[ 2], Input.TextureCoords * Input.TilingFactor); break;
		case  3: textureColor *= texture(u_Textures[ 3], Input.TextureCoords * Input.TilingFactor); break;
		case  4: textureColor *= texture(u_Textures[ 4], Input.TextureCoords * Input.TilingFactor); break;
		case  5: textureColor *= texture(u_Textures[ 5], Input.TextureCoords * Input.TilingFactor); break;
		case  6: textureColor *= texture(u_Textures[ 6], Input.TextureCoords * Input.TilingFactor); break;
		case  7: textureColor *= texture(u_Textures[ 7], Input.TextureCoords * Input.TilingFactor); break;
		case  8: textureColor *= texture(u_Textures[ 8], Input.TextureCoords * Input.TilingFactor); break;
		case  9: textureColor *= texture(u_Textures[ 9], Input.TextureCoords * Input.TilingFactor); break;
		case 10: textureColor *= texture(u_Textures[10], Input.TextureCoords * Input.TilingFactor); break;
		case 11: textureColor *= texture(u_Textures[11], Input.TextureCoords * Input.TilingFactor); break;
		case 12: textureColor *= texture(u_Textures[12], Input.TextureCoords * Input.TilingFactor); break;
		case 13: textureColor *= texture(u_Textures[13], Input.TextureCoords * Input.TilingFactor); break;
		case 14: textureColor *= texture(u_Textures[14], Input.TextureCoords * Input.TilingFactor); break;
		case 15: textureColor *= texture(u_Textures[15], Input.TextureCoords * Input.TilingFactor); break;
		case 16: textureColor *= texture(u_Textures[16], Input.TextureCoords * Input.TilingFactor); break;
		case 17: textureColor *= texture(u_Textures[17], Input.TextureCoords * Input.TilingFactor); break;
		case 18: textureColor *= texture(u_Textures[18], Input.TextureCoords * Input.TilingFactor); break;
		case 19: textureColor *= texture(u_Textures[19], Input.TextureCoords * Input.TilingFactor); break;
		case 20: textureColor *= texture(u_Textures[20], Input.TextureCoords * Input.TilingFactor); break;
		case 21: textureColor *= texture(u_Textures[21], Input.TextureCoords * Input.TilingFactor); break;
		case 22: textureColor *= texture(u_Textures[22], Input.TextureCoords * Input.TilingFactor); break;
		case 23: textureColor *= texture(u_Textures[23], Input.TextureCoords * Input.TilingFactor); break;
		case 24: textureColor *= texture(u_Textures[24], Input.TextureCoords * Input.TilingFactor); break;
		case 25: textureColor *= texture(u_Textures[25], Input.TextureCoords * Input.TilingFactor); break;
		case 26: textureColor *= texture(u_Textures[26], Input.TextureCoords * Input.TilingFactor); break;
		case 27: textureColor *= texture(u_Textures[27], Input.TextureCoords * Input.TilingFactor); break;
		case 28: textureColor *= texture(u_Textures[28], Input.TextureCoords * Input.TilingFactor); break;
		case 29: textureColor *= texture(u_Textures[29], Input.TextureCoords * Input.TilingFactor); break;
		case 30: textureColor *= texture(u_Textures[30], Input.TextureCoords * Input.TilingFactor); break;
		case 31: textureColor *= texture(u_Textures[31], Input.TextureCoords * Input.TilingFactor); break;
	}

	if (textureColor.a == 0.0)
		discard;

	o_Color = textureColor;
}