#version 450 core

layout(location = 0) in vec3 WorldPosition;
layout(location = 1) in vec3 LocalPosition;
layout(location = 2) in vec4 Color;
layout(location = 3) in float Thickness;
layout(location = 4) in float Fade;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec3 LocalPosition;
	vec4 Color;
	float Thickness;
	float Fade;
};

layout (location = 0) out VertexOutput Output;

void main()
{
	Output.LocalPosition = LocalPosition;
	Output.Color = Color;
	Output.Thickness = Thickness;
	Output.Fade = Fade;

	gl_Position = u_ViewProjection * vec4(WorldPosition, 1.0);
}
