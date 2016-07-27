// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PARTICLESIMULATION_H
#define VUP_PARTICLESIMULATION_H

#include "vup/defs.h"
#include "vup/Exceptions/CorruptDataException.h"
#include "vup/ParticleHandling/DataLoader.h"
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
  void loadProgram(const char* path);

private:
  const char* m_kernelpath;
  const char* m_datapath;
  int m_particleCount;
  float m_size;
  OpenCLBasis* m_clBasis;
  BufferHandler* m_buffers;
  std::vector<std::string> m_kernelorder;
  KernelHandler* m_kernels;
  ParticleQueue* m_queue;
  std::map<std::string, vup::ParticleType> m_types;

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
