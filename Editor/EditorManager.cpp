#include "EditorManager.h"

bool EditorManager::m_ShowSceneWidget{ true };
bool EditorManager::m_ShowComponentWidget{ true };
ecs::EntityHandle EditorManager::m_SelectedEntity{ nullptr };

void EditorManager::init()
{
	EventManager::Register("CreateEntity");
	EventManager::AddListener("CreateEntity", &EditorManager::EditorManager(), &EditorManager::createNewEntity);

	EventManager::Register("SelectEntity");
	EventManager::AddListener("SelectEntity", &EditorManager::EditorManager(), &EditorManager::selectEntity);
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

void EditorManager::selectEntity(int EntityId)
{
	ecs::PositionComponent pos;
	m_SelectedEntity = ecs::ECSManager::addEntity(pos);
}