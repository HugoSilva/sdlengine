precision highp float;

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

varying highp vec2 vs_uv;
varying highp float vs_slot;
varying highp vec4 vs_color;

// This stage is GLSL ES 1.00, which has no switch, so the units are picked with
// a comparison chain. Every vertex of a quad carries the same slot, so the
// interpolated value is an exact small integer.
vec4 SampleSlot(float slot, vec2 uv)
{
	if (slot < 0.5) return texture2D(texture0, uv);
	if (slot < 1.5) return texture2D(texture1, uv);
	if (slot < 2.5) return texture2D(texture2, uv);
	if (slot < 3.5) return texture2D(texture3, uv);
	if (slot < 4.5) return texture2D(texture4, uv);
	if (slot < 5.5) return texture2D(texture5, uv);
	if (slot < 6.5) return texture2D(texture6, uv);
	if (slot < 7.5) return texture2D(texture7, uv);
	if (slot < 8.5) return texture2D(texture8, uv);
	if (slot < 9.5) return texture2D(texture9, uv);
	if (slot < 10.5) return texture2D(texture10, uv);
	if (slot < 11.5) return texture2D(texture11, uv);
	if (slot < 12.5) return texture2D(texture12, uv);
	if (slot < 13.5) return texture2D(texture13, uv);
	if (slot < 14.5) return texture2D(texture14, uv);
	return texture2D(texture15, uv);
}

void main()
{
	if (vs_slot < 0.0)
	{
		gl_FragColor = vs_color;
		return;
	}

	gl_FragColor = vs_color * SampleSlot(vs_slot, vs_uv);
}
