#ifndef MESH
#define MESH

#include "../Math/Vector3.h"
#include "../Math/Vector2.h"
#include "Shader.h"

#include "..\..\Libs\Glew\include\glew.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 TexCoords;
};

struct Texture
{
	GLuint id;
	string type;
	string path;
};

class Mesh
{
public:
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures);
	void Draw(Shader shader);

private:
	GLuint VAO, VBO, EBO;

	void setupMesh();
};

#endif