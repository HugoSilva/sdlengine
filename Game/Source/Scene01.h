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
	Shader* shader;
	Camera* camera;
	graphics::Layer* m_BaseLayer;
};
