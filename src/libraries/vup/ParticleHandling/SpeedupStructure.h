// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_SPEEDUPSTRUCTURE_H
#define VUP_SPEEDUPSTRUCTURE_H

#include "vup/defs.h"
#include "datadefs.h"
#include "vup/ParticleHandling/ParticleType.h"
#include <map>
#include <iostream>

namespace vup {

class SpeedupStructure {
public:
  SpeedupStructure() { m_name = ""; m_count = 0; };
  SpeedupStructure(std::string name, int count);
  ~SpeedupStructure();
  void addData(std::string name, std::vector<int> d);
  std::map<std::string, std::vector<int>> getData();
  void addIntConstant(std::string name, int v);
  void addFloatConstant(std::string name, float v);
  void addVec4Constant(std::string name, glm::vec4 v);
  int getIntConstant(std::string name);
  float getFloatConstant(std::string name);
  glm::vec4 getVec4Constant(std::string name);
  identifiers getDataIdentifiers() { return m_dataidentifiers; }
  identifiers getConstantIdentifiers() { return m_constantidentifiers; }
  int getCount() { return m_count; }

private:
  std::string m_name;
  int m_count;
  identifiers m_identifiers;
  identifiers m_dataidentifiers;
  identifiers m_constantidentifiers;
  std::map<std::string, int> m_intConstants;
  std::map<std::string, float> m_floatConstants;
  std::map<std::string, glm::vec4> m_vec4Constants;
  std::map<std::string, std::vector<int>> m_data;

  template <typename T> bool doesKeyExist(std::string key, std::map<std::string, T> m);
  bool doesKeyExist(std::string key, dataMap m);
};

template<typename T>
inline bool SpeedupStructure::doesKeyExist(std::string key, std::map<std::string, T> m)
{
  std::map<std::string, T>::iterator it = m.find(key);
  return it != m.end();
}

}

#endif
