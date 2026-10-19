#include "ThreadManager.hpp"

#include "../Input/InputManager.hpp"
#include "../Physics/PhysicsManager.h"

unsigned int ThreadManager::s_available_threads;
std::vector<std::thread*> ThreadManager::s_threads;
std::vector<std::mutex*> ThreadManager::s_mutexes;

void ThreadManager::Init() { s_available_threads = std::thread::hardware_concurrency(); }

void ThreadManager::Start() {
  s_threads.push_back(new std::thread(PhysicsManager::UpdateObjects));

  for (unsigned int num = 0; num < s_available_threads; num++) {
    // s_Threads.push_back(&threadIO);
  }
}

void ThreadManager::Clean() {
  for (unsigned int i = 0; i < s_available_threads; i++) {
    s_threads[i]->join();
    // delete s_Threads[i];
  }
}