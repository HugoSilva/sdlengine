#pragma once

#include "Component.hpp"
#include "System.hpp"
#include <map>

namespace ecs
{
	class ECS
	{
	public:
		ECS() { }
		~ECS();

		EntityHandle makeEntity(ComponentBase** components, const unsigned int* componentIDs, size_t numComponents);
		void removeEntity(EntityHandle handle);

		template<class... TArgs>
		EntityHandle makeEntity(TArgs&&... arguments)
		{
			ComponentBase* components[] = { (&arguments)... };
			unsigned int componentIDs[] = { (std::remove_reference_t<TArgs>::ID)... };
			return makeEntity(components, componentIDs, sizeof...(TArgs));
		}

		template<class Component>
		inline void addComponent(EntityHandle entity, Component* component)
		{
			addComponentInternal(entity, handleToEntity(entity), Component::ID, component);
		}

		template<class Component>
		inline bool removeComponent(EntityHandle entity)
		{
			return removeComponentInternal(entity, Component::ID);
		}

		template<class Component>
		inline Component* getComponent(EntityHandle entity)
		{
			return (Component*)getComponentInternal(handleToEntity(entity), components[Component::ID], Component::ID);
		}

		void updateSystems(SystemList& systems, float delta);

	private:
		std::map<unsigned int, std::vector<char>> components;
		std::vector<std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>* > entities;

		inline std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>* handleToRawType(EntityHandle handle)
		{
			return (std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>*)handle;
		}

		inline unsigned int handleToEntityIndex(EntityHandle handle)
		{
			return handleToRawType(handle)->first;
		}

		inline std::vector<std::pair<unsigned int, unsigned int> >& handleToEntity(EntityHandle handle)
		{
			return handleToRawType(handle)->second;
		}

		void deleteComponent(unsigned int componentID, unsigned int index);
		bool removeComponentInternal(EntityHandle handle, unsigned int componentID);
		void addComponentInternal(EntityHandle handle, std::vector<std::pair<unsigned int, unsigned int> >& entity, unsigned int componentID, ComponentBase* component);
		ComponentBase* getComponentInternal(std::vector<std::pair<unsigned int, unsigned int>>& entityComponents, std::vector<char>& array, unsigned int componentID);
		
		void updateSystemWithMultipleComponents(unsigned int index, SystemList& systems, float delta, const std::vector<unsigned int>& componentTypes,
			std::vector<ComponentBase*>& componentParam, std::vector<std::vector<char>*>& componentArrays);
		
		unsigned int findLeastCommonComponent(const std::vector<unsigned int>& componentTypes, const std::vector<unsigned int>& componentFlags);
	};
}