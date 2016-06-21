// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_UNIFORM_GRID
#define VUP_UNIFORM_GRID

#include "vup/defs.h"
#include "vup/OpenCLUtil/OpenCLUtil.h"
#include "vup/Exceptions/BufferCreationException.h"

namespace vup {


class UniformGrid {
public:
  UniformGrid(int maxNeighbors, float cellsize, float gridradius, cl::Context context, cl_mem_flags flags);
  ~UniformGrid();
  cl::Buffer getGridBuffer() { return m_gridBuffer; }
  cl::Buffer getCounterBuffer() { return m_counterBuffer; }
  int getCellAmount() { return m_cellamount; }
  int getMaxGridCapacity() { return m_maxGridCapacity; }
  int getCellCapacity() { return m_cellCapacity; }
  int getLineSize() { return m_lineSize; }
  float getCellSize() { return m_cellsize; }
  std::vector<int> getGridData() { return m_gridData; }
  std::vector<int> getCounterData() { return m_counterData; }
private:
  int m_cellCapacity;
  int m_cellamount;
  int m_maxGridCapacity;
  int m_lineSize;
  float m_gridradius;
  float m_cellsize;
  cl::Buffer m_gridBuffer;
  cl::Buffer m_counterBuffer;
  std::vector<int> m_gridData;
  std::vector<int> m_counterData;
};

}

#endif
