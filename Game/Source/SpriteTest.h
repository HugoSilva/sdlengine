#pragma once

#include "../Engine/Scene.h"
#include "../Engine/Components/Shader.h"
#include "../Engine/Components/Texture.h"
#include "../Engine/Components/Sprite.h"
#include "../Engine/Camera.h"
#include "../Engine/Graphics/SDLRenderer.h"
#include "../Engine/Graphics/OpenGLRenderer.h"
#include "../Engine/Audio/Sound.h"
#include "../Engine/Audio/SoundManager.h"
#include "../Engine/Graphics/FontManager.h"
#include "../Engine/Graphics/Font.h"
#include "../Engine/Components/Label.h"
#include "../Engine/Components/TextureManager.h"
#include "../Engine/Graphics/Layer.h"
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
		Camera* camera;
		graphics::Layer* m_BaseLayer;
	};
}