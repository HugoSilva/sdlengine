#pragma once

#include <Core.h>
#include <SDL.h>

class SpriteTest : public Scene
{

public:
	SpriteTest(SDL_Window* win);
	~SpriteTest();

	void Update(float deltaTime);
	void Render();

private:
	graphics::Layer* m_Layer;
	Label* m_Fps;
	Sprite* m_GroundSprite;
	Sprite* m_PlayerSprite;
	Shader* m_Shader;

	Camera* m_Camera;
};
