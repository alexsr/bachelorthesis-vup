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
  UniformGrid(int maxNeighbors, float cellsize, float gridradius, cl::Context context, cl_mem_flags flags);
  ~UniformGrid();
  cl::Buffer getGridBuffer() { return m_gridBuffer; }
  cl::Buffer getNeighborCounterBuffer() { return m_neighborCounterBuffer; }
  int getCellAmount() { return m_cellamount; }
  int getMaxGridCapacity() { return m_maxGridCapacity; }
  int getMaxNeighbors() { return m_cellCapacity; }
  int getLineSize() { return m_lineSize; }
private:
  int m_cellCapacity;
  int m_cellamount;
  int m_maxGridCapacity;
  int m_lineSize;
  float m_gridradius;
  float m_cellsize;
  cl::Buffer m_gridBuffer;
  cl::Buffer m_neighborCounterBuffer;
  std::vector<int> m_cellContents;
  std::vector<int> m_neighborCounter;
};

}

#endif
