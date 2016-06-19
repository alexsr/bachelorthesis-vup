#include "UniformGrid.h"

vup::UniformGrid::UniformGrid(int cellCapacity, float cellsize, float gridradius, cl::Context context, cl_mem_flags flags)
{
  m_cellCapacity = cellCapacity;
  m_gridradius = gridradius;
  m_cellsize = cellsize;
  m_lineSize = (int) 2.0f * m_gridradius / m_cellsize;
  m_cellamount = m_lineSize * m_lineSize * m_lineSize;
  m_maxGridCapacity = (int) m_cellCapacity * m_cellamount;
  m_cellContents = std::vector<int>(m_maxGridCapacity);
  m_neighborCounter = std::vector<int>(m_cellamount);
  for (int i = 0; i < m_lineSize; i++) {
    for (int j = 0; j < m_lineSize; j++) {
      for (int k = 0; k < m_lineSize; k++) {
        m_neighborCounter[i * m_lineSize * m_lineSize + j * m_lineSize + k] = 0;
        for (int n = 0; n < m_cellCapacity; n++) {
          m_cellContents[i * m_lineSize * m_lineSize * m_cellCapacity + j * m_lineSize * m_cellCapacity + k * m_cellCapacity + n] = -1;
        }
      }
    }
  }
  cl_int clError;
  m_gridBuffer = cl::Buffer(context, flags, m_maxGridCapacity * sizeof(int), &m_cellContents[0], &clError);
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
