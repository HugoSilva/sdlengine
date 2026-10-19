#pragma once

#include <fstream>
#include <iostream>
#include <sstream>

#include "Logger.hpp"

class File {
 public:
  static std::string Read(const char* filepath) {
    std::ifstream inputStream;

    inputStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
      inputStream.open(filepath);

      std::stringstream fileStream;

      fileStream << inputStream.rdbuf();

      inputStream.close();

      return fileStream.str();
    } catch (std::ifstream::failure e) {
      std::string failedFile(filepath);
      Logger::Error("Failed to read file: " + failedFile);
      return "";
    }
  }

  static bool Exists(const char* filepath) {
    struct stat buffer;
    return (stat(filepath, &buffer) == 0);
  }

  static bool Write(const char* filepath, const char* content) {
    std::ofstream file(filepath);

    if (file.is_open()) {
      file << content;
      file.close();
      return true;
    } else {
      return false;
    }
  }
};