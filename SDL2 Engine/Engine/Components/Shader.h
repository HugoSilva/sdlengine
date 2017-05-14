#ifndef SHADER
#define SHADER

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "..\..\libs\Glew\include\glew.h"

class Shader
{
public:
	// The program ID
	GLuint Program;
	// Constructor reads and builds the shader
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	// Use the program
	void Use();
};

#endif
