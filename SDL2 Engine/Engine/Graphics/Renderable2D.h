#pragma once

#include <vector>
#include <SDL.h>
#include <glm\glm.hpp>
#include "../Components/Texture.h"
#include "Renderer2D.h"

namespace graphics 
{
	struct VertexData
	{
		glm::vec3 vertex;
		glm::vec2 uv;
		float tid;
		unsigned int color;
	};

	class Renderable2D
	{
	public:
		Renderable2D(glm::vec3 position, glm::vec2 size, unsigned int color)
			: m_Position(position), m_Size(size), m_Color(color)
		{
		}

		virtual ~Renderable2D() { }

		virtual void submit(Renderer2D* renderer) const
		{
			renderer->submit(this);
		}

		virtual void Render(SDL_Renderer* renderer) {};

		inline const glm::vec3& getPosition() const { return m_Position; }
		inline const glm::vec2& getSize() const { return m_Size; }
		inline const unsigned int& getColor() const { return m_Color; }
		SDL_Rect texr;

	private:
	protected:
		glm::vec3 m_Position;
		glm::vec2 m_Size;
		unsigned int m_Color;
	};
}