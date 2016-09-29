// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_SPEEDUPSTRUCTURE_H
#define VUP_SPEEDUPSTRUCTURE_H

#include "vup/Core/defs.h"
#include "vup/Core/datadefs.h"
#include "vup/ParticleHandling/ParticleType.h"
#include <map>
#include <iostream>

namespace vup {

// Holds multiple data necessary for speed up structures.
// Objects contain int data because data structures only hold indices.
// Constants are necessary to allow speedup specific kernel arguments.
class SpeedupStructure {
public:
  SpeedupStructure() { m_name = ""; m_count = 0; };
  SpeedupStructure(std::string name, int count);
  ~SpeedupStructure();
  // Adds data to the structure.
  // Checks if data or constant with the same name exists. If it does, an exception is thrown.
  void addData(std::string name, std::vector<int> d);
  // Adds an int constant to the structure.
  // Checks if data or constant with the same name exists. If it does, an exception is thrown.
  void addIntConstant(std::string name, int v);
  // Adds a float constant to the structure.
  // Checks if data or constant with the same name exists. If it does, an exception is thrown.
  void addFloatConstant(std::string name, float v);
  // Adds a vec4 constant to the structure.
  // Checks if data or constant with the same name exists. If it does, an exception is thrown.
  void addVec4Constant(std::string name, glm::vec4 v);
  std::map<std::string, std::vector<int>> getData() { return m_data; }
  int getIntConstant(std::string name);
  float getFloatConstant(std::string name);
  glm::vec4 getVec4Constant(std::string name);
  identifiers getDataIdentifiers() { return m_dataidentifiers; }
  identifiers getConstantIdentifiers() { return m_constantidentifiers; }
  int getCount() { return m_count; }

private:
  template <typename T> bool doesKeyExist(std::string key, std::map<std::string, T> m);
  bool doesKeyExist(std::string key, dataMap m);

  std::string m_name;
  int m_count;
  identifiers m_identifiers;
  identifiers m_dataidentifiers;
  identifiers m_constantidentifiers;
  std::map<std::string, int> m_intConstants;
  std::map<std::string, float> m_floatConstants;
  std::map<std::string, glm::vec4> m_vec4Constants;
  std::map<std::string, std::vector<int>> m_data;
};

template<typename T>
inline bool SpeedupStructure::doesKeyExist(std::string key, std::map<std::string, T> m)
{
  typename std::map<std::string, T>::iterator it = m.find(key);
  return it != m.end();
}

}

#endif
