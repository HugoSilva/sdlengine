#pragma once

#include "Component.hpp"

namespace ecs
{
	class SystemBase
	{
	public:
		SystemBase(const std::vector<unsigned int>& componentTypesIn) : componentTypes(componentTypesIn) {}
		virtual void updateComponents(float delta, ComponentBase** components) {}
		const std::vector<unsigned int>& getComponentTypes()
		{
			return componentTypes;
		}
	private:
		std::vector<unsigned int> componentTypes;
	};
}