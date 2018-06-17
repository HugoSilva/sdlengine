#pragma once

#include <fstream>
#include <sstream>
#include <iostream>
#include "Logger.h"

class File
{
public:
	static std::string read(const char* filepath)
	{
		std::ifstream inputStream;

		inputStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			inputStream.open(filepath);

			std::stringstream fileStream;

			fileStream << inputStream.rdbuf();

			inputStream.close();

			return fileStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::string failedFile(filepath);
			Logger::error("Failed to read file: " + failedFile);
			return "";
		}
	}

	static bool exists(const char* filepath)
	{
		struct stat buffer;
		return (stat(filepath, &buffer) == 0);
	}

	static bool write(const char* filepath, const char* content)
	{
		std::ofstream file(filepath);

		if (file.is_open())
		{
			file << content;
			file.close();
			return true;
		}
		else
		{
			return false;
		}
	}
};