// Vertex Shader
#version 450 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 TextureCoords;
layout(location = 3) in float TextureIndex;
layout(location = 4) in float TilingFactor;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec4 Color;
	vec2 TextureCoords;
	float TilingFactor;
};

layout (location = 0) out VertexOutput Output;
layout (location = 3) out flat float v_TextureIndex;

void main()
{
	Output.Color = Color;
	Output.TextureCoords = TextureCoords;
	Output.TilingFactor = TilingFactor;
	v_TextureIndex = TextureIndex;

	gl_Position = u_ViewProjection * vec4(Position, 1.0);
}
