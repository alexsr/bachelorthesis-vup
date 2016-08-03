// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_KERNELINFOLOADER_H
#define VUP_KERNELINFOLOADER_H

#include "vup/defs.h"
#include "vup/Util/FileReader.h"
#include "vup/Exceptions/CorruptDataException.h"
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

struct KernelInfo {
  std::vector<int> pos = std::vector<int>();
  bool global = false;
  std::vector<std::string> onSystems = std::vector<std::string>();
  std::vector<std::string> onTypes = std::vector<std::string>();
  std::map<std::string, float> constants = std::map<std::string, float>();
  int iterations = 1;
};

class KernelInfoLoader
{
public:
  KernelInfoLoader(const char* path);
  ~KernelInfoLoader();
  std::map<std::string, vup::KernelInfo> getKernelInfos() { return m_kernelInfos; }
  
private:
  void load(const char* path);
  const char* m_path;
  std::map<std::string, vup::KernelInfo> m_kernelInfos;
  template <typename T> bool doesKeyExist(std::string key, std::map<std::string, T> m);
  template <typename T> std::string toString(T any);
};

template<typename T>
inline bool KernelInfoLoader::doesKeyExist(std::string key, std::map<std::string, T> m)
{
  std::map<std::string, T>::iterator it = m.find(key);
  return it != m.end();
}

template<typename T>
inline std::string KernelInfoLoader::toString(T any)
{
  std::ostringstream buff;
  buff << any;
  return buff.str();
}

}

#endif
