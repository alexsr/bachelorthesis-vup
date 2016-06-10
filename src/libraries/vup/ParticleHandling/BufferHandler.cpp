#include "BufferHandler.h"


vup::BufferHandler::BufferHandler(cl::Context defaultContext)
{
  m_defaultContext = defaultContext;
  m_buffers = std::map<std::string, cl::Buffer>();
  m_glBuffers = std::map<std::string, cl::BufferGL>();
  m_vbos = std::map<std::string, vup::VBO>();
  m_interopVBOs = std::map<std::string, vup::VBO>();
}

vup::BufferHandler::~BufferHandler()
{
}

void vup::BufferHandler::addBuffer(std::string name, cl::Buffer buffer)
{
  if (doesBufferExist(name)) {
    std::cout << "WARNING: Kernel " << name << "already exists.";
  }
  cl_int clError;
  m_buffers[name] = buffer;
  if (clError != CL_SUCCESS) {
    throw vup::BufferCreationException(name, clError);
  }
}

void vup::BufferHandler::createBufferGL(std::string name, cl_mem_flags flags, std::string vbo)
{
  if (doesBufferGLExist(name)) {
    std::cout << "WARNING: Kernel " << name << "already exists.";
  }
  cl_int clError;
  m_glBuffers[name] = cl::BufferGL(m_defaultContext, flags, getInteropVBOHandle(vbo), &clError);
  m_glBuffersVector.push_back(m_glBuffers[name]);
  if (clError != CL_SUCCESS) {
    throw vup::BufferCreationException(name, clError);
  }
}

void vup::BufferHandler::addBufferGL(std::string name, cl::BufferGL buffer)
{
  if (doesBufferGLExist(name)) {
    std::cout << "WARNING: Kernel " << name << "already exists.";
  }
  cl_int clError;
  m_glBuffers[name] = buffer;
  if (clError != CL_SUCCESS) {
    throw vup::BufferCreationException(name, clError);
  }
}

cl::Buffer vup::BufferHandler::getBuffer(std::string name)
{
  if (doesBufferExist(name))
  {
    return m_buffers[name];
  }
  else
  {
    throw vup::BufferNotFoundException(name);
  }
}

cl::BufferGL vup::BufferHandler::getBufferGL(std::string name)
{
  if (doesBufferGLExist(name))
  {
    return m_glBuffers[name];
  }
  else
  {
    throw vup::BufferNotFoundException(name);
  }
}

bool vup::BufferHandler::doesBufferExist(std::string name)
{
  std::map<std::string, cl::Buffer>::iterator it = m_buffers.find(name);
  if (it != m_buffers.end())
  {
    return true;
  }
  return false;
}

bool vup::BufferHandler::doesBufferGLExist(std::string name)
{
  std::map<std::string, cl::BufferGL>::iterator it = m_glBuffers.find(name);
  if (it != m_glBuffers.end())
  {
    return true;
  }
  return false;
}

vup::VBO vup::BufferHandler::getVBO(std::string name)
{
  std::map<std::string, vup::VBO>::iterator it = m_vbos.find(name);
  if (it != m_vbos.end())
  {
    return it->second;
  }
  else
  {
    throw vup::BufferNotFoundException(name);
  }
}

vup::VBO vup::BufferHandler::getInteropVBO(std::string name)
{
  std::map<std::string, vup::VBO>::iterator it = m_interopVBOs.find(name);
  if (it != m_interopVBOs.end())
  {
    return it->second;
  }
  else
  {
    throw vup::BufferNotFoundException(name);
  }
}

GLuint vup::BufferHandler::getVBOHandle(std::string name)
{
  return getVBO(name).handle;
}

GLuint vup::BufferHandler::getInteropVBOHandle(std::string name)
{
  return getInteropVBO(name).handle;
}
