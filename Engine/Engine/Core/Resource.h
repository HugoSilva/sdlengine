#pragma once

#include <string>

#include "../Utils/Logger.h"

class Resource
{
public:
	Resource(const std::string& name, const std::string& filename);
	~Resource();

	inline const std::string& getName() const { return m_Name; }
	inline const std::string& getFileName() const { return m_Filename; }

private:
	std::string m_Name;
	std::string m_Filename;
};