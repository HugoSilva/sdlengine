#pragma once

#include <glm/glm.hpp>
#include "Font.hpp"
#include "../Ecs/Components/Transform.hpp"

namespace rse
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

		// A flat, untextured quad in world space. Same transform and corner
		// convention as submit(), but with no texture bound, so the shader falls
		// through to the vertex colour and `color` is used as-is. The editor's
		// grid and gizmos are built out of these.
		virtual void submitQuad(TransformComponent& transform, glm::vec2 halfExtent, unsigned int color) {}

		virtual void end() {}

		virtual void flush() = 0;

		// The colour the renderer clears the target with, for debug UIs that want
		// to expose it. Null when the backend has no such notion.
		virtual float* getClearColor() { return nullptr; }
	};
}
