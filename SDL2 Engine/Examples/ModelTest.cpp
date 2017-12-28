#include "ModelTest.h"

namespace examples {

	ModelTest::ModelTest()
	{
		shader = new Shader("Assets/Shaders/Default.vs", "Assets/Shaders/Default.frag");
		testModel = new Model("Assets/Models/nanosuit.obj");
		camera = new Camera(glm::vec3(0.0f, 0.0f, 4.0f));
	}

	ModelTest::~ModelTest()
	{
	}

	void ModelTest::Update(float deltaTime)
	{
		camera->Update(deltaTime);
		testModel->Update();
	}

	void ModelTest::Render()
	{
		shader->Use();
		camera->Render(*shader);
		testModel->Render(*shader);
	}
}