#include "BufferHandler.h"


vup::BufferHandler::BufferHandler(cl::Context defaultContext)
{
  m_defaultContext = defaultContext;
  m_buffers = std::map<std::string, cl::Buffer>();
  m_glBuffers = std::map<std::string, cl::BufferGL>();
}

vup::BufferHandler::~BufferHandler()
{
}

void vup::BufferHandler::add(std::string name, cl_mem_flags flags, int size)
{
  if (doesBufferExist(name)) {
    std::cout << "WARNING: Kernel " << name << "already exists.";
  }
  cl_int clError;
  m_buffers[name] = cl::Buffer(m_defaultContext, flags, size, NULL, &clError);
  if (clError != CL_SUCCESS) {
    throw std::exception();
  }
}

void vup::BufferHandler::addGL(std::string name, cl_mem_flags flags, GLuint vboHandle)
{
  if (doesGLBufferExist(name)) {
    std::cout << "WARNING: Kernel " << name << "already exists.";
  }
  cl_int clError;
  m_glBuffers[name] = cl::BufferGL(m_defaultContext, flags, vboHandle, &clError);
  m_glBuffersVector.push_back(m_glBuffers[name]);
  if (clError != CL_SUCCESS) {
    throw std::exception();
  }
}

cl::Buffer vup::BufferHandler::get(std::string name)
{
  if (doesBufferExist(name))
  {
    return m_buffers[name];
  }
  else
  {
    // TODO: throw Exception
    throw std::exception();
  }
}

cl::BufferGL vup::BufferHandler::getGL(std::string name)
{
  if (doesGLBufferExist(name))
  {
    return m_glBuffers[name];
  }
  else
  {
    // TODO: throw Exception
    throw std::exception();
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

bool vup::BufferHandler::doesGLBufferExist(std::string name)
{
  std::map<std::string, cl::BufferGL>::iterator it = m_glBuffers.find(name);
  if (it != m_glBuffers.end())
  {
    return true;
  }
  return false;
}
