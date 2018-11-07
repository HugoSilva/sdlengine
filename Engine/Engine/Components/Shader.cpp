#include "Shader.hpp"

#include <string>
#ifdef EMSCRIPTEN
	#include <SDL_opengles2.h>
#else
	#include <GL/glew.h>
#endif // EMSCRIPTEN

#include "../Utils/File.hpp"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
	:m_VertPath(vertexPath), m_FragPath(fragmentPath)
{
	std::string vertString = File::read(m_VertPath);
	std::string fragString = File::read(m_FragPath);

	m_ShaderID = load(vertString.c_str(), fragString.c_str());
}

Shader::Shader(const char* name, const char* vertexSource, const char* fragmentSource)
	: m_Name(name), m_VertSource(vertexSource), m_FragSource(fragmentSource)
{
	m_ShaderID = load(m_VertSource, m_FragSource);
}

Shader::~Shader()
{
	glDeleteProgram(m_ShaderID);
}

void Shader::enable() const
{
	glUseProgram(m_ShaderID);
}

void Shader::disable() const
{
	glUseProgram(0);
}

unsigned int Shader::getUniformLocation(const char* name)
{
	int result = glGetUniformLocation(m_ShaderID, name);
	if (result == -1)
	{
		Logger::error("Could not find uniform " + (std::string)name + " in shader!");
	}
	return result;
}

void Shader::setUniform1i(const char* name, int value)
{
	glUniform1i(getUniformLocation(name), value);
}

void Shader::setUniform1iv(const char* name, int* value, int count)
{
	glUniform1iv(getUniformLocation(name), count, value);
}

void Shader::setUniformMat4(const char* name, const float* matrix)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, matrix);
}

Shader* Shader::fromFile(const char* vertexPath, const char* fragmentPath)
{
	return new Shader(vertexPath, fragmentPath);
}

Shader* Shader::fromSource(const char* vertexSource, const char* fragmentSource)
{
	return new Shader(vertexSource, fragmentSource);
}

Shader* Shader::fromSource(const char* name, const char* vertexSource, const char* fragmentSource)
{
	return new Shader(name, vertexSource, fragmentSource);
}

unsigned int Shader::load(const char* vertexSource, const char* fragmentSource)
{
	unsigned int program = glCreateProgram();
	unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
	int success;
	char infoLog[512];

	glShaderSource(vertex, 1, &vertexSource, NULL);
	glCompileShader(vertex);

	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		Logger::error("Shader error vertex compilation failed");
		std::string log(infoLog);
		Logger::warning(log);
	}

	glShaderSource(fragment, 1, &fragmentSource, NULL);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		Logger::error("Shader error fragment compilation failed");
		std::string log(infoLog);
		Logger::warning(log);
	}

	glAttachShader(program, vertex);
	glAttachShader(program, fragment);

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		Logger::error("Shader error linking failed");
		std::string log(infoLog);
		Logger::warning(log);
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	return program;
}