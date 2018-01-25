#pragma once

#include "../Engine/Scene.h"
#include "../Engine/Components/Shader.h"
#include "../Engine/Components/Model.h"
#include "../Engine/Camera.h"

namespace examples {

	class ModelTest : public Scene
	{
	public:
		ModelTest();
		~ModelTest();

		void Update(float deltaTime);
		void Render();

	private:
		Shader * shader;
		Model* testModel;
		Camera* camera;
	};
}