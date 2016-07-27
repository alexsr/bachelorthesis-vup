#include "ParticleSimulation.h"

vup::ParticleSimulation::ParticleSimulation(const char * kernelpath, const char * kernelinfopath, const char * datapath)
{
  m_clBasis = new OpenCLBasis(1, CL_DEVICE_TYPE_GPU, 0);
  m_buffers = new BufferHandler(m_clBasis->context());

  // Use data from DataLoader to populate clBuffers
  DataLoader dataloader(datapath);
  m_particleCount = dataloader.getParticleCount();
  m_size = dataloader.getParticleSize();
  m_queue = new ParticleQueue(m_clBasis->context(), m_particleCount);
  std::map<std::string, std::pair<int, datatype>> interop = dataloader.getInteropIdentifiers();
  for (auto &id : interop) {
    int offset = 0;
    if (id.second.second == vup::FLOAT) {
      m_buffers->createVBO<float>(id.first + "_vbo", id.second.first, m_particleCount, 1, true, GL_STREAM_DRAW);
      m_buffers->createBufferGL(id.first, CL_MEM_READ_WRITE, id.first + "_vbo");
      for (auto &t : dataloader.getSystems()) {
        for (auto &s : t.second) {
          m_buffers->updateSubVBO(id.first + "_vbo", s.second.getFloatData(id.first), offset, s.second.getCount());
          offset += s.second.getCount();
        }
      }
    }
    else if (id.second.second == vup::VEC4) {
      m_buffers->createVBO<glm::vec4>(id.first + "_vbo", id.second.first, m_particleCount, 4, true, GL_STREAM_DRAW);
      m_buffers->createBufferGL(id.first, CL_MEM_READ_WRITE, id.first + "_vbo");
      for (auto &t : dataloader.getSystems()) {
        for (auto &s : t.second) {
          m_buffers->updateSubVBO(id.first + "_vbo", s.second.getVec4Data(id.first), offset, s.second.getCount());
          offset += s.second.getCount();
        }
      }
    }
    else {
      continue;
    }
  }
  typeIdentifiers global = dataloader.getGlobalIdentifiers();
  for (auto &id : global) {
    int offset = 0;
    if (id.second == vup::FLOAT) {
      m_buffers->createBuffer<float>(id.first, CL_MEM_READ_WRITE, m_particleCount);
      for (auto &t : dataloader.getSystems()) {
        for (auto &s : t.second) {
          m_queue->writeBuffer(m_buffers->getBuffer(id.first), true, offset, s.second.getCount(), &(s.second.getFloatData(id.first))[0]);
          offset += s.second.getCount();
        }
      }
    }
    else if (id.second == vup::VEC4) {
      m_buffers->createBuffer<glm::vec4>(id.first, CL_MEM_READ_WRITE, m_particleCount);
      for (auto &t : dataloader.getSystems()) {
        for (auto &s : t.second) {
          m_queue->writeBuffer(m_buffers->getBuffer(id.first), true, offset, s.second.getCount(), &(s.second.getVec4Data(id.first))[0]);
          offset += s.second.getCount();
        }
      }
    }
    else {
      continue;
    }
  }
  m_types = dataloader.getTypes();
  for (auto &type : m_types) {
    int typeVarSize = dataloader.getTypeParticleCount(type.first);
    std::map<std::string, vup::ParticleSystem> systems = dataloader.getSystemsOfType(type.first);
    for (auto &var : type.second.getTypeSpecificIdentifiers()) {
      int offset = 0;
      if (var.second == vup::FLOAT) {
        m_buffers->createBuffer<float>(var.first, CL_MEM_READ_WRITE, typeVarSize);
        for (auto &smap : systems) {
          vup::ParticleSystem s = smap.second;
          m_queue->writeBuffer(m_buffers->getBuffer(var.first), true, offset, s.getCount(), &s.getFloatData(var.first)[0]);
          offset += s.getCount();
        }
      }
      else if (var.second == vup::VEC4) {
        m_buffers->createBuffer<glm::vec4>(var.first, CL_MEM_READ_WRITE, typeVarSize);
        for (auto &smap : systems) {
          vup::ParticleSystem s = smap.second;
          m_queue->writeBuffer(m_buffers->getBuffer(var.first), true, offset, s.getCount(), &s.getVec4Data(var.first)[0]);
          offset += s.getCount();
        }
      }
      else {
        continue;
      }
    }
  }

  //KernelInfoLoader kernelinfoloader()
  m_kernels = new KernelHandler(m_clBasis->context(), m_clBasis->device(), kernelpath);
  //m_kernels->initKernels();
}

vup::ParticleSimulation::~ParticleSimulation()
{
}
