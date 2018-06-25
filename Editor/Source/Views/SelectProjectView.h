#pragma once

#include <Core.h>
#include <SDL.h>

class SelectProjectView : public Scene
{

public:
	SelectProjectView(std::string name, SDL_Window* win);

	void Update(float deltaTime);
	void Render();

	void NewProject();

private:

	graphics::Layer* m_Layer;
	Shader* m_Shader;
	Camera* m_Camera;
};
