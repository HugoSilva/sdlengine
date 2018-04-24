#pragma once

namespace physics 
{
	class Updatable
	{
	public:
		Updatable() { }

		virtual ~Updatable() { }

		virtual void Update(float deltaTime) { }
	};
}