// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PARTICLESYSTEM_H
#define VUP_PARTICLESYSTEM_H

#include "vup/defs.h"
#include "datadefs.h"
#include "vup/ParticleHandling/ParticleType.h"
#include <map>
#include <iostream>

namespace vup {

class ParticleSystem {
public:
  ParticleSystem() { m_name = ""; m_count = 0; };
  ParticleSystem(std::string name, int count, ParticleType p);
  ~ParticleSystem();
  void addData(std::string name, std::vector<int> d);
  void addData(std::string name, std::vector<float> d);
  void addData(std::string name, std::vector<glm::vec4> d);
  std::vector<float> getFloatData(std::string key);
  std::vector<int> getIntData(std::string key);
  std::vector<glm::vec4> getVec4Data(std::string key);
  int getCount() { return m_count; }

private:
  std::string m_name;
  int m_count;
  identifiers m_identifiers;
  std::map<std::string, std::vector<float>> m_fData;
  std::map<std::string, std::vector<int>> m_iData;
  std::map<std::string, std::vector<glm::vec4>> m_vec4Data;

  void migrateData(std::string name, vup::DataValue d);
  template <typename T> bool doesKeyExist(std::string key, std::map<std::string, T> m);
  bool doesKeyExist(std::string key, dataMap m);
  std::vector<std::string> splitVec4(const char* v);
};

template<typename T>
inline bool ParticleSystem::doesKeyExist(std::string key, std::map<std::string, T> m)
{
  std::map<std::string, T>::iterator it = m.find(key);
  return it != m.end();
}

}

#endif
