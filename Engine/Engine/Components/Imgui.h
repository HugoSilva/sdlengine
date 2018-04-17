#pragma once

#include <imgui.h>
#include "../Graphics/Renderable2D.h"

class Imgui : public graphics::Renderable2D
{
public:
	Imgui();
	void submit(graphics::Renderer2D* renderer) const override;
};