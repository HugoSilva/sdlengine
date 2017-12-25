#pragma once

#include "../Engine/Scene.h"
#include "../Engine/Components/Shader.h"
#include "../Engine/Components/Sprite.h"
#include "../Engine/Camera.h"

namespace examples {

	class SpriteTest : public Scene
	{
	public:
		SpriteTest();
		~SpriteTest();

		void Update(float deltaTime);
		void Render();

	private:
		Shader * shader;
		Sprite* testSprite;
		Camera* camera;
	};
}
