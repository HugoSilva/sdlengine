#ifndef SCENE
#define SCENE

#include "Components/Shader.h"
//#include "Components/Sprite.h"
#include "Components/Model.h"
#include "Camera.h"

class Scene
{
public:
	Scene();
	~Scene();

	void Update(float deltaTime);
	void Render();

private:
	Shader* shader;
	//Sprite testsprite;
	Model* testModel;
	Camera* camera;
};

#endif