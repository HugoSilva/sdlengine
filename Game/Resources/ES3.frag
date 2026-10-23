precision highp float;

uniform sampler2D texture0;

varying highp vec2 vs_uv;
varying highp float vs_tid;
varying highp vec4 vs_color;

void main()
{
	// One texture unit per batch; `vs_tid` only says whether this vertex is
	// textured. Untextured geometry keeps its vertex colour.
	vec4 texColor = vs_color;
	if (vs_tid > 0.5)
	{
		texColor = vs_color * texture2D(texture0, vs_uv);
	}
	gl_FragColor = texColor;
}
