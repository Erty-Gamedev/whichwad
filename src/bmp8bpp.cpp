#include <iostream>
#include <filesystem>
#include "bmp8bpp.h"


BMP::BMP8Bpp::BMP8Bpp(const int width, const int height)
{
	const int size = width * height;
	m_header = {
		.filesize = static_cast<uint32_t>(c_BMPSIZEHEADER + size),
		.dataOffset = c_BMPSIZEHEADER
	};
	m_infoHeader = {
		static_cast<uint32_t>(40), static_cast<uint32_t>(width), static_cast<uint32_t>(height),
		static_cast<uint16_t>(1), static_cast<uint16_t>(8), static_cast<uint32_t>(0), static_cast<uint32_t>(size),
		3780, // Horizontal pixels per meter
		3780, // Vertical pixels per meter
		256,  // Number of colours used (always 256 here)
		256   // Number of important colours (always 256 here)
	};
}
BMP::BMP8Bpp::~BMP8Bpp()
{
	if (m_file.is_open())
		m_file.close();
}

bool BMP::BMP8Bpp::save(const std::filesystem::path& filepath)
{
	m_file.open(filepath, std::ios::binary);
	if (!m_file.is_open() || !m_file.good())
	{
		m_file.close();
		std::cout << "Could not create file " << filepath.string() << std::endl;
		exit(EXIT_FAILURE);
	}

	m_file.write(reinterpret_cast<char*>(&m_header), sizeof(m_header));
	m_file.write(reinterpret_cast<char*>(&m_infoHeader), sizeof(m_infoHeader));
	m_file.write(reinterpret_cast<char*>(&m_palette[0]), c_BMPPALETTESIZE * 4);
	m_file.write(reinterpret_cast<char*>(&m_data[0]), static_cast<std::streamsize>(m_infoHeader.width) * m_infoHeader.height);

	m_file.close();

	return m_file.good();
}
