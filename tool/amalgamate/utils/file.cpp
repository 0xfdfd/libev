#include <iostream>
#include <fstream>
#include <sstream>
#include "file.hpp"

std::string am::read_file(const std::string& path)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		std::string msg = "Could not open file `" + path + "`";
		throw std::runtime_error(msg);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	return buffer.str();
}

void am::write_file(const std::string& path, const std::string& content)
{
	std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!file.is_open())
	{
		std::string msg = "Could not open file `" + path + "`";
		throw std::runtime_error(msg);
	}

	file << content;

	if (!file)
	{
		throw std::runtime_error("Failed to write to file");
	}
}

std::string am::file_name(const std::string& path)
{
	const char* file = path.c_str();
	const char* pos = file;

	for (; *file; ++file)
	{
		if (*file == '\\' || *file == '/')
		{
			pos = file + 1;
		}
	}
	return pos;
}
