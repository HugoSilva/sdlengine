#pragma once

#include "Renderer2D.h"
#include "Renderable2D.h"
#include "../Components/Shader.h"

namespace graphics
{
	class Layer
	{
	public:
		Layer(Renderer2D* renderer, Shader* shader);
		virtual ~Layer();
		virtual void add(Renderable2D* renderable);
		virtual void render();

		inline const std::vector<Renderable2D*>& getRenderables() const { return m_Renderables; }

	protected:
		Renderer2D* m_Renderer;
		std::vector<Renderable2D*> m_Renderables;
		Shader* m_Shader;
	};
}