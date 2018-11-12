#include "ShaderFactory.hpp"

namespace ShaderFactory {
#ifdef EMSCRIPTEN
	const char* default_shader_vert =
		"";

	const char* default_shader_frag =
		"";
#else
	const char* default_shader_vert =
		"#version 330 core\n"
		"\n"
		"layout (location = 0) in vec4 position;\n"
		"layout (location = 1) in vec2 uv;\n"
		"layout (location = 2) in float tid;\n"
		"layout (location = 3) in vec4 color;\n"
		"\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view = mat4(1.0);\n"
		"uniform mat4 model = mat4(1.0);\n"
		"\n"
		"out DATA\n"
		"{\n"
		"	vec4 position;\n"
		"	vec2 uv;\n"
		"	float tid;\n"
		"	vec4 color;\n"
		"} vs_out;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = projection * view * model * position;\n"
		"	vs_out.position = model * position;\n"
		"	vs_out.uv = uv;\n"
		"	vs_out.tid = tid;\n"
		"	vs_out.color = color;\n"
		"}\n";

	const char* default_shader_frag =
		"#version 330 core\n"
		"\n"
		"layout (location = 0) out vec4 color;\n"
		"\n"
		"uniform vec4 colour;\n"
		"\n"
		"in DATA\n"
		"{\n"
		"	vec4 position;\n"
		"	vec2 uv;\n"
		"	float tid;\n"
		"	vec4 color;\n"
		"} fs_in;\n"
		"\n"
		"uniform sampler2D textures[32];\n"
		"\n"
		"void main()\n"
		"{\n"
		"	vec4 texColor = fs_in.color;\n"
		"	if (fs_in.tid > 0.0)\n"
		"	{\n"
		"		int tid = int(fs_in.tid - 0.5);\n"
		"		texColor = fs_in.color * texture(textures[tid], fs_in.uv);\n"
		"	}\n"
		"	color = texColor;\n"
		"}\n";
#endif // EMSCRIPTEN

	const char* basic_light_shader_vert =
		"";

	const char* basic_light_shader_frag =
		"";

	rse::Shader* DefaultShader()
	{
		return rse::Shader::fromSource("Default Shader", default_shader_vert, default_shader_frag);
	}

	rse::Shader* BasicLightShader()
	{
		return rse::Shader::fromSource("Basic Light Shader", basic_light_shader_vert, basic_light_shader_frag);
	}
}