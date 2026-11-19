#version 330 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in float slot;
layout (location = 3) in vec4 color;

uniform mat4 projection;
uniform mat4 view;

out DATA
{
	vec2 uv;
	float slot;
	vec4 color;
} vs_out;

void main()
{
	gl_Position = projection * view * position;
	vs_out.uv = uv;
	vs_out.slot = slot;
	vs_out.color = color;
}
