#pragma once

#include "Component.hpp"

namespace ecs
{
	class SystemBase
	{
	public:
		enum
		{
			FLAG_OPTIONAL = 1,
		};
		
		SystemBase() {}

		virtual void updateComponents(float delta, ComponentBase** components) {}

		const std::vector<unsigned int>& getComponentTypes() const
		{
			return componentTypes;
		}
		
		const std::vector<unsigned int>& getComponentFlags() const
		{
			return componentFlags;
		}
		
		bool isValid() const;

	protected:
		void addComponentType(unsigned int componentType, unsigned int componentFlag = 0)
		{
			componentTypes.push_back(componentType);
			componentFlags.push_back(componentFlag);
		}

	private:
		std::vector<unsigned int> componentTypes;
		std::vector<unsigned int> componentFlags;
	};

	class SystemList
	{
	public:
		inline bool addSystem(SystemBase& system)
		{
			if (!system.isValid())
			{
				return false;
			}
			systems.push_back(&system);
			return true;
		}

		inline size_t size() const
		{
			return systems.size();
		}
		
		inline SystemBase* operator[](unsigned int index)
		{
			return systems[index];
		}
		
		bool removeSystem(SystemBase& system);

	private:
		std::vector<SystemBase*> systems;
	};
}