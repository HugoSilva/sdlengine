#pragma once

#include "..\Components\Sprite.h"

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