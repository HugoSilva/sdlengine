#pragma once

class BaseSystem
{
public:
	BaseSystem(graphics::Renderer2D* renderer) : m_renderer(renderer) {}
	virtual void update(const float alpha) = 0;

	graphics::Renderer2D* m_renderer;
};