#version 330 core

layout (location = 0) out vec4 color;

uniform sampler2D texture0;

in DATA
{
	vec2 uv;
	float tid;
	vec4 color;
} fs_in;

void main()
{
	// `tid` is the textured flag now, not a sampler index: the renderer binds
	// one texture per batch to unit 0 and submits an untextured quad with tid 0.
	vec4 texColor = fs_in.color;
	if (fs_in.tid > 0.5)
	{
		texColor = fs_in.color * texture(texture0, fs_in.uv);
	}
	color = texColor;
}
