#include "System.hpp"

namespace ecs
{
	bool SystemBase::isValid()
	{
		for (unsigned int i = 0; i < componentFlags.size(); i++)
		{
			if ((componentFlags[i] & SystemBase::FLAG_OPTIONAL) == 0)
			{
				return true;
			}
		}
		return false;
	}

	bool SystemList::removeSystem(SystemBase& system)
	{
		for (unsigned int i = 0; i < systems.size(); i++)
		{
			if (&system == systems[i])
			{
				systems.erase(systems.begin() + i);
				return true;
			}
		}
		return false;
	}
}