#pragma once

class ThreadManager
{
public:
	static void Init();

private:
	ThreadManager() { }

	static unsigned int m_AvailableThreads;
};