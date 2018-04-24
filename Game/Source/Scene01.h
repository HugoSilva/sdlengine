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
	Sprite* m_Sprite;
	Shader* m_Shader;
	Box2D* m_TestBox2d;

	Camera* m_Camera;
};
