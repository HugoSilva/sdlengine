attribute highp vec4 position;
attribute highp vec2 uv;
attribute highp float tid;
attribute highp vec4 color;

uniform highp mat4 projection;
uniform highp mat4 view;

varying highp vec2 vs_uv;
varying highp float vs_tid;
varying highp vec4 vs_color;

void main()
{
	gl_Position = projection * view * position;
	vs_uv = uv;
	vs_tid = tid;
	vs_color = color;
}
