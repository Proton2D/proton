#version 450 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec4 Color;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

layout (location = 0) out vec4 OutputColor;

void main()
{
	OutputColor = Color;

	gl_Position = u_ViewProjection * vec4(Position, 1.0);
}