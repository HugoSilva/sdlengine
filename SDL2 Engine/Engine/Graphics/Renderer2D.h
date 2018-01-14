#pragma once

namespace graphics
{
	class Renderable2D;

	class Renderer2D
	{
	protected:
		Renderer2D() {};

	public:
		virtual void begin() {}

		virtual void submit(const Renderable2D* renderable) = 0;

		virtual void end() {}

		virtual void flush() = 0;
	};
}
