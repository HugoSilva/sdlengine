#pragma once

#include "ECS.hpp"

namespace ecs
{
	class ECSManager
	{
	public:
		ECSManager() { }
		~ECSManager();

		static void init();
		static void addSystemList();
		static void update(float delta);
		static void clean();

	private:
		static std::vector<SystemList*> m_SystemLists;
		static ECS m_ecs;
	};
}