#include "SDLRenderer.h"

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
		const glm::vec4& color = renderable->getColor();

		SDL_Rect texr = renderable->texr;

		SDL_SetRenderDrawColor(m_Renderer, color.r * 255, color.g * 255, color.b * 255, color.a * 255);
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
