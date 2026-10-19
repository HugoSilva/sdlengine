#include "Logger.hpp"

#include <deque>
#include <mutex>
#include <sstream>

std::string Logger::s_warning_prefix{"Warning: "};
std::string Logger::s_error_prefix{"Error:   "};

namespace {
// Systems can log from worker threads, so the history is guarded.
std::mutex s_mutex{};
std::deque<Logger::Entry> s_entries{};
uint64_t s_revision{0};
}  // namespace

void Logger::Append(Level level, const std::string& message) {
  std::lock_guard<std::mutex> lock{s_mutex};

  s_entries.push_back(Entry{level, message});
  while (s_entries.size() > kMaxEntries) {
    s_entries.pop_front();
  }

  ++s_revision;
}

void Logger::Log(std::string msg) {
  std::cout << msg << std::endl;
  Append(Level::Log, msg);
}

void Logger::Warning(std::string msg) {
  std::cerr << s_warning_prefix << msg << std::endl;
  Append(Level::Warning, msg);
}

void Logger::Error(std::string msg) {
  std::cerr << s_error_prefix << msg << std::endl;
  Append(Level::Error, msg);
}

std::vector<Logger::Entry> Logger::GetEntries() {
  std::lock_guard<std::mutex> lock{s_mutex};
  return std::vector<Entry>{s_entries.begin(), s_entries.end()};
}

uint64_t Logger::GetRevision() {
  std::lock_guard<std::mutex> lock{s_mutex};
  return s_revision;
}

void Logger::Clear() {
  std::lock_guard<std::mutex> lock{s_mutex};
  s_entries.clear();
  ++s_revision;
}
