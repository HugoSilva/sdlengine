#pragma once

#include <vector>

class Shader; //forward declaration for Shader

namespace graphics
{
	class Renderable2D; //forward declaration for graphics::Renderable2D
	class Renderer2D; //forward declaration for graphics::Renderer2D

	class Layer
	{
	public:
		Layer(Renderer2D* renderer, Shader* shader);
		virtual ~Layer();
		virtual void add(Renderable2D* renderable);
		virtual void remove(Renderable2D* renderable);
		virtual void render();

		inline const std::vector<Renderable2D*>& getRenderables() const { return m_Renderables; }

	protected:
		Renderer2D* m_Renderer;
		std::vector<Renderable2D*> m_Renderables;
		Shader* m_Shader;
	};
}