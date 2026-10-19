#include "ThreadManager.hpp"

#include "../Input/InputManager.hpp"
#include "../Physics/PhysicsManager.h"

unsigned int ThreadManager::s_AvailableThreads;
std::vector<std::thread*> ThreadManager::s_Threads;
std::vector<std::mutex*> ThreadManager::s_Mutexes;

void ThreadManager::Init() { s_AvailableThreads = std::thread::hardware_concurrency(); }

void ThreadManager::Start() {
  s_Threads.push_back(new std::thread(PhysicsManager::UpdateObjects));

  for (unsigned int num = 0; num < s_AvailableThreads; num++) {
    // s_Threads.push_back(&threadIO);
  }
}

void ThreadManager::Clean() {
  for (unsigned int i = 0; i < s_AvailableThreads; i++) {
    s_Threads[i]->join();
    // delete s_Threads[i];
  }
}