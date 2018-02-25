#pragma once

#include <vector>
#include <bitset>
#include <array>
#include "Component.h"

namespace ecs
{
	typedef unsigned int EntityId;

	constexpr std::size_t MaxComponets  = 32;

	class Entity
	{
	public:
		Entity();

		template <typename T, typename... TArgs>
		void AddComponent(TArgs&&... arguments);

		template <typename T>
		T& GetComponent() const;

		template <typename T>
		void RemoveComponent();

		void RemoveAllComponents();

		template <class T>
		bool HasComponent() const;

	protected:
		EntityId id;

		std::vector<Component*> m_Components;
		std::bitset<MaxComponets> m_ComponentBitset;
		std::array<Component*, MaxComponets> m_ComponentArray;
	};
}