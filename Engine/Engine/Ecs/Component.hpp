#pragma once
#include <vector>

namespace ecs
{
	struct ComponentBase;
	typedef void* EntityHandle;
	typedef unsigned int (*ComponentCreateFunction)(std::vector<char>& memory, EntityHandle entity, ComponentBase* comp);
	typedef void (*ComponentFreeFunction)(ComponentBase* comp);
	#define NULL_ENTITY_HANDLE nullptr

	struct ComponentBase
	{
		EntityHandle entity = NULL_ENTITY_HANDLE;
		static unsigned int nextID();
	};

	template<typename T>
	struct Component : public ComponentBase
	{
		static const ComponentCreateFunction CREATE_FUNCTION;
		static const ComponentFreeFunction FREE_FUNCTION;
		static const unsigned int ID;
		static const unsigned int SIZE;
	};

	template<typename Component>
	unsigned int ECSComponentCreate(std::vector<char>& memory, EntityHandle entity, ComponentBase* comp)
	{
		unsigned int index = memory.size();
		memory.resize(index + Component::SIZE);
		Component* component = new(&memory[index])Component(*(Component*)comp);
		component->entity = entity;
		return index;
	}

	template<typename Component>
	void ECSComponentFree(ComponentBase* comp)
	{
		Component* component = (Component*)comp;
		component->~Component();
	}

	template<typename T>
	const unsigned int ComponentBase<T>::ID(ComponentBase::nextID());

	template<typename T>
	const unsigned int ComponentBase<T>::SIZE(sizeof(T));

	template<typename T>
	const ComponentCreateFunction ECSComponent<T>::CREATE_FUNCTION(ECSComponentCreate<T>);

	template<typename T>
	const ComponentFreeFunction ECSComponent<T>::FREE_FUNCTION(ECSComponentFree<T>);

	struct PositionComponent : public Component<PositionComponent>
	{
		float x;
		float y;
		float z;
	};
}