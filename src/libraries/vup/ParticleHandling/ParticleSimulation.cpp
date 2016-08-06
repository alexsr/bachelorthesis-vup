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
  typeIdentifiers interop = dataloader.getInteropIdentifiers();
  for (auto &id : interop) {
    int offset = 0;
    if (id.second.format == vup::FLOAT) {
      m_buffers->createVBO<float>(id.first + "_vbo", id.second.loc, m_particleCount * id.second.instances, 1, true, GL_STREAM_DRAW);
      m_buffers->createBufferGL(id.first, CL_MEM_READ_WRITE, id.first + "_vbo");
      for (auto &t : dataloader.getSystems()) {
        for (auto &s : t.second) {
          m_buffers->updateSubVBO(id.first + "_vbo", s.second.getFloatData(id.first), offset, s.second.getCount() * id.second.instances);
          offset += s.second.getCount() * id.second.instances;
        }
      }
    }
    else if (id.second.format == vup::VEC4) {
      m_buffers->createVBO<glm::vec4>(id.first + "_vbo", id.second.loc, m_particleCount * id.second.instances, 4, true, GL_STREAM_DRAW);
      m_buffers->createBufferGL(id.first, CL_MEM_READ_WRITE, id.first + "_vbo");
      for (auto &t : dataloader.getSystems()) {
        for (auto &s : t.second) {
          m_buffers->updateSubVBO(id.first + "_vbo", s.second.getVec4Data(id.first), offset, s.second.getCount() * id.second.instances);
          offset += s.second.getCount() * id.second.instances;
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
    if (id.second.format == vup::INT) {
      m_buffers->createBuffer<int>(id.first, CL_MEM_READ_WRITE, m_particleCount * id.second.instances);
      for (auto &t : dataloader.getSystems()) {
        for (auto &s : t.second) {
          m_queue->writeBuffer(m_buffers->getBuffer(id.first), true, offset * sizeof(int), s.second.getCount() * sizeof(int) * id.second.instances, &(s.second.getIntData(id.first))[0]);
          offset += s.second.getCount() * id.second.instances;
        }
      }
    }
    else if (id.second.format == vup::FLOAT) {
      m_buffers->createBuffer<float>(id.first, CL_MEM_READ_WRITE, m_particleCount * id.second.instances);
      for (auto &t : dataloader.getSystems()) {
        for (auto &s : t.second) {
          m_queue->writeBuffer(m_buffers->getBuffer(id.first), true, offset * sizeof(float), s.second.getCount() * sizeof(float) * id.second.instances, &(s.second.getFloatData(id.first))[0]);
          offset += s.second.getCount() * id.second.instances;
        }
      }
    }
    else if (id.second.format == vup::VEC4) {
      m_buffers->createBuffer<glm::vec4>(id.first, CL_MEM_READ_WRITE, m_particleCount * id.second.instances);
      for (auto &t : dataloader.getSystems()) {
        for (auto &s : t.second) {
          m_queue->writeBuffer(m_buffers->getBuffer(id.first), true, offset * sizeof(glm::vec4), s.second.getCount() * sizeof(glm::vec4) * id.second.instances, &(s.second.getVec4Data(id.first))[0]);
          offset += s.second.getCount() * id.second.instances;
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
      if (var.second.format == vup::FLOAT) {
        m_buffers->createBuffer<float>(var.first, CL_MEM_READ_WRITE, typeVarSize * var.second.instances);
        for (auto &smap : systems) {
          vup::ParticleSystem s = smap.second;
          m_queue->writeBuffer(m_buffers->getBuffer(var.first), true, offset * sizeof(float), s.getCount() * sizeof(float) * var.second.instances, &s.getFloatData(var.first)[0]);
          offset += s.getCount() * var.second.instances;
        }
      }
      else if (var.second.format == vup::INT) {
        m_buffers->createBuffer<int>(var.first, CL_MEM_READ_WRITE, typeVarSize * var.second.instances);
        for (auto &smap : systems) {
          vup::ParticleSystem s = smap.second;
          m_queue->writeBuffer(m_buffers->getBuffer(var.first), true, offset * sizeof(int), s.getCount() * sizeof(int) * var.second.instances, &s.getIntData(var.first)[0]);
          offset += s.getCount() * var.second.instances;
        }
      }
      else if (var.second.format == vup::VEC4) {
        m_buffers->createBuffer<glm::vec4>(var.first, CL_MEM_READ_WRITE, typeVarSize * var.second.instances);
        for (auto &smap : systems) {
          vup::ParticleSystem s = smap.second;
          m_queue->writeBuffer(m_buffers->getBuffer(var.first), true, offset * sizeof(glm::vec4), s.getCount() * sizeof(glm::vec4) * var.second.instances, &s.getVec4Data(var.first)[0]);
          offset += s.getCount() * var.second.instances;
        }
      }
      else {
        continue;
      }
    }
  }
  int globalOffset = 0;
  for (auto &t : dataloader.getSystems()) {
    int typeOffset = 0;
    for (auto &s : t.second) {
      int indexCount = s.second.getCount();
      m_globalIndices[s.first] = std::vector<int>(indexCount);
      m_typeIndices[s.first] = std::vector<int>(indexCount);
      for (int i = 0; i < indexCount; i++) {
        m_globalIndices[s.first].at(i) = i + globalOffset;
        m_typeIndices[s.first].at(i) = i + typeOffset;
      }
      typeOffset += indexCount;
      globalOffset += indexCount;
    }
  }


  m_kernels = new KernelHandler(m_clBasis->context(), m_clBasis->device(), kernelpath);
  std::map<std::string, std::map<std::string, KernelArgument>> arguments = m_kernels->getKernelArguments();
  KernelInfoLoader kinfLoader(kernelinfopath);
  std::map<std::string, vup::KernelInfo> kernelinfos = kinfLoader.getKernelInfos();
  for (auto &kinf : kernelinfos) {
    m_kernelorder.resize(m_kernelorder.size() + kinf.second.pos.size());
  }
  for (auto &kinf : kernelinfos) {
    if (!doesKeyExist(kinf.first, arguments)) {
      throw new CorruptDataException("", "Kernel does not exist.");
    }
    m_kernels->initKernel(kinf.first);
    for (int i = 0; i < kinf.second.pos.size(); i++) {
      m_kernelorder.at(kinf.second.pos.at(i)) = kinf.first;
    }
    std::vector<int> typeIndices;
    std::vector<int> globalIndices;
    for (auto &onSystem : kinf.second.onSystems) {
      typeIndices.insert(typeIndices.end(), m_typeIndices[onSystem].begin(), m_typeIndices[onSystem].end());
      globalIndices.insert(globalIndices.end(), m_globalIndices[onSystem].begin(), m_globalIndices[onSystem].end());
    }
    for (auto &onType : kinf.second.onTypes) {
      for (auto &onSystem : dataloader.getSystemsOfType(onType)) {
        globalIndices.insert(globalIndices.end(), m_globalIndices[onSystem.first].begin(), m_globalIndices[onSystem.first].end());
      }
    }
    m_kernelSize[kinf.first] = globalIndices.size();
    if (globalIndices.size() == 0) {
      m_kernelSize[kinf.first] = m_particleCount;
    }
    else {
      m_buffers->createBuffer<int>("globalIndices" + kinf.first, CL_MEM_READ_WRITE, globalIndices.size());
      m_queue->writeBuffer(m_buffers->getBuffer("globalIndices" + kinf.first), globalIndices.size() * sizeof(int), &globalIndices[0]);
    }
    if (typeIndices.size() != 0) {
      m_buffers->createBuffer<int>("typeIndices" + kinf.first, CL_MEM_READ_WRITE, typeIndices.size());
      m_queue->writeBuffer(m_buffers->getBuffer("typeIndices" + kinf.first), typeIndices.size() * sizeof(int), &typeIndices[0]);
    }
    for (auto &arg : arguments[kinf.first]) {
      if (arg.first == "globalIndices") {
        if (globalIndices.size() != 0) {
          m_kernels->setArg(kinf.first.c_str(), arg.second.index, m_buffers->getBuffer("globalIndices" + kinf.first));
        }
      }
      else if (arg.first == "typeIndices") {
        if (typeIndices.size() != 0) {
          m_kernels->setArg(kinf.first.c_str(), arg.second.index, m_buffers->getBuffer("typeIndices" + kinf.first));
        }
      }
      else if (arg.second.constant) {
        if (doesKeyExist(arg.first, kinf.second.constants)) {
          m_kernels->setArg(kinf.first.c_str(), arg.second.index, kinf.second.constants[arg.first]);
          std::cout << "Set " << arg.first << " at " << arg.second.index << " of " << kinf.first << std::endl;
        } else {
          std::cout << arg.first << " not found." << std::endl;
        }
      }
      else {
        if (doesKeyExist(arg.first, interop)) {
          m_kernels->setArg(kinf.first.c_str(), arg.second.index, m_buffers->getBufferGL(arg.first));
          std::cout << "Set " << arg.first << " at " << arg.second.index << " of " << kinf.first << std::endl;
        }
        else {
          m_kernels->setArg(kinf.first.c_str(), arg.second.index, m_buffers->getBuffer(arg.first));
          std::cout << "Set " << arg.first << " at " << arg.second.index << " of " << kinf.first << std::endl;
        }
      }
    }
  }
}

vup::ParticleSimulation::~ParticleSimulation()
{
}

void vup::ParticleSimulation::run()
{
  glFinish();
  m_queue->acquireGL(&(m_buffers->getGLBuffers()));
  for (std::string &kernel : m_kernelorder) {
    //std::cout << "Running " << kernel << std::endl;
    m_queue->runRangeKernel(m_kernels->get(kernel), m_kernelSize.at(kernel));
    //std::cout << std::endl;
  }
  m_queue->releaseGL(&(m_buffers->getGLBuffers()));
}

void vup::ParticleSimulation::updateConstant(const char * name, int index, float c)
{
  m_kernels->setArg(name, index, c);
}
