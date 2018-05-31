#include "ThreadManager.h"

#include <thread>

unsigned int ThreadManager::m_AvailableThreads;

void ThreadManager::Init()
{
	m_AvailableThreads = std::thread::hardware_concurrency();
}