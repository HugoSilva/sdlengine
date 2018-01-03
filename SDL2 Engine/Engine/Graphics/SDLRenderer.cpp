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
		m_Renderables.push_back(const_cast<Renderable2D*>(renderable));
	}

	void SDLRenderer::end()
	{
		int size = m_Renderables.size();
		for (int i = 0; i < size; i++)
		{
			glm::vec4 color = m_Renderables[i]->getColor();
			glm::vec3 position = m_Renderables[i]->getPosition();
			glm::vec2 size = m_Renderables[i]->getSize();

			SDL_Rect texr = m_Renderables[i]->texr;

			SDL_SetRenderDrawColor(m_Renderer, color.r * 255, color.g * 255, color.b * 255, color.a * 255);
			SDL_RenderFillRect(m_Renderer, &texr);
		}
	}

	void SDLRenderer::flush()
	{
		SDL_RenderPresent(m_Renderer);
	}
}
