#ifndef MODEL
#define MODEL

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include "..\Math\Vector2.h"
#include "Shader.h"
#include "..\..\Libs\Assimp\include\Importer.hpp"
#include "..\..\Libs\Assimp\include\scene.h"
#include "..\..\Libs\Assimp\include\postprocess.h"

#include "Mesh.h"

using namespace std;

GLint TextureFromFile(const char *path, string directory);

class Model
{
public:
	Model(char *path);
	void Draw(Shader shader);
    
private:
    vector<Mesh> meshes;
    string directory;
    vector<Texture> textures_loaded;

	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};

#endif