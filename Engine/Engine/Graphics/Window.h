#pragma once

#include <iostream>
#include <SDL.h>
#include "../IO/InputManager.h"
#include "../Audio/SoundManager.h"
#include "../Graphics/FontManager.h"
#include "../Components/TextureManager.h"

namespace graphics
{
	class Window
	{
	public:
		Window(const char *title, int width, int height);
		~Window();

		bool Init();
		void Run();
		bool GetRunning();
		SDL_Window* GetWindow() { return window; };

	private:
		void HandleEvents();

		SDL_Event windowEvent;
		SDL_Window* window;
		SDL_GLContext context;
		IO::InputManager* m_Input;
		bool running;

		const char *m_Title;
		int m_Width, m_Height;
	};
}