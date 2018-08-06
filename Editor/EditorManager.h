#pragma once

#include "Core.h"

class EditorManager
{
public:
	EditorManager() {}
	~EditorManager() {}

	static bool* getShowSceneWidget();
	static bool* getShowComponentWidget();

private:
	static bool m_ShowSceneWidget;
	static bool m_ShowComponentWidget;
	static ecs::EntityHandle m_SelectedEntity;
};

