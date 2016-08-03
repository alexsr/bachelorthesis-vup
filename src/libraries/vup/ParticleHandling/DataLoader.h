// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_DATALOADER_H
#define VUP_DATALOADER_H

#include "vup/defs.h"
#include "datadefs.h"
#include "vup/Util/FileReader.h"
#include "vup/Exceptions/CorruptDataException.h"
#include "vup/ParticleHandling/ParticleSystem.h"
#include "vup/ParticleHandling/ParticleType.h"
#include <vector>
#include <map>
#include <string>
#include <iterator>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include "rapidjson/document.h"

namespace vup {

// Manages OpenCL and OpenGL buffers and provides 

class DataLoader
{
public:
  DataLoader(const char* path);
  ~DataLoader();
  void load(const char* path);
  float getParticleSize() { return m_size; }
  std::map<std::string, vup::ParticleType> getTypes() { return m_types; }
  std::map<std::string, std::map<std::string, vup::ParticleSystem>> getSystems() { return m_systems; }
  std::map<std::string, vup::ParticleSystem> getSystemsOfType(std::string type);
  typeIdentifiers getGlobalIdentifiers() { return m_globalIdentifiers; }
  typeIdentifiers getInteropIdentifiers() { return m_interopIdentifiers; }
  int getParticleCount() { return m_overallParticleCount; }
  std::map<std::string, int> getTypeParticleCounts() { return m_typeParticleCount; }
  int getTypeParticleCount(std::string type);

private:
  const char* m_path;
  float m_size;
  int m_overallParticleCount;
  std::map<std::string, vup::ParticleType> m_types;
  std::map<std::string, int> m_typeParticleCount;
  std::map<std::string, std::map<std::string, vup::ParticleSystem>> m_systems;
  typeIdentifiers m_globalIdentifiers;
  typeIdentifiers m_interopIdentifiers;
  void extractTypes(rapidjson::Value &a);
  void extractSystems(rapidjson::Value &a);

  datatype evalDatatype(std::string s);
  datatype findFormat(std::string f, typeIdentifiers typeVars);
  vup::DataSpecification getDataSpec(std::string f, typeIdentifiers typeVars);
  void extractVars(rapidjson::Value o, typeIdentifiers &typeMap);

  float createFloatRandom(const char* str);
  float randomFloat(float lower, float upper) {
    return lower + (upper-lower) * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
  }

  bool isFloat(std::string str) {
    std::istringstream iss(str);
    double d;
    char c;
    return iss >> d && !(iss >> c);
  }
  //bool isInt(std::string str) {
  //  std::istringstream iss(str);
  //  int i;
  //  char c;
  //  return iss >> i && !(iss >> c);
  //}
  template <typename T> bool doesKeyExist(std::string key, std::map<std::string, T> m);
  template <typename T> std::string toString(T any);
};

template<typename T>
inline bool DataLoader::doesKeyExist(std::string key, std::map<std::string, T> m)
{
  std::map<std::string, T>::iterator it = m.find(key);
  return it != m.end();
}

template<typename T>
inline std::string DataLoader::toString(T any)
{
  std::ostringstream buff;
  buff << any;
  return buff.str();
}

}

#endif
