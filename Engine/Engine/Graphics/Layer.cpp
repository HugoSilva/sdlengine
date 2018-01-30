#include "layer.h"

namespace graphics {

	Layer::Layer(Renderer2D* renderer, Shader* shader)
		: m_Renderer(renderer), m_Shader(shader)
	{
		m_Shader->Enable();

		GLint texIDs[] =
		{
			0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
			10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
			20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
			30, 31
		};

		m_Shader->setUniform1iv("textures", texIDs, 32);
		m_Shader->Disable();
	}

	Layer::~Layer()
	{
		delete m_Shader;
		delete m_Renderer;

		for (unsigned int i = 0; i < m_Renderables.size(); i++)
			delete m_Renderables[i];
	}

	void Layer::add(Renderable2D* renderable)
	{
		m_Renderables.push_back(renderable);
	}

	void Layer::render()
	{
		m_Shader->Enable();
		m_Renderer->begin();

		for (const Renderable2D* renderable : m_Renderables)
			renderable->submit(m_Renderer);

		m_Renderer->end();
		m_Renderer->flush();
	}
}