// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PARTICLETYPE_H
#define VUP_PARTICLETYPE_H

#include "vup/defs.h"
#include "datadefs.h"
#include "vup/Exceptions/CorruptDataException.h"
#include <map>
#include <algorithm>
#include <iostream>

namespace vup {

class ParticleType {
public:
  ParticleType() { m_name = ""; };
  ParticleType(std::string name, typeIdentifiers typeValues);
  ~ParticleType();
  typeIdentifiers getTypeSpecificIdentifiers() { return m_typeSpecificIdentifiers; }
  //void addConstant(std::string name, datatype t, std::string v);
  void addData(std::string name, vup::DataSpecification t, std::string v);
  /*float getIntConstant(std::string name);
  float getFloatConstant(std::string name);
  glm::vec4 getVec4Constant(std::string name);*/
  datavalue getData(std::string name);
  /*std::map<std::string, int> getIntConstants() { return m_iConsts; }
  std::map<std::string, float> getFloatConstants() { return m_fConsts; }
  std::map<std::string, glm::vec4> getVec4Constants() { return m_vec4Consts; }*/
  datamap getDatasets() { return m_data; }

private:
  std::string m_name;
  identifiers m_identifiers;
  typeIdentifiers m_typeSpecificIdentifiers;
  /*std::map<std::string, int> m_iConsts;
  std::map<std::string, float> m_fConsts;
  std::map<std::string, glm::vec4> m_vec4Consts;*/
  datamap m_data;
  std::vector<std::string> splitVec4(const char* v);

  template <typename T> bool doesKeyExist(std::string key, std::map<std::string, T> m);
};

template<typename T>
inline bool ParticleType::doesKeyExist(std::string key, std::map<std::string, T> m)
{
  std::map<std::string, T>::iterator it = m.find(key);
  return it != m.end();
}

}

#endif
