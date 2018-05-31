#pragma once

#include <iostream>
#include <SDL.h>
#include "../IO/InputManager.h"
#include "../Audio/SoundManager.h"
#include "../Graphics/FontManager.h"
#include "../Components/TextureManager.h"
#include "../Physics/PhysicsManager.h"

namespace graphics
{
	class Window
	{
	public:
		Window(const char *title, int width, int height);
		~Window();

		bool Init();
		bool GetRunning();
		SDL_Window* GetWindow() { return window; };

	private:
		SDL_Event windowEvent;
		SDL_Window* window;
		SDL_GLContext context;

		const char* m_Title;
		int m_Width, m_Height;
	};
}