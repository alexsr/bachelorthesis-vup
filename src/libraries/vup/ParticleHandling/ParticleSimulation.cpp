#include "ParticleSimulation.h"

vup::ParticleSimulation::ParticleSimulation(std::string configpath, int platformID, cl_device_type deviceType, int deviceID)
{
  m_clBasis = new GPUBoilerplate(platformID, deviceType, deviceID);
  m_buffers = new BufferHandler(m_clBasis->getContext());
  m_queue = new Queue(m_clBasis->getContext());
  m_configpath = configpath;
  reload();
  init();
}

vup::ParticleSimulation::~ParticleSimulation()
{
  delete m_clBasis;
  delete m_buffers;
  delete m_queue;
}

void vup::ParticleSimulation::init()
{
  m_queue->acquireGL(m_buffers->getGLBuffers());
  for (std::string &kernel : m_initkernels) {
    m_queue->runRangeKernel(m_kernels->getKernel(kernel), m_kernelSize.at(kernel));
  }
  m_queue->releaseGL(m_buffers->getGLBuffers());
}

void vup::ParticleSimulation::run()
{
  m_queue->acquireGL(m_buffers->getGLBuffers());
  for (std::string &kernel : m_kernelorder) {
    m_queue->runRangeKernel(m_kernels->getKernel(kernel), m_kernelSize.at(kernel));
  }
  m_queue->releaseGL(m_buffers->getGLBuffers());
}

void vup::ParticleSimulation::updateConstant(const char * name, int index, float c)
{
  m_kernels->setArg(name, index, c);
}

