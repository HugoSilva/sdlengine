#pragma once

#include <SDL.h>
#include <GL/glew.h>
#include <glm\glm.hpp>
#include <vector>
#include "Components\Sprite.h"

namespace Engine {

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
		
		void submit(Sprite* sprite)
		{
			m_Sprites.push_back(sprite);
		}

		void end() 
		{
			SDL_RenderPresent(m_Renderer);
		}

		void flush()
		{
			int size = m_Sprites.size();
			for (int i = 0; i < size; i++)
			{
				m_Sprites[i]->Render(m_Renderer);
			}
		}

	protected:
		SDL_Renderer* m_Renderer = NULL;
		std::vector<Sprite*> m_Sprites;
	};
}
