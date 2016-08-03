#include "ParticleSystem.h"

vup::ParticleSystem::ParticleSystem(std::string name, int count, vup::ParticleType p)
{
  m_name = name;
  m_count = count;
  datamap datasets = p.getDatasets();
  for (auto &entry : datasets) {
    std::string dName = entry.first;
    datavalue d = entry.second;
    migrateData(dName, d.first, d.second);
  }
  
}

vup::ParticleSystem::~ParticleSystem()
{
}

void vup::ParticleSystem::addData(std::string name, std::vector<int> d)
{
  if (std::find(m_identifiers.begin(), m_identifiers.end(), name) != m_identifiers.end()) {
    std::cout << "Overriding type specification of " << name << std::endl;
  }
  m_iData[name] = d;
}

void vup::ParticleSystem::addData(std::string name, std::vector<float> d)
{
  if (std::find(m_identifiers.begin(), m_identifiers.end(), name) != m_identifiers.end()) {
    std::cout << "Overriding type specification of " << name << std::endl;
  }
  m_fData[name] = d;
}

void vup::ParticleSystem::addData(std::string name, vec4data d)
{
  if (std::find(m_identifiers.begin(), m_identifiers.end(), name) != m_identifiers.end()) {
    std::cout << "Overriding type specification of " << name << std::endl;
  }
  m_vec4Data[name] = d;
}

vup::floatdata vup::ParticleSystem::getFloatData(std::string key)
{
  if (doesKeyExist(key, m_fData)) {
    return m_fData[key];
  }
  throw new CorruptDataException("", "Float data does not exist.");
}

std::vector<int> vup::ParticleSystem::getIntData(std::string key)
{
  if (doesKeyExist(key, m_iData)) {
    return m_iData[key];
  }
  throw new CorruptDataException("", "Int data does not exist.");
}

vup::vec4data vup::ParticleSystem::getVec4Data(std::string key)
{
  if (doesKeyExist(key, m_vec4Data)) {
    return m_vec4Data[key];
  }
  throw new CorruptDataException("", "Vec4 data does not exist.");
}

void vup::ParticleSystem::migrateData(std::string name, vup::DataSpecification t, std::string v)
{
  if (std::find(m_identifiers.begin(), m_identifiers.end(), name) != m_identifiers.end()) {
    throw new CorruptDataException("", "Identifier already taken.");
  }
  else {
    if (t.format == FLOAT) {
      std::vector<float> result;
      float value = std::atof(v.c_str());
      for (int i = 0; i < m_count * t.instances; i++) {
        result.push_back(value);
      }
      m_fData[name] = result;
    }
    else if (t.format == INT) {
      std::vector<int> result;
      int value = (int) std::atof(v.c_str());
      for (int i = 0; i < m_count * t.instances; i++) {
        result.push_back(value);
      }
      m_iData[name] = result;
    }
    else if (t.format == VEC4) {
      std::vector<std::string> str = splitVec4(v.c_str());
      std::vector<glm::vec4> result;
      glm::vec4 vec(0);
      for (int i = 0; i < str.size(); i++) {
        vec[i] = std::atof(str[i].c_str());
      }
      for (int i = 0; i < m_count * t.instances; i++) {
        result.push_back(vec);
      }
      m_vec4Data[name] = result;
    }
    std::cout << "Migrated data " << name << "." << std::endl;
  }
}

bool vup::ParticleSystem::doesKeyExist(std::string key, datamap m)
{
  datamap::iterator it = m.find(key);
  return it != m.end();
}

std::vector<std::string> vup::ParticleSystem::splitVec4(const char* v) {
  std::vector<std::string> vec4;
  do {
    const char *begin = v;
    while (*v != ',' && *v) {
      v++;
    }
    vec4.push_back(std::string(begin, v));
  } while (0 != *v++);
  return vec4;
}
