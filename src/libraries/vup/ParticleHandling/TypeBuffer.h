// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_TYPEBUFFER_H
#define VUP_TYPEBUFFER_H

#include "vup/defs.h"
#include "vup/Exceptions/BufferCreationException.h"
#include <vector>
#include <algorithm>

namespace vup {

class TypeBuffer {
public:
  TypeBuffer();
  TypeBuffer(std::vector<int> indices, cl_mem_flags flags, cl_bool blocking, cl::Context context, int maxSize);
  ~TypeBuffer();
  int size() { return m_size; }
  int range() { return m_range; }
  std::vector<int> indices() { return m_indices; }
  cl::Buffer buffer() { return m_buffer; }
  void addIndices(std::vector<int> indices);
  void removeIndices(std::vector<int> indices);
  void removeIndex(int index);
  void * getIndexPointer(int type);

private:
  std::vector<int> m_indices;
  int m_range;
  int m_size;
  int m_maxSize;
  cl::Buffer m_buffer;
};

}

#endif
