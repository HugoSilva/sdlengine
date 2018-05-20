#ifndef MODEL
#define MODEL

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#ifdef EMSCRIPTEN
	#define GLFW_INCLUDE_ES3
	#include <GLFW/glfw3.h>
#else
	#include <GL/glew.h>
#endif // EMSCRIPTEN
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Texture.h"

using namespace std;

class Model
{
public:
	Model(GLchar *path);
	void Update();
	void Render(Shader shader);

private:
	float testRotation;
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;
	
	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);

	Mesh processMesh(aiMesh *mesh, const aiScene *scene);

	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};

#endif
