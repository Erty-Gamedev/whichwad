#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string_view>


#include <iostream>


class BaseReader
{
public:
	std::filesystem::path m_filepath;

	BaseReader(const std::filesystem::path& filepath) : m_filepath(filepath) {};
	~BaseReader()
	{ if (m_file.is_open()) m_file.close(); /*std::cout << "Reader for " << m_filepath.filename().string() << " closed\n";*/ }

	virtual bool contains(const std::string_view& textureName) const = 0;
	virtual bool extract(const std::string& textureName, const std::filesystem::path& outPath) = 0;
protected:
	std::ifstream m_file;

	virtual void parse() = 0;
};
