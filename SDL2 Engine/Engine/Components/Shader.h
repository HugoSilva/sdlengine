#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <GL/glew.h>

class Shader
{
public:
	Shader(const GLchar *vertexPath, const GLchar *fragmentPath);

	void Enable() const;
	void Disable() const;
	GLuint GetShaderID() const { return m_ShaderID; };

private:
	GLuint m_ShaderID;

	const GLchar* m_VertPath;
	const GLchar* m_FragPath;
};

#endif