#include "Shader.h"

#include <string>
#ifdef EMSCRIPTEN
	#define GLFW_INCLUDE_ES3
	#include <GLFW/glfw3.h>
#else
	#include <GL/glew.h>
#endif // EMSCRIPTEN

#include "../Utils/File.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
	:m_VertPath(vertexPath), m_FragPath(fragmentPath)
{
	std::string vertString = File::read(m_VertPath);
	const char* vShaderCode = vertString.c_str();
	std::string fragString = File::read(m_FragPath);
	const char* fShaderCode = fragString.c_str();

	m_ShaderID = glCreateProgram();
	unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
	int success;
	char infoLog[512];

	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);

	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		Logger::error("Shader error vertex compilation failed");
		std::string log(infoLog);
		Logger::warning(log);
	}

	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		Logger::error("Shader error fragment compilation failed");
		std::string log(infoLog);
		Logger::warning(log);
	}

	glAttachShader(m_ShaderID, vertex);
	glAttachShader(m_ShaderID, fragment);

	glLinkProgram(m_ShaderID);

	glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(m_ShaderID, 512, NULL, infoLog);
		Logger::error("Shader error linking failed");
		std::string log(infoLog);
		Logger::warning(log);
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::Enable() const
{
	glUseProgram(m_ShaderID);
}

void Shader::Disable() const
{
	glUseProgram(0);
}

unsigned int Shader::getUniformLocation(const char* name)
{
	return glGetUniformLocation(m_ShaderID, name);
}

void Shader::setUniform1iv(const char* name, int* value, int count)
{
	glUniform1iv(getUniformLocation(name), count, value);
}

void Shader::setUniformMat4(const char* name, const float* matrix)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, matrix);
}