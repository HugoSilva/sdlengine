#pragma once

#include <Core.h>
#include <SDL.h>
#include "../Systems/RenderSystem.hpp"

class SetupView : public Scene
{

public:
	SetupView(std::string name, SDL_Window* win);

	void Update(float deltaTime);
	void Render();

private:
	rse::Layer* m_Layer;
	rse::Camera* m_Camera;
	RenderSystem* m_render;
};
