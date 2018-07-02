#include "Resource.h"

Resource::Resource(const std::string& name, const std::string& filename)
	: m_Name(name), m_Filename(filename)
{
}

Resource::~Resource()
{
}