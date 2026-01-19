#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <string_view>


#include <iostream>


class BaseReader
{
public:
	std::filesystem::path m_filepath;

	explicit BaseReader(const std::filesystem::path& filepath) : m_filepath(filepath) {};

	virtual ~BaseReader() { if (m_file.is_open()) m_file.close();}

	virtual bool contains(const std::string_view& textureName) const = 0;
	virtual bool extract(const std::string& textureName, const std::filesystem::path& outPath) = 0;
protected:
	std::ifstream m_file;

	virtual void parse() = 0;
};
