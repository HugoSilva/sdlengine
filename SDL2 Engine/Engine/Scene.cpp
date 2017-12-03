#include "Scene.h"

Scene::Scene()
{
	shader = new Shader("Assets/Shaders/Default.vs", "Assets/Shaders/Default.frag");
	//Sprite testSprite = Sprite("Assets/Textures/texture.png", Vector3(Engine::SCREEN_WIDTH / 2, Engine::SCREEN_HEIGHT / 2, 0));
	testModel = new Model("Assets/Models/nanosuit.obj");
	camera = new Camera(glm::vec3(0.0f, 0.0f, 4.0f));
}

Scene::~Scene()
{
}

void Scene::Update(float deltaTime)
{
	camera->Update(deltaTime);
	//testSprite.Update();
	testModel->Update();
}

void Scene::Render()
{
	shader->Use();
	camera->Render(*shader);
	//testSprite.Render();
	testModel->Render(*shader);
}