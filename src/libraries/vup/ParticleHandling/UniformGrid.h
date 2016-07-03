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
  UniformGrid(int maxNeighbors, int cellsPerLine, float gridradius, cl::Context context, cl_mem_flags flags);
  ~UniformGrid();
  cl::Buffer getGridBuffer() { return m_gridBuffer; }
  cl::Buffer getCounterBuffer() { return m_counterBuffer; }
  int getCellAmount() { return m_cellamount; }
  int getMaxGridCapacity() { return m_maxGridCapacity; }
  int getCellCapacity() { return m_cellCapacity; }
  int getCellsPerLine() { return m_cellsPerLine; }
  float getGridRadius() { return m_gridradius; }
  std::vector<int> getGridData() { return m_gridData; }
  std::vector<int> getCounterData() { return m_counterData; }
private:
  int m_cellCapacity;
  int m_cellamount;
  int m_maxGridCapacity;
  int m_cellsPerLine;
  float m_gridradius;
  cl::Buffer m_gridBuffer;
  cl::Buffer m_counterBuffer;
  std::vector<int> m_gridData;
  std::vector<int> m_counterData;
};

}

#endif
