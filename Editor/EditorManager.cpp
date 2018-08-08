#include "EditorManager.h"

bool EditorManager::m_ShowSceneWidget{ true };
bool EditorManager::m_ShowComponentWidget{ true };
ecs::EntityHandle EditorManager::m_SelectedEntity{ nullptr };

void EditorManager::init()
{
	EventManager::getEventDispatcher()->sink<CreateEntity>().connect(&EditorManager::EditorManager());
	EventManager::getEventDispatcher()->sink<SelectEntity>().connect(&EditorManager::EditorManager());
}

bool* EditorManager::getShowSceneWidget()
{
	return &m_ShowSceneWidget;
}

bool* EditorManager::getShowComponentWidget()
{
	return &m_ShowComponentWidget;
}

void EditorManager::receive(const CreateEntity &event)
{
	ecs::PositionComponent pos;
	m_SelectedEntity = ecs::ECSManager::addEntity(pos);
}

void EditorManager::receive(const SelectEntity &event)
{
	event.value;
}