#pragma once

#include <SDL.h>
#include <vector>
#include "Renderable2d.h"
#include "Renderer2D.h"

namespace graphics
{
	class SDLRenderer : public Renderer2D
	{
	public:
		SDLRenderer(SDL_Window* win);

		void begin();
		void submit(const Renderable2D* renderable);
		void end();
		void flush();

	private:
		SDL_Renderer * m_Renderer = NULL;
		std::vector<Renderable2D*> m_Renderables;
	};
}
