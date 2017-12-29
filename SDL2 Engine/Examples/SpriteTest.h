#pragma once

#include "../Engine/Scene.h"
#include "../Engine/Components/Shader.h"
#include "../Engine/Components/Texture.h"
#include "../Engine/Components/Sprite.h"
#include "../Engine/Camera.h"
#include "../Engine/Graphics/Renderer2D.h"
#include <SDL.h>

namespace examples {

	class SpriteTest : public Scene
	{

	public:
		SpriteTest(SDL_Window* win);
		~SpriteTest();

		void Update(float deltaTime);
		void Render();

	private:
		Shader* shader;
		Texture* testTexture;
		Camera* camera;
		graphics::Renderer2D* renderer;
	};
}
