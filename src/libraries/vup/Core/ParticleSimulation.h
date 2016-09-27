// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PARTICLESIMULATION_H
#define VUP_PARTICLESIMULATION_H

#include "vup/Core/defs.h"
#include "vup/Exceptions/CorruptDataException.h"
#include "vup/Util/DataLoader.h"
#include "vup/Util/KernelInfoLoader.h"
#include "vup/OpenCLUtil/OpenCLUtil.h"
#include "vup/Rendering/ParticleRenderer.h"
#include <memory>

namespace vup {

// Handles the whole particle simulation.
// Loads kernels, kernel info and data.
// Creates buffers and writes data into them
// Executes kernels in the specified order.
class ParticleSimulation
{
public:
  // Loads all the info from the files defined in the config file.
  // Also creates the Boilerplate, buffer handler and queue.
  ParticleSimulation(std::string configpath, int platformID, cl_device_type deviceType, int deviceID);
  ~ParticleSimulation();
  // Executes the initializing kernels.
  // This should only be called after a full reload.
  void init();
  // Executes all kernels except initializing kernels in the order specified in m_kernelorder.
  void run();
  float getSize() { return m_size; }
  int getParticleCount() { return m_particleCount; }
  std::map<std::string, vup::VBO> getInteropVBOs() { return m_buffers->getInteropVBOs(); }
  // Updates the constant of kernel name at index with c.
  void updateConstant(const char* name, int index, float c);
  // Deletes all previously loaded data and reloads the config from m_configpath.
  // Creates kernel handler, populates buffers and associates them with kernels, sets kernel execution order,
  // all corresponding to the information from the config files.
  // All data is written to buffers and then cleared from the CPU.
  void reload();
  // Reloads the kernel program and associates the data again.
  // This method does however not reload the kernel info and relies on already loaded kernel info.
  void reloadKernel();

private:
  template <typename T> bool doesKeyExist(std::string key, std::map<std::string, T> m);

  // These variables are pointers, so there is no need for a default contructor in these classes.
  GPUBoilerplate* m_clBasis;
  BufferHandler* m_buffers;
  Queue* m_queue;

  std::string m_configpath;
  std::string m_kernelpath;
  std::string m_datapath;
  std::string m_kernelinfopath;
  kernelInfoMap m_kernelinfos;
  int m_particleCount;
  float m_size;
  // A unique pointer is used to prevent dangling pointers.
  std::unique_ptr<KernelHandler> m_kernels;
  std::vector<std::string> m_initkernels;
  std::vector<std::string> m_kernelorder;
  std::map<std::string, int> m_kernelSize;
  std::map<std::string, std::vector<int>> m_globalIndices;
  std::map<std::string, std::vector<int>> m_typeIndices;
};

template<typename T>
inline bool ParticleSimulation::doesKeyExist(std::string key, std::map<std::string, T> m)
{
  std::map<std::string, T>::iterator it = m.find(key);
  return it != m.end();
}
}

#endif
