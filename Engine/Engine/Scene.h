#pragma once

#include "Components/Shader.h"
#include "Ecs/Entity.h"

#include "rapidxml.hpp"

class Scene
{
public:
	Scene() {};

	void AddEntity(ecs::Entity* entity)
	{
		m_Entities.push_back(entity);
	}

	virtual void Update(float deltaTime) = 0;

	virtual void Render() = 0;

	inline const std::string& getName() const { return m_Name; }

	void writeXML(rapidxml::xml_document<>* doc, rapidxml::xml_node<>* parentNode)
	{
		rapidxml::xml_node<>* scene = doc->allocate_node(rapidxml::node_element, "Scene");
		doc->append_node(scene);

		for (auto item : m_Entities)
		{
			item->writeXML(doc, scene);
		}
	}

	void readXML(rapidxml::xml_document<>* doc, rapidxml::xml_node<>* node)
	{
		rapidxml::xml_node<>* root_node;
		root_node = doc->first_node("Scene");
		int size = 0;
		for (rapidxml::xml_node<>* entity_node = root_node->first_node("Entity"); entity_node; entity_node = entity_node->next_sibling())
		{
			ecs::Entity* nextNode = new ecs::Entity();
			nextNode->readXML(doc, entity_node);

			m_Entities.push_back(nextNode);
		}
	}

protected:
	std::string m_Name;
	Shader* m_Shader;

private:
	std::vector<ecs::Entity*> m_Entities;
};