#pragma once

#include "../Components/Sprite.hpp"

namespace physics 
{
	class Updatable
	{
	public:
		Updatable() { }

		virtual ~Updatable() { }

		virtual void Update() { }
	};
}