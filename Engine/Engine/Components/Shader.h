#pragma once

class Shader
{
public:
	Shader(const char* vertexPath, const char* fragmentPath);
	Shader(const char* name, const char* vertexSource, const char* fragmentSource);
	~Shader();

	void Enable() const;
	void Disable() const;

	unsigned int getUniformLocation(const char* name);
	void setUniform1i(const char* name, int value);
	void setUniform1iv(const char* name, int* value, int count);
	void setUniformMat4(const char* name, const float* matrix);

	static Shader* FromFile(const char* vertexPath, const char* fragmentPath);
	static Shader* FromSource(const char* vertexSource, const char* fragmentSource);
	static Shader* FromSource(const char* name, const char* vertexSource, const char* fragmentSource);

	unsigned int GetShaderID() const { return m_ShaderID; };

private:
	unsigned int Load(const char* vertexSource, const char* fragmentSource);

	unsigned int m_ShaderID;
	const char* m_Name;
	const char* m_VertPath;
	const char* m_FragPath;
	const char* m_VertSource;
	const char* m_FragSource;
};