#pragma once

#include <glm\glm.hpp>
#include "Font.h"
#include "../Ecs/Components/Transform.hpp"

namespace graphics
{
	class Renderable2D;

	class Renderer2D
	{
	protected:
		Renderer2D() {};

	public:
		virtual ~Renderer2D() { }

		virtual void begin() {}

		virtual void submit(TransformComponent& position, const Renderable2D* renderable) = 0;

		virtual void end() {}

		virtual void flush() = 0;
	};
}
