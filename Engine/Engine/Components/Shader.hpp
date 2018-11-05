#pragma once

class Shader
{
public:
	Shader(const char* vertexPath, const char* fragmentPath);
	Shader(const char* name, const char* vertexSource, const char* fragmentSource);
	~Shader();

	void enable() const;
	void disable() const;

	unsigned int getUniformLocation(const char* name);
	void setUniform1i(const char* name, int value);
	void setUniform1iv(const char* name, int* value, int count);
	void setUniformMat4(const char* name, const float* matrix);

	static Shader* fromFile(const char* vertexPath, const char* fragmentPath);
	static Shader* fromSource(const char* vertexSource, const char* fragmentSource);
	static Shader* fromSource(const char* name, const char* vertexSource, const char* fragmentSource);

	unsigned int getShaderID() const { return m_ShaderID; };

private:
	unsigned int load(const char* vertexSource, const char* fragmentSource);

	unsigned int m_ShaderID;
	const char* m_Name;
	const char* m_VertPath;
	const char* m_FragPath;
	const char* m_VertSource;
	const char* m_FragSource;
};