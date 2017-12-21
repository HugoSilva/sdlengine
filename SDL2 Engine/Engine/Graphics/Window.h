#pragma once

#include <iostream>
#include <SDL.h>
#include "../Scene.h"
#include "../Input.h"

namespace graphics {

	class Window
	{
	public:
		Window(const char *title, int width, int height);
		~Window();

		bool Init();
		void Run();
		bool GetRunning();

	private:
		void HandleEvents();

		SDL_Event windowEvent;
		SDL_Window * window;
		SDL_GLContext context;
		Scene* scene;
		Input* input;
		bool running;
		long now, last = 0;
		float deltaTime = 0.0;
		const char *m_Title;
		int m_Width, m_Height;
	};
}