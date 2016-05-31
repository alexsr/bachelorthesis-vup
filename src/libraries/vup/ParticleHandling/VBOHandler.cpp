#include "VBOHandler.h"

vup::VBOHandler::VBOHandler()
{
  m_vbos = std::map<std::string, vup::VBO>();
  m_interopVBOs = std::map<std::string, vup::VBO>();
}

vup::VBOHandler::~VBOHandler()
{
}

vup::VBO vup::VBOHandler::getVBO(std::string name)
{
  std::map<std::string, vup::VBO>::iterator it = m_vbos.find(name);
  if (it != m_vbos.end())
  {
    return it->second;
  }
  else
  {
    // TODO: throw Exception
    throw std::exception();
  }
}

vup::VBO vup::VBOHandler::getInteropVBO(std::string name)
{
  std::map<std::string, vup::VBO>::iterator it = m_interopVBOs.find(name);
  if (it != m_interopVBOs.end())
  {
    return it->second;
  }
  else
  {
    // TODO: throw Exception
    throw std::exception();
  }
}

GLuint vup::VBOHandler::getVBOHandle(std::string name)
{
  return getVBO(name).handle;
}

GLuint vup::VBOHandler::getInteropVBOHandle(std::string name)
{
  return getInteropVBO(name).handle;
}
