#include "TypeBuffer.h"

vup::TypeBuffer::TypeBuffer()
{
  m_indices = std::vector<int>();
  m_range = 0;
  m_size = 0;
  m_buffer = cl::Buffer();
}

vup::TypeBuffer::TypeBuffer(std::vector<int> indices, cl_mem_flags flags, cl_bool blocking, cl::Context context, int maxSize)
{
  std::sort(indices.begin(), indices.end());
  indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
  m_indices = indices;
  m_range = m_indices.size();
  m_size = m_range * sizeof(int);
  m_maxSize = maxSize;
  cl_int clError;
  m_buffer = cl::Buffer(context, flags, sizeof(int) * m_maxSize, nullptr, &clError);
  if (clError != CL_SUCCESS) {
    throw vup::BufferCreationException("TypeBuffer", clError);
  }
}

vup::TypeBuffer::~TypeBuffer()
{
}

void vup::TypeBuffer::addIndices(std::vector<int> indices)
{
  m_indices.insert(m_indices.end(), indices.begin(), indices.end());
  std::sort(m_indices.begin(), m_indices.end());
  m_indices.erase(std::unique(m_indices.begin(), m_indices.end()), m_indices.end());
  m_range = m_indices.size();
  m_size = m_range * sizeof(int);
}

void vup::TypeBuffer::removeIndices(std::vector<int> indices)
{
  std::sort(indices.begin(), indices.end());
  indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
  for (unsigned int i = 0; i < indices.size(); i++) {
    removeIndex(indices.at(i));
  }
  m_range = m_indices.size();
  m_size = m_range * sizeof(int);
}

void vup::TypeBuffer::removeIndex(int index)
{
  m_indices.erase(std::remove(m_indices.begin(), m_indices.end(), index), m_indices.end());
}

void * vup::TypeBuffer::getIndexPointer(int type)
{
  return &m_indices[0];
}
