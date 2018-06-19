#pragma once

#include <Graphics/Renderable2D.h>

class ImguiCreate : public graphics::Renderable2D
{
public:
	ImguiCreate();
	void submit(graphics::Renderer2D* renderer) const override;
};