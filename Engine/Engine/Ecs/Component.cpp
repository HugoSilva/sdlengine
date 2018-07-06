#include "Component.hpp"

namespace ecs
{
	static unsigned int componentID = 0;

	unsigned int ComponentBase::nextID()
	{
		return componentID++;
	}
}