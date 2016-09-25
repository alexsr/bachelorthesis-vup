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

// Manages data for a particle system.
// Data is migrated from the particle system's particle type and can also be added later on.
class ParticleSystem {
public:
  ParticleSystem() { m_name = ""; m_count = 0; };
  // Creates a system with a particle count equal to count
  // and automatically adds all default data from the type to the system.
  ParticleSystem(std::string name, int count, ParticleType p);
  ~ParticleSystem();
  // Adds data of name to int data map. Overrides, if it was added prior.
  void addData(std::string name, std::vector<int> d);
  // Adds data of name to float data map. Overrides, if it was added prior.
  void addData(std::string name, std::vector<float> d);
  // Adds data of name to vec4 data map. Overrides, if it was added prior.
  void addData(std::string name, std::vector<glm::vec4> d);
  std::vector<float> getFloatData(std::string key);
  std::vector<int> getIntData(std::string key);
  std::vector<glm::vec4> getVec4Data(std::string key);
  int getCount() { return m_count; }

private:
  // Uses info and values stored in a DataValue struct to add data to the system.
  // The amount of data is given by m_count and d.instances.
  void migrateData(std::string name, vup::DataValue d);
  // Converts a string into a vector with 4 strings, splitting it at ",".
  std::vector<std::string> splitVec4(const char* v);
  template <typename T> bool doesKeyExist(std::string key, std::map<std::string, T> m);

  std::string m_name;
  int m_count;
  identifiers m_identifiers;
  std::map<std::string, std::vector<float>> m_fData;
  std::map<std::string, std::vector<int>> m_iData;
  std::map<std::string, std::vector<glm::vec4>> m_vec4Data;
};

template<typename T>
inline bool ParticleSystem::doesKeyExist(std::string key, std::map<std::string, T> m)
{
  std::map<std::string, T>::iterator it = m.find(key);
  return it != m.end();
}

}

#endif
