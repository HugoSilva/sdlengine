#ifndef TEXTURE
#define TEXTURE

#include <assimp/Importer.hpp>
#include "SOIL.h"

#include <iostream>
#include <string>
#include <GL/glew.h>
#pragma comment(lib, "opengl32.lib")

using namespace std;

class Texture
{
public:
	Texture();
	Texture(int _id);
	Texture(const char *path, string directory, string type);

	int GetID();
	string GetType();
	aiString GetPath();
	int GetWidth();
	int GetHeight();

private:
	GLuint id;
	string type;
	aiString path;

	int width;
	int height;
};

#endif