void vup::ParticleSimulation::reload()
{
  vup::FileReader configReader(m_configpath);
  std::istringstream f(configReader.getSource());
  std::string kernelpath;
  std::string kerneldatapath;
  std::string particledatapath;
  std::getline(f, kernelpath);
  kernelpath = OPENCL_KERNEL_PATH "/" + kernelpath;
  m_kernelpath = kernelpath.c_str();
  std::getline(f, kerneldatapath);
  kerneldatapath = RESOURCES_PATH "/data/" + kerneldatapath;
  m_kernelinfopath = kerneldatapath;
  std::getline(f, particledatapath);
  particledatapath = RESOURCES_PATH "/data/" + particledatapath;
  m_datapath = particledatapath;

  m_kernelorder.clear();
  m_initkernels.clear();
  m_kernelSize.clear();
  m_buffers->clear();

  m_kernels = std::unique_ptr<KernelHandler>(new KernelHandler(m_clBasis->getContext(), m_clBasis->getDevice(), m_kernelpath));
  KernelInfoLoader kinfLoader(m_kernelinfopath);
  DataLoader dataloader(m_datapath);

  m_size = dataloader.getParticleSize();
  m_particleCount = dataloader.getParticleCount();
  std::map<std::string, vup::ParticleType> types = dataloader.getTypes();
  // All code pushing data to buffers uses the same order as this map to have consistent data in every buffer.
  std::map<std::string, std::map<std::string, vup::ParticleSystem>> systemsByType = dataloader.getSystems();

  // Create buffers for data that all particles share and enqueue data transfer to device.
  typeIdentifiers global = dataloader.getGlobalIdentifiers();
  for (auto &id : global) {
    int offset = 0;
    if (id.second.format == vup::INT) {
      m_buffers->createBuffer<int>(id.first, CL_MEM_READ_WRITE, m_particleCount * id.second.instances);
      for (auto &t : systemsByType) {
        for (auto &s : t.second) {
          m_queue->writeBuffer(m_buffers->getBuffer(id.first), true, offset * sizeof(int), s.second.getCount() * sizeof(int) * id.second.instances, &(s.second.getIntData(id.first))[0]);
          offset += s.second.getCount() * id.second.instances;
        }
      }
    }
    else if (id.second.format == vup::FLOAT) {
      m_buffers->createBuffer<float>(id.first, CL_MEM_READ_WRITE, m_particleCount * id.second.instances);
      for (auto &t : systemsByType) {
        for (auto &s : t.second) {
          m_queue->writeBuffer(m_buffers->getBuffer(id.first), true, offset * sizeof(float), s.second.getCount() * sizeof(float) * id.second.instances, &(s.second.getFloatData(id.first))[0]);
          offset += s.second.getCount() * id.second.instances;
        }
      }
    }
    else if (id.second.format == vup::VEC4) {
      m_buffers->createBuffer<glm::vec4>(id.first, CL_MEM_READ_WRITE, m_particleCount * id.second.instances);
      for (auto &t : systemsByType) {
        for (auto &s : t.second) {
          m_queue->writeBuffer(m_buffers->getBuffer(id.first), true, offset * sizeof(glm::vec4), s.second.getCount() * sizeof(glm::vec4) * id.second.instances, &(s.second.getVec4Data(id.first))[0]);
          offset += s.second.getCount() * id.second.instances;
        }
      }
    }
  }

  // Create VBOs for interoparability data that all particles share and fill them with data.
  // Also create OpenCL buffers that are associated with their VBO.
  typeIdentifiers interop = dataloader.getInteropIdentifiers();
  for (auto &id : interop) {
    int offset = 0;
    // The _vbo suffix is used to distinguish between VBOs and OpenCL buffers.
    if (id.second.format == vup::FLOAT) {
      m_buffers->createVBO<float>(id.first + "_vbo", id.second.loc, m_particleCount * id.second.instances, 1, true, GL_STREAM_DRAW);
      m_buffers->createBufferGL(id.first, CL_MEM_READ_WRITE, id.first + "_vbo");
      for (auto &t : systemsByType) {
        for (auto &s : t.second) {
          m_buffers->updateSubVBO(id.first + "_vbo", s.second.getFloatData(id.first), offset, s.second.getCount() * id.second.instances);
          offset += s.second.getCount() * id.second.instances;
        }
      }
    }
    else if (id.second.format == vup::VEC4) {
      m_buffers->createVBO<glm::vec4>(id.first + "_vbo", id.second.loc, m_particleCount * id.second.instances, 4, true, GL_STREAM_DRAW);
      m_buffers->createBufferGL(id.first, CL_MEM_READ_WRITE, id.first + "_vbo");
      for (auto &t : systemsByType) {
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

  // Create particle type specific buffers and write data to them for all the systems that are of this particular type.
  for (auto &type : types) {
    int typeVarSize = dataloader.getTypeParticleCount(type.first);
    std::map<std::string, vup::ParticleSystem> systems = systemsByType[type.first];
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
    }
  }

  // Create maps for global and type-wise indices for each system.
  int globalOffset = 0;
  for (auto &t : systemsByType) {
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

  // Load the speed up structure data into buffers.
  // Constants are handled separately.
  vup::SpeedupStructure speedup = dataloader.getSpeedupStructure();
  for (auto &speedupdata : speedup.getData()) {
    m_buffers->createBuffer<int>(speedupdata.first, CL_MEM_READ_WRITE, speedupdata.second.size());
    m_queue->writeBuffer(m_buffers->getBuffer(speedupdata.first), speedupdata.second.size() * sizeof(int), &speedupdata.second[0]);
  }

  kernelArgumentsMap arguments = m_kernels->getKernelArguments();
  m_kernelinfos = kinfLoader.getKernelInfos();

  // Initialize all kernels and add them to either the initializing kernels or the execution kernels.
  // All kernels that have multiple executions per simulation step are pushed added to the m_kernelorder at their respective positions.
  for (auto &kinf : m_kernelinfos) {
    if (!kinf.second.init) {
      m_kernelorder.resize(m_kernelorder.size() + kinf.second.pos.size());
    }
  }
  for (auto &kinf : m_kernelinfos) {
    if (!doesKeyExist(kinf.first, arguments)) {
      throw new CorruptDataException("", "Kernel does not exist.");
    }
    m_kernels->initKernel(kinf.first);
    if (!kinf.second.init) {
      for (unsigned int i = 0; i < kinf.second.pos.size(); i++) {
        m_kernelorder.at(kinf.second.pos.at(i)) = kinf.first;
      }
    }
    else {
      m_initkernels.push_back(kinf.first);
    }

    // These are kernel function built-ins of this framework that have to be created separately for
    // every kernel function because the systems and types kernels are executed on can vary from kernel to kernel.
    std::vector<int> kernelTypeIndices;
    std::vector<int> kernelGlobalIndices;
    std::vector<int> kernelTypeIDs;
    std::vector<int> kernelTypeSizes;
    std::vector<int> kernelSystemIDs;
    std::vector<int> kernelSystemSizes;
    int sysID = 0;
    int typeID = 0;
    if (kinf.second.global) {
      for (auto &t : systemsByType) {
        for (auto &s : t.second) {
          int indexCount = s.second.getCount();
          kernelSystemIDs.insert(kernelSystemIDs.end(), indexCount, sysID);
          kernelSystemSizes.insert(kernelSystemSizes.end(), indexCount, indexCount);
          sysID += indexCount;
          // The type and global indices are taken from the system based vector in order to ensure that the right data is pointed at
          // by the newly created index lists.
          kernelTypeIndices.insert(kernelTypeIndices.end(), m_typeIndices[s.first].begin(), m_typeIndices[s.first].end());
          kernelGlobalIndices.insert(kernelGlobalIndices.end(), m_globalIndices[s.first].begin(), m_globalIndices[s.first].end());
        }
        int typeIndexCount = dataloader.getTypeParticleCount(t.first);
        kernelTypeIDs.insert(kernelTypeIDs.end(), typeIndexCount, typeID);
        kernelTypeSizes.insert(kernelTypeSizes.end(), typeIndexCount, typeIndexCount);
        typeID += typeIndexCount;
      }
    }
    else {
      for (auto &t : systemsByType) {
        int typeIndexCount = dataloader.getTypeParticleCount(t.first);
        for (auto &s : t.second) {
          int indexCount = s.second.getCount();
          // Only add a specific type or system if it is mentioned in the kernel's info.
          if (std::find(kinf.second.onTypes.begin(), kinf.second.onTypes.end(), t.first) != kinf.second.onTypes.end() || std::find(kinf.second.onSystems.begin(), kinf.second.onSystems.end(), s.first) != kinf.second.onSystems.end()) {
            // The type and global indices are taken from the system based vector in order to ensure that the right data is pointed at
            // by the newly created index lists.
            kernelTypeIndices.insert(kernelTypeIndices.end(), m_typeIndices[s.first].begin(), m_typeIndices[s.first].end());
            kernelGlobalIndices.insert(kernelGlobalIndices.end(), m_globalIndices[s.first].begin(), m_globalIndices[s.first].end());
            kernelSystemIDs.insert(kernelSystemIDs.end(), indexCount, sysID);
            kernelSystemSizes.insert(kernelSystemSizes.end(), indexCount, indexCount);
            kernelTypeIDs.insert(kernelTypeIDs.end(), indexCount, typeID);
            kernelTypeSizes.insert(kernelTypeSizes.end(), indexCount, typeIndexCount);
          }
          sysID += indexCount;
        }
        typeID += typeIndexCount;
      }
    }

    if (kinf.second.onStructure) {
      m_kernelSize[kinf.first] = speedup.getCount();
    }
    else {
      m_kernelSize[kinf.first] = kernelGlobalIndices.size();
    }

    // The built-in buffers are created for every kernel as the data may be different for each.
    // If this data is not used by the kernel, no buffer will be created and they won't be set.

    for (auto &arg : arguments[kinf.first]) {
      if (arg.name == "globalIndices") {
        m_buffers->createBuffer<int>("globalIndices" + kinf.first, CL_MEM_READ_WRITE, kernelGlobalIndices.size());
        m_queue->writeBuffer(m_buffers->getBuffer("globalIndices" + kinf.first), kernelGlobalIndices.size() * sizeof(int), &kernelGlobalIndices[0]);
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer("globalIndices" + kinf.first));
      }
      else if (arg.name == "typeIndices") {
        m_buffers->createBuffer<int>("typeIndices" + kinf.first, CL_MEM_READ_WRITE, kernelTypeIndices.size());
        m_queue->writeBuffer(m_buffers->getBuffer("typeIndices" + kinf.first), kernelTypeIndices.size() * sizeof(int), &kernelTypeIndices[0]);
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer("typeIndices" + kinf.first));
      }
      else if (arg.name == "systemIDs") {
        m_buffers->createBuffer<int>("systemIDs" + kinf.first, CL_MEM_READ_WRITE, kernelSystemIDs.size());
        m_queue->writeBuffer(m_buffers->getBuffer("systemIDs" + kinf.first), kernelSystemIDs.size() * sizeof(int), &kernelSystemIDs[0]);
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer("systemIDs" + kinf.first));
      }
      else if (arg.name == "systemSizes") {
        m_buffers->createBuffer<int>("systemSizes" + kinf.first, CL_MEM_READ_WRITE, kernelSystemSizes.size());
        m_queue->writeBuffer(m_buffers->getBuffer("systemSizes" + kinf.first), kernelSystemSizes.size() * sizeof(int), &kernelSystemSizes[0]);
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer("systemSizes" + kinf.first));
      }
      else if (arg.name == "typeIDs") {
        m_buffers->createBuffer<int>("typeIDs" + kinf.first, CL_MEM_READ_WRITE, kernelTypeIDs.size());
        m_queue->writeBuffer(m_buffers->getBuffer("typeIDs" + kinf.first), kernelTypeIDs.size() * sizeof(int), &kernelTypeIDs[0]);
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer("typeIDs" + kinf.first));
      }
      else if (arg.name == "typeSizes") {
        m_buffers->createBuffer<int>("typeSizes" + kinf.first, CL_MEM_READ_WRITE, kernelTypeSizes.size());
        m_queue->writeBuffer(m_buffers->getBuffer("typeSizes" + kinf.first), kernelTypeSizes.size() * sizeof(int), &kernelTypeSizes[0]);
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer("typeSizes" + kinf.first));
      }
      else if (arg.constant) {
        identifiers speedupConstantIdentifiers = speedup.getConstantIdentifiers();
        // Constants in kernels either have to be defined in the kernel info
        // or have to be part of the speed up structure.
        // Otherwise they are left empty, which could cause issues.
        if (doesKeyExist(arg.name, kinf.second.constants)) {
          m_kernels->setArg(kinf.first.c_str(), arg.index, kinf.second.constants[arg.name]);
        }
        else if (std::find(speedupConstantIdentifiers.begin(), speedupConstantIdentifiers.end(), arg.name) != speedupConstantIdentifiers.end()) {
          if (arg.type == vup::INT) {
            m_kernels->setArg(kinf.first.c_str(), arg.index, speedup.getIntConstant(arg.name));
          }
          else if (arg.type == vup::FLOAT) {
            m_kernels->setArg(kinf.first.c_str(), arg.index, speedup.getFloatConstant(arg.name));
          }
          else if (arg.type == vup::VEC4) {
            m_kernels->setArg(kinf.first.c_str(), arg.index, speedup.getVec4Constant(arg.name));
          }
        }
        else {
          std::cout << arg.name << " not found." << std::endl;
        }
      }
      else if (doesKeyExist(arg.name, interop)) {
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBufferGL(arg.name));
      }
      else {
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer(arg.name));
      }
      std::cout << "Set " << arg.name << " at " << arg.index << " of " << kinf.first << std::endl;
    }
  }
}

