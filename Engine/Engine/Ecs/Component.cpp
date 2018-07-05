#include "Component.hpp"

namespace ecs
{
	static unsigned int componentID = 0;

	static unsigned int ComponentBase::nextID()
	{
		return componentID++;
	}
}