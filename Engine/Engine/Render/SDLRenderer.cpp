#include "SDLRenderer.hpp"

namespace graphics
{
	SDLRenderer::SDLRenderer(SDL_Window* win)
	{
		m_Renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	}

	void SDLRenderer::begin()
	{
		SDL_SetRenderDrawColor(m_Renderer, 255, 0, 255, 255);
		SDL_RenderClear(m_Renderer);
	}

	void SDLRenderer::submit(const Renderable2D* renderable)
	{
		const unsigned int& color = renderable->getColor();

		SDL_Rect texr = renderable->texr;

		SDL_SetRenderDrawColor(m_Renderer, color, color >> 8, color >> 16, color >> 24);
		SDL_RenderFillRect(m_Renderer, &texr);
	}

	void SDLRenderer::end()
	{
	}

	void SDLRenderer::flush()
	{
		SDL_RenderPresent(m_Renderer);
	}
}
