#include "FileReader.h"

vup::FileReader::FileReader(std::string path)
{
  m_path = path;
  load(path);
}

vup::FileReader::~FileReader()
{
  
}

void vup::FileReader::load(std::string path)
{
  m_loaded = false;
  std::ifstream file(path);
  if (file.is_open()) {
    m_source = std::string(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
    m_loaded = true;
    m_size = m_source.size();
  }
  else {
    throw(vup::FileNotFoundException(path));
  }
}

