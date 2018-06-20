#pragma once

#include <Graphics/Renderable2D.h>

class ImguiDialog : public graphics::Renderable2D
{
public:
	ImguiDialog();
	void submit(graphics::Renderer2D* renderer) const override;
};