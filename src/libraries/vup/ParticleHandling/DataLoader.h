// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_DATALOADER_H
#define VUP_DATALOADER_H

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

namespace vup {

// Manages OpenCL and OpenGL buffers and provides 

class DataLoader
{
public:
  DataLoader(const char* path);
  ~DataLoader();
  void load(const char* path);
  int particleAmount() { return m_particleAmount; }
  float getFloatConst(std::string name);
  int getIntConst(std::string name);
  glm::vec4 getVec4Const(std::string name);
  std::vector<float> getFloatDataset(std::string name);
  std::vector<int> getIntDataset(std::string name);
  std::vector<glm::vec4> getVec4Dataset(std::string name);

private:
  const char* m_path;
  int m_particleAmount;

  std::map<std::string, float> m_floatConstants;
  std::map<std::string, glm::vec4> m_vec4Constants;
  std::map<std::string, int> m_intConstants;
  std::map<std::string, std::vector<float>> m_floatDatasets;
  std::map<std::string, std::vector<glm::vec4>> m_vec4Datasets;
  std::map<std::string, std::vector<int>> m_intDatasets;


  std::vector<std::string> tokenizeLine(const char *src, char split);
  void readData(std::string source);
  void handleConstantLine(std::vector<std::string> line, int lineIndex);
  void handleData(std::vector<std::vector<std::string>> data);
  void handleDataset(std::vector<std::string> dataset, int index);

  float createFloatRandom(std::string str);
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

};

template<typename T>
inline bool DataLoader::doesKeyExist(std::string key, std::map<std::string, T> m)
{
  std::map<std::string, T>::iterator it = m.find(key);
  return it != m.end();
}

}

#endif
