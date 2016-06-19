// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_UNIFORM_GRID
#define VUP_UNIFORM_GRID

#include "vup/defs.h"
#include "vup/Exceptions/BufferCreationException.h"

namespace vup {


class UniformGrid {
public:
  UniformGrid(int maxNeighbors, float cellsize, float gridlength, cl::Context context, cl_mem_flags flags);
  ~UniformGrid();
  cl::Buffer getCellBuffer() { return m_cellBuffer; }
  cl::Buffer getNeighborCounterBuffer() { return m_neighborCounterBuffer; }
  int getCellAmount() { return m_cellamount; }
  int getMaxGridCapacity() { return m_maxGridCapacity; }
  int getMaxNeighbors() { return m_maxNeighbors; }
  int getLineSize() { return m_lineSize; }
private:
  int m_maxNeighbors;
  int m_cellamount;
  int m_maxGridCapacity;
  int m_lineSize;
  float m_gridlength;
  float m_cellsize;
  cl::Buffer m_cellBuffer;
  cl::Buffer m_neighborCounterBuffer;
  std::vector<int> m_cellContents;
  std::vector<int> m_neighborCounter;
};

}

#endif
