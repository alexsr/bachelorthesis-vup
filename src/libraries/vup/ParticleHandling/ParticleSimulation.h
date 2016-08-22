// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PARTICLESIMULATION_H
#define VUP_PARTICLESIMULATION_H

#include "vup/defs.h"
#include "vup/Exceptions/CorruptDataException.h"
#include "vup/ParticleHandling/DataLoader.h"
#include "vup/ParticleHandling/KernelInfoLoader.h"
#include "vup/ParticleHandling/BufferHandler.h"
#include "vup/OpenCLUtil/OpenCLUtil.h"
#include "vup/Rendering/ParticleRenderer.h"

namespace vup {

// Manages OpenCL and OpenGL buffers and provides 

class ParticleSimulation
{
public:
  ParticleSimulation(const char* kernelpath, const char * kernelinfopath, const char* datapath);
  ~ParticleSimulation();
  void init();
  void run();
  float getSize() { return m_size; }
  int getParticleCount() { return m_particleCount; }
  std::map<std::string, vup::VBO> getInteropVBOs() { return m_buffers->getInteropVBOs(); }
  void updateConstant(const char* name, int index, float c);
  void reload();

private:
  const char* m_kernelpath;
  const char* m_datapath;
  const char* m_kernelinfopath;
  int m_particleCount;
  float m_size;
  OpenCLBasis* m_clBasis;
  BufferHandler* m_buffers;
  std::vector<std::string> m_kernelorder;
  std::vector<std::string> m_initkernels;
  std::map<std::string, int> m_kernelSize;
  KernelHandler* m_kernels;
  Queue* m_queue;
  std::map<std::string, vup::ParticleType> m_types;
  std::map<std::string, std::vector<int>> m_globalIndices;
  std::map<std::string, std::vector<int>> m_typeIndices;

  template <typename T> bool doesKeyExist(std::string key, std::map<std::string, T> m);
};

template<typename T>
inline bool ParticleSimulation::doesKeyExist(std::string key, std::map<std::string, T> m)
{
  std::map<std::string, T>::iterator it = m.find(key);
  return it != m.end();
}
}

#endif
