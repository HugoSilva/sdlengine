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
	// The sprite's own tint, then a warm cast from the material. `tid` is the
	// textured flag: the renderer binds one texture per batch to unit 0.
	vec4 texColor = fs_in.color;
	if (fs_in.tid > 0.5)
	{
		texColor *= texture(texture0, fs_in.uv);
	}

	color = vec4(texColor.rgb * vec3(1.0, 0.82, 0.65), texColor.a);
}
