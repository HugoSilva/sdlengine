#include "Component.hpp"

namespace ecs
{
	std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, size_t>>* ComponentBase::componentTypes;

	unsigned int ComponentBase::registerComponentType(ComponentCreateFunction createfn, ComponentFreeFunction freefn, unsigned int size)
	{
		if (componentTypes == nullptr)
		{
			componentTypes = new std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, size_t> >();
		}

		unsigned int componentID = componentTypes->size();
		componentTypes->push_back(std::tuple<ComponentCreateFunction, ComponentFreeFunction, size_t>(createfn, freefn, size));

		return componentID;
	}
}
