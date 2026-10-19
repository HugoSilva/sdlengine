#pragma once

#include <fstream>
#include <iostream>
#include <sstream>

#include "Logger.hpp"

class File {
 public:
  static std::string Read(const char* filepath) {
    std::ifstream input_stream;

    input_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
      input_stream.open(filepath);

      std::stringstream file_stream;

      file_stream << input_stream.rdbuf();

      input_stream.close();

      return file_stream.str();
    } catch (std::ifstream::failure e) {
      std::string failed_file(filepath);
      Logger::Error("Failed to read file: " + failed_file);
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