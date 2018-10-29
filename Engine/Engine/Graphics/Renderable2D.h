#pragma once

#include <vector>
#include <memory>
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
			setUVDefaults();
		}

		virtual ~Renderable2D() { }

		virtual void submit(Renderer2D* renderer) const
		{
			//renderer->submit(this);
		}

		virtual void Render(SDL_Renderer* renderer) {};

		inline const glm::vec3& getPosition() const { return m_Position; }
		inline const glm::vec2& getSize() const { return m_Size; }
		inline void setColor(unsigned int color) { m_Color = color; }
		inline const unsigned int& getColor() const { return m_Color; }
		inline const std::vector<glm::vec2>& getUV() const { return m_UV; }
		inline const unsigned int getTId() const { return m_Texture.get() == nullptr ? 0 : m_Texture.get()->getID(); }
		inline const std::string getTextureName() const { return m_Texture.get() == nullptr ? "" : m_Texture.get()->getName(); }

		SDL_Rect texr;

		template<typename Archive>
		void serialize(Archive &archive)
		{
			archive(cereal::make_nvp("Size", m_Size), cereal::make_nvp("Color", m_Color), cereal::make_nvp("Texture", m_Texture));
		}

	private:
		void setUVDefaults()
		{
			m_UV.push_back(glm::vec2(0, 1));
			m_UV.push_back(glm::vec2(0, 0));
			m_UV.push_back(glm::vec2(1, 0));
			m_UV.push_back(glm::vec2(1, 1));
		}

	protected:
		Renderable2D() { setUVDefaults(); }

		glm::vec3 m_Position;
		glm::vec2 m_Size;
		unsigned int m_Color{ 0x00 };
		std::vector<glm::vec2> m_UV;
		std::shared_ptr<Texture> m_Texture{ nullptr };
	};
}