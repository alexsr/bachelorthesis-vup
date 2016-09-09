#include "FileReader.h"

vup::FileReader::FileReader(std::string path)
{
  m_path = path;
  m_source = load(path);
  m_size = m_source.size();
}

vup::FileReader::~FileReader()
{
  
}

std::string vup::FileReader::load(std::string path)
{
  m_loaded = false;
  std::ifstream file(path);
  if (file.is_open()) {
    std::string source(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
    m_loaded = true;
    return source;
  }
  else {
    throw(vup::FileNotFoundException(path));
  }
}

