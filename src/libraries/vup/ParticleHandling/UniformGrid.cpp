#include "UniformGrid.h"

vup::UniformGrid::UniformGrid(int maxNeighbors, float cellsize, float gridlength, cl::Context context, cl_mem_flags flags)
{
  m_maxNeighbors = maxNeighbors;
  m_gridlength = gridlength;
  m_cellsize = cellsize;
  m_lineSize = m_gridlength / m_cellsize;
  m_cellamount = m_lineSize * m_lineSize * m_lineSize;
  m_maxGridCapacity = (int) m_maxNeighbors * m_cellamount;
  m_cellContents = std::vector<int>(m_maxGridCapacity);
  m_neighborCounter = std::vector<int>(m_cellamount);
  for (int i = 0; i < m_lineSize; i++) {
    for (int j = 0; j < m_lineSize; j++) {
      for (int k = 0; k < m_lineSize; k++) {
        m_neighborCounter[i * m_lineSize * m_lineSize + j * m_lineSize + k] = 0;
        for (int n = 0; n < m_maxNeighbors; n++) {
          m_cellContents[i * m_lineSize * m_lineSize * m_maxNeighbors + j * m_lineSize * m_maxNeighbors + k * m_maxNeighbors + n] = -1;
        }
      }
    }
  }
  cl_int clError;
  m_cellBuffer = cl::Buffer(context, flags, m_maxGridCapacity * sizeof(int), &m_cellContents[0], &clError);
  if (clError != CL_SUCCESS) {
    throw vup::BufferCreationException("Uniform grid cell buffer", clError);
  }
  m_neighborCounterBuffer = cl::Buffer(context, flags, m_cellamount * sizeof(int), &m_neighborCounter[0], &clError);
  if (clError != CL_SUCCESS) {
    throw vup::BufferCreationException("Uniform grid counter buffer", clError);
  }
}

vup::UniformGrid::~UniformGrid()
{
}
