#pragma once

#include <Core.h>
#include <SDL.h>

class SetupView : public Scene
{

public:
	SetupView(SDL_Window* win);

	void Update(float deltaTime);
	void Render();

private:
	graphics::Layer* m_Layer;
	Camera* m_Camera;
};
