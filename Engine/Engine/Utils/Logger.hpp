#pragma once

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class Logger {
 public:
  enum class Level {
    Log,
    Warning,
    Error,
  };

  struct Entry {
    Level level;
    std::string message;
  };

  static void Log(std::string msg);
  static void Warning(std::string msg);
  static void Error(std::string msg);

  // The editor's console panel reads history through these. Messages are kept
  // in a fixed-size history, so a long session cannot grow without bound.
  static std::vector<Entry> GetEntries();

  // Bumped whenever the history changes. The panel only re-reads when this
  // moves, which keeps it from copying the whole history every frame.
  static uint64_t GetRevision();

  static void Clear();

 private:
  static std::string s_ErrorPrefix;
  static std::string s_WarningPrefix;

  static void Append(Level level, const std::string& message);

  static constexpr size_t s_MaxEntries{512};
};
