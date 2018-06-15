#pragma once

#include <imgui.h>
#include "../Graphics/Renderable2D.h"

class ImguiMenuBar : public graphics::Renderable2D
{
public:
	ImguiMenuBar();
	void submit(graphics::Renderer2D* renderer) const override;
};