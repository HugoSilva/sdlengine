#pragma once

#include <SDL.h>
#include <GL/glew.h>
#include <glm\glm.hpp>
#include <vector>
#include "Renderable2D.h"

namespace graphics
{
	class Renderer2D
	{
	public:
		Renderer2D(SDL_Window* win)
		{
			m_Renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
		}

		void begin()
		{
			SDL_SetRenderDrawColor(m_Renderer, 255, 0, 255, 255);
			SDL_RenderClear(m_Renderer);
		}
		
		void submit(Renderable2D* renderable)
		{
			m_Renderables.push_back(renderable);
		}

		void end() 
		{
			SDL_RenderPresent(m_Renderer);
		}

		void flush()
		{
			int size = m_Renderables.size();
			for (int i = 0; i < size; i++)
			{
				m_Renderables[i]->Render(m_Renderer);
			}
		}

	protected:
		SDL_Renderer* m_Renderer = NULL;
		std::vector<Renderable2D*> m_Renderables;
	};
}
