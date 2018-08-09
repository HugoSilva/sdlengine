#include "EditorManager.h"

#include <glm\glm.hpp>

bool EditorManager::m_ShowSceneWidget{ true };
bool EditorManager::m_ShowComponentWidget{ true };
uint32_t EditorManager::m_SelectedEntity;

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

	struct Position
	{
		Position()
		{
		};

		Position(const glm::vec2& position, const glm::vec2& direction)
			: position(position), direction(direction)
		{
		};

		glm::vec2 position, direction;
	};

	//ecs::PositionComponent pos;
	//m_SelectedEntity = ecs::ECSManager::addEntity(pos);

	m_SelectedEntity = ecs::ECSManager::createEntity();
	ecs::ECSManager::addComponent<Position>(m_SelectedEntity);
}

void EditorManager::receive(const SelectEntity &event)
{
	event.value;
}