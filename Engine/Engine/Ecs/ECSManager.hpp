#pragma once

#include "ECS.hpp"
#include "System.hpp"
#include <vector>

namespace ecs
{
	class ECSManager
	{
	public:
		ECSManager() { }

		static void init();
		static void addSystemList();
		static void update(float delta);
		static void clean();

	private:
		static std::vector<SystemList*> m_SystemLists;
		static ECS m_ecs;
	};
}