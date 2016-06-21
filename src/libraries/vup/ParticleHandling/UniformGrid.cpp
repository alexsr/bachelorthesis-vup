#include "UniformGrid.h"

vup::UniformGrid::UniformGrid(int cellCapacity, float cellsize, float gridradius, cl::Context context, cl_mem_flags flags)
{
  m_cellCapacity = cellCapacity;
  m_gridradius = gridradius;
  m_cellsize = cellsize;
  m_lineSize = (int) (2.0f * m_gridradius) / m_cellsize;
  m_cellamount = m_lineSize * m_lineSize * m_lineSize;
  m_maxGridCapacity = (int) m_cellCapacity * m_cellamount;
  m_gridData = std::vector<int>(m_maxGridCapacity);
  m_counterData = std::vector<int>(m_cellamount);
  for (int i = 0; i < m_lineSize; i++) {
    for (int j = 0; j < m_lineSize; j++) {
      for (int k = 0; k < m_lineSize; k++) {
        m_counterData[i * m_lineSize * m_lineSize + j * m_lineSize + k] = 0;
        for (int n = 0; n < m_cellCapacity; n++) {
          m_gridData[i * m_lineSize * m_lineSize * m_cellCapacity + j * m_lineSize * m_cellCapacity + k * m_cellCapacity + n] = -1;
        }
      }
    }
  }
  cl_int clError;
  m_gridBuffer = cl::Buffer(context, flags, m_gridData.size() * sizeof(int), &m_gridData[0], &clError);
  if (clError != CL_SUCCESS) {
    throw vup::BufferCreationException("Uniform grid cell buffer", clError);
  }
  m_counterBuffer = cl::Buffer(context, flags, m_counterData.size() * sizeof(int), &m_counterData[0], &clError);
  if (clError != CL_SUCCESS) {
    throw vup::BufferCreationException("Uniform grid counter buffer", clError);
  }
}

vup::UniformGrid::~UniformGrid()
{
}
