#include "EditorManager.h"

bool EditorManager::m_ShowSceneWidget{ true };
bool EditorManager::m_ShowComponentWidget{ true };
ecs::EntityHandle EditorManager::m_SelectedEntity{ nullptr };

void EditorManager::init()
{
	EventManager::Register("CreateEntity");
	EventManager::AddListener("CreateEntity", &EditorManager::EditorManager(), &EditorManager::createNewEntity);
}

bool* EditorManager::getShowSceneWidget()
{
	return &m_ShowSceneWidget;
}

bool* EditorManager::getShowComponentWidget()
{
	return &m_ShowComponentWidget;
}

void EditorManager::createNewEntity()
{
	ecs::PositionComponent pos;
	m_SelectedEntity = ecs::ECSManager::addEntity(pos);
}