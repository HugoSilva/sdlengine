#pragma once

#include "Core.h"

class EditorManager
{
public:
	EditorManager() {}
	~EditorManager() {}

	static void init();
	static bool* getShowSceneWidget();
	static bool* getShowComponentWidget();

private:
	void createNewEntity();
	void selectEntity(int EntityId);

	static bool m_ShowSceneWidget;
	static bool m_ShowComponentWidget;
	static ecs::EntityHandle m_SelectedEntity;
};

