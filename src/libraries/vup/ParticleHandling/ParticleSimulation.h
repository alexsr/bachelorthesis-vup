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
#include <memory>

namespace vup {

// Manages OpenCL and OpenGL buffers and provides 

class ParticleSimulation
{
public:
  ParticleSimulation(std::string configpath);
  ~ParticleSimulation();
  void init();
  void run();
  float getSize() { return m_size; }
  int getParticleCount() { return m_particleCount; }
  std::map<std::string, vup::VBO> getInteropVBOs() { return m_buffers->getInteropVBOs(); }
  void updateConstant(const char* name, int index, float c);
  void reload();
  void reloadKernel();

private:
  std::string m_configpath;
  std::string m_kernelpath;
  std::string m_datapath;
  std::string m_kernelinfopath;
  kernelInfoMap m_kernelinfos;
  int m_particleCount;
  float m_size;
  GPUBoilerplate* m_clBasis;
  BufferHandler* m_buffers;
  Queue* m_queue;
  std::unique_ptr<KernelHandler> m_kernels;
  std::vector<std::string> m_kernelorder;
  std::vector<std::string> m_initkernels;
  std::map<std::string, int> m_kernelSize;
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
