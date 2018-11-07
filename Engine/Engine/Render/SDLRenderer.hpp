#pragma once

#include <SDL.h>
#include <vector>
#include "Renderable2d.hpp"
#include "Renderer2D.hpp"

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
		SDL_Renderer* m_Renderer = NULL;
	};
}
