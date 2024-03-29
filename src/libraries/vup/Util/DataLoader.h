// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_DATALOADER_H
#define VUP_DATALOADER_H

#include "vup/Core/defs.h"
#include "vup/Core/datadefs.h"
#include "vup/Util/FileReader.h"
#include "vup/Exceptions/CorruptDataException.h"
#include "vup/ParticleHandling/ParticleSystem.h"
#include "vup/ParticleHandling/ParticleType.h"
#include "vup/ParticleHandling/SpeedupStructure.h"
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

// Loads particle data from a JSON file using rapidjson http://rapidjson.org/
// Creates a map of particle types containing a map of ParticleSystem objects of its type.
// Also loads the speedup structure data into a SpeedupStructure object.
class DataLoader
{
public:
  DataLoader(std::string path);
  ~DataLoader();
  // Loads file from path and checks if it is valid JSON.
  // If it is valid, it is parsed, otherwise a CorruptDataException is thrown.
  // Checks for predefined data and structures to parse further.
  // Global and interop variables are extracted.
  // Types and systems are created.
  // If a speedup structure is defined, it is extracted as well.
  void load(std::string path);

  float getParticleSize() { return m_size; }
  std::map<std::string, vup::ParticleType> getTypes() { return m_types; }
  std::map<std::string, std::map<std::string, vup::ParticleSystem>> getSystems() { return m_systems; }
  std::map<std::string, vup::ParticleSystem> getSystemsOfType(std::string type);
  typeIdentifiers getGlobalIdentifiers() { return m_globalIdentifiers; }
  typeIdentifiers getInteropIdentifiers() { return m_interopIdentifiers; }
  int getParticleCount() { return m_overallParticleCount; }
  std::map<std::string, int> getTypeParticleCounts() { return m_typeParticleCount; }
  int getTypeParticleCount(std::string type);
  vup::SpeedupStructure getSpeedupStructure() { return m_speedupStructure; }

private:
  // Extracts the specifications of all variables in the json object o and put these in the passed typeMap.
  void extractVars(rapidjson::Value &o, typeIdentifiers &typeMap);
  // Extracts the specifications of all variables in the json object o and put these in the passed typeMap.
  void extractTypes(rapidjson::Value &a);
  // Extracts the data for the system. The variables have to conform to global, interop or type variables.
  // Creates a ParticleSystem object from its ParticleType object.
  void extractSystems(rapidjson::Value &a);
  // Extracts the speedup structure data and constants and creates a SpeedupStructure objects from it.
  void extractSpeedupStructure(rapidjson::Value &o);

  // Returns a data type corresponding to s. If there is no data type specified, an exception is thrown.
  datatype evalDatatype(std::string s);
  // Returns the format of a variable that may be in m_globalIdentifiers, m_interopIdentifiers or in typeVars.
  datatype findFormat(std::string f, typeIdentifiers &typeVars);
  // Looks for the variable f in m_globalIdentifiers, m_interopIdentifiers or in typeVars and return its specs
  vup::DataSpecification getDataSpec(std::string f, typeIdentifiers typeVars);
  // Creates a float from a string in the format of "randomX,Y" with X and Y being any kind of float.
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
  bool isInt(std::string str) {
    std::istringstream iss(str);
    int i;
    char c;
    return iss >> i && !(iss >> c);
  }
  template <typename T> bool doesKeyExist(std::string key, std::map<std::string, T> m);
  template <typename T> std::string toString(T any);

  std::string m_path;
  // size of the particle representation
  float m_size;
  int m_overallParticleCount;
  std::map<std::string, vup::ParticleType> m_types;
  std::map<std::string, int> m_typeParticleCount;
  // A map of systems in a map with type names as keys
  std::map<std::string, std::map<std::string, vup::ParticleSystem>> m_systems;
  vup::SpeedupStructure m_speedupStructure;
  typeIdentifiers m_globalIdentifiers;
  typeIdentifiers m_interopIdentifiers;
};

template<typename T>
inline bool DataLoader::doesKeyExist(std::string key, std::map<std::string, T> m)
{
  typename std::map<std::string, T>::iterator it = m.find(key);
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
