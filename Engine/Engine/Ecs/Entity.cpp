#include "Entity.h"

namespace ecs
{
	Entity::Entity()
	{
	}

	template <typename T, typename... TArgs>
	void Entity::AddComponent(TArgs&&... arguments)
	{
		T* component(new T(std::forward<TArgs>(arguments)));
		component->entity = this;
		m_Components.push_back(component);

		m_ComponentArray[GetComponentTypeID<T>()] = component;
		m_ComponentBitset[GetComponentTypeID<T>()] = true;

		component->
	}

	template <typename T>
	T& Entity::GetComponent() const
	{
		auto ptr(m_ComponentArray[GetComponentTypeID<T>()]);
		return *static_cast<T*>(ptr);
	}

	template <typename T>
	void Entity::RemoveComponent()
	{

	}

	void Entity::RemoveAllComponents()
	{

	}

	template <typename T>
	bool Entity::HasComponent() const
	{
		return m_ComponentBitset[GetComponentTypeID<T>()];
	}
}