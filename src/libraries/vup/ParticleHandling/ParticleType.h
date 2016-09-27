// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PARTICLETYPE_H
#define VUP_PARTICLETYPE_H

#include "vup/Core/defs.h"
#include "vup/Core/datadefs.h"
#include "vup/Exceptions/CorruptDataException.h"
#include <map>
#include <algorithm>
#include <iostream>

namespace vup {

// Holds type default data and its specification. Data is not only type specific but also global and interop.
class ParticleType {
public:
  ParticleType() { m_name = ""; };
  // Creates a particle type with its default values specifications.
  ParticleType(std::string name, typeIdentifiers typeValues);
  ~ParticleType();
  typeIdentifiers getTypeSpecificIdentifiers() { return m_typeSpecificIdentifiers; }
  // Adds default data specification and data if it was not added already.
  void addData(std::string name, vup::DataSpecification t, std::string v);
  DataValue getData(std::string name);
  dataMap getDatasets() { return m_data; }

private:
  std::vector<std::string> splitVec4(const char* v);
  template <typename T> bool doesKeyExist(std::string key, std::map<std::string, T> m);

  std::string m_name;
  // Identifiers for added data.
  identifiers m_identifiers;
  // Identifiers for type data only.
  typeIdentifiers m_typeSpecificIdentifiers;
  dataMap m_data;
};

template<typename T>
inline bool ParticleType::doesKeyExist(std::string key, std::map<std::string, T> m)
{
  std::map<std::string, T>::iterator it = m.find(key);
  return it != m.end();
}

}

#endif