void vup::ParticleSimulation::reloadKernel()
{
  DataLoader dataloader(m_datapath);
  typeIdentifiers interop = dataloader.getInteropIdentifiers();
  vup::SpeedupStructure speedup = dataloader.getSpeedupStructure();
  m_kernels = std::unique_ptr<KernelHandler>(new KernelHandler(m_clBasis->getContext(), m_clBasis->getDevice(), m_kernelpath));
  kernelArgumentsMap arguments = m_kernels->getKernelArguments();
  for (auto &kinf : m_kernelinfos) {
    // Since the kernel handler got replaced, the kernels have to be initialized again.
    m_kernels->initKernel(kinf.first);
    // This code is the same as in the reload() function, but without buffer creation, because the data did not change.
    for (auto &arg : arguments[kinf.first]) {
      if (arg.name == "globalIndices") {
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer("globalIndices" + kinf.first));
      }
      else if (arg.name == "typeIndices") {
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer("typeIndices" + kinf.first));
      }
      else if (arg.name == "systemIDs") {
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer("systemIDs" + kinf.first));
      }
      else if (arg.name == "systemSizes") {
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer("systemSizes" + kinf.first));
      }
      else if (arg.name == "typeIDs") {
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer("typeIDs" + kinf.first));
      }
      else if (arg.name == "typeSizes") {
        m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer("typeSizes" + kinf.first));
      }
      else if (arg.constant) {
        identifiers speedupConstantIdentifiers = speedup.getConstantIdentifiers();
        if (doesKeyExist(arg.name, kinf.second.constants)) {
          m_kernels->setArg(kinf.first.c_str(), arg.index, kinf.second.constants[arg.name]);
        }
        else if (std::find(speedupConstantIdentifiers.begin(), speedupConstantIdentifiers.end(), arg.name) != speedupConstantIdentifiers.end()) {
          if (arg.type == vup::INT) {
            m_kernels->setArg(kinf.first.c_str(), arg.index, speedup.getIntConstant(arg.name));
          }
          else if (arg.type == vup::FLOAT) {
            m_kernels->setArg(kinf.first.c_str(), arg.index, speedup.getFloatConstant(arg.name));
          }
          else if (arg.type == vup::VEC4) {
            m_kernels->setArg(kinf.first.c_str(), arg.index, speedup.getVec4Constant(arg.name));
          }
        }
        else {
          std::cout << "Constant " << arg.name << " not found." << std::endl;
        }
      }
      else {
        if (doesKeyExist(arg.name, interop)) {
          m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBufferGL(arg.name));
        }
        else {
          m_kernels->setArg(kinf.first.c_str(), arg.index, m_buffers->getBuffer(arg.name));
        }
      }
      std::cout << "Set " << arg.name << " at " << arg.index << " of " << kinf.first << std::endl;
    }
  }
}
