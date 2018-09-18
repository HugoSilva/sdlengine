#include "EditorManager.h"

bool EditorManager::m_ShowSceneWidget{ true };
bool EditorManager::m_ShowComponentWidget{ true };
uint32_t EditorManager::m_SelectedEntity{ INVALID_ENTITY_ID };

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
	m_SelectedEntity = ecs::ECSManager::createEntity();
	ecs::ECSManager::addComponent<BasicComponent>(m_SelectedEntity);
	ecs::ECSManager::addComponent<TransformComponent>(m_SelectedEntity);
    ecs::ECSManager::addComponent<SpriteComponent>(m_SelectedEntity, new Sprite(glm::vec3(0, 0, 0), glm::vec2(40, 40), TextureManager::get("test00")));
}

void EditorManager::receive(const SelectEntity &event)
{
	m_SelectedEntity = event.value;
}