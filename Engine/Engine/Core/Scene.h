#pragma once

#include "../Components/Shader.h"

#include "rapidxml.hpp"

class Scene
{
public:
	Scene() {};

	virtual void Update(float deltaTime) = 0;

	virtual void Render() = 0;

	inline const std::string& getName() const { return m_Name; }

	void writeXML(rapidxml::xml_document<>* doc, rapidxml::xml_node<>* parentNode)
	{
	}

	void readXML(rapidxml::xml_document<>* doc, rapidxml::xml_node<>* node)
	{
	}

protected:
	std::string m_Name;
	Shader* m_Shader;
};