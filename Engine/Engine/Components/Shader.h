#pragma once

class Shader
{
public:
	Shader(const char* vertexPath, const char* fragmentPath);

	void Enable() const;
	void Disable() const;

	unsigned int getUniformLocation(const char* name);
	void setUniform1iv(const char* name, int* value, int count);
	void setUniformMat4(const char* name, const float* matrix);

	unsigned int GetShaderID() const { return m_ShaderID; };

private:
	unsigned int m_ShaderID;

	const char* m_VertPath;
	const char* m_FragPath;
};