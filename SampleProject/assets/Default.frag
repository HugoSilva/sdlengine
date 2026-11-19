#version 330 core

layout (location = 0) out vec4 color;

// One sampler per texture unit. A batch fills units 0..n-1 with the textures it
// draws, and a vertex names its unit in `slot`; -1 means the quad is untextured
// and keeps its vertex colour alone.
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
uniform sampler2D texture6;
uniform sampler2D texture7;
uniform sampler2D texture8;
uniform sampler2D texture9;
uniform sampler2D texture10;
uniform sampler2D texture11;
uniform sampler2D texture12;
uniform sampler2D texture13;
uniform sampler2D texture14;
uniform sampler2D texture15;

in DATA
{
	vec2 uv;
	float slot;
	vec4 color;
} fs_in;

// GLSL 330 only guarantees sampler indexing by a dynamically uniform
// expression, so the sixteen units are selected by hand instead of indexing a
// sampler array with `slot`. Every vertex of a quad carries the same slot, so
// the interpolated value is an exact small integer.
vec4 SampleSlot(int slot, vec2 uv)
{
	switch (slot)
	{
		case 0: return texture(texture0, uv);
		case 1: return texture(texture1, uv);
		case 2: return texture(texture2, uv);
		case 3: return texture(texture3, uv);
		case 4: return texture(texture4, uv);
		case 5: return texture(texture5, uv);
		case 6: return texture(texture6, uv);
		case 7: return texture(texture7, uv);
		case 8: return texture(texture8, uv);
		case 9: return texture(texture9, uv);
		case 10: return texture(texture10, uv);
		case 11: return texture(texture11, uv);
		case 12: return texture(texture12, uv);
		case 13: return texture(texture13, uv);
		case 14: return texture(texture14, uv);
		case 15: return texture(texture15, uv);
	}
	return vec4(1.0);
}

void main()
{
	if (fs_in.slot < 0.0)
	{
		color = fs_in.color;
		return;
	}

	color = fs_in.color * SampleSlot(int(fs_in.slot), fs_in.uv);
}
