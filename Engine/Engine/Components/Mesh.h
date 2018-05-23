#ifndef MESH
#define MESH

#include "Shader.h"

#ifdef EMSCRIPTEN
	#include <SDL_opengles2.h>
#else
	#include <GL/glew.h>
#endif // EMSCRIPTEN
//#pragma comment(lib, "opengl32.lib")
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "Texture.h"

using namespace std;

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	//glm::vec4 Colors;
	glm::vec2 TexCoords;
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