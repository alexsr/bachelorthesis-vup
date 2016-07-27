#include "ParticleType.h"

vup::ParticleType::ParticleType(std::string name, typeIdentifiers typeValues)
{
  m_name = name;
  m_typeSpecificIdentifiers = typeValues;
}

vup::ParticleType::~ParticleType()
{
}

void vup::ParticleType::addConstant(std::string name, datatype t, std::string v)
{
  if (std::find(m_identifiers.begin(), m_identifiers.end(), name) != m_identifiers.end()) {
    throw new CorruptDataException("", "Identifier is already taken.");
  }
  m_identifiers.push_back(name);
  if (t == FLOAT) {
    m_fConsts[name] = std::atof(v.c_str());
  }
  else if (t == INT) {
    m_iConsts[name] = std::atoi(v.c_str());
  }
  else if (t == VEC4) {
    std::vector<std::string> vec = splitVec4(v.c_str());
    glm::vec4 result(0);
    for (int i = 0; i < vec.size(); i++) {
      result[i] = std::atof(vec[i].c_str());
    }
    m_vec4Consts[name] = result;
  }
}

void vup::ParticleType::addData(std::string name, datatype t, std::string v)
{
  if (std::find(m_identifiers.begin(), m_identifiers.end(), name) != m_identifiers.end()) {
    throw new CorruptDataException("", "Identifier is already taken.");
  }
  m_identifiers.push_back(name);
  m_data[name] = std::make_pair(t, v);
}

float vup::ParticleType::getFloatConstant(std::string name)
{
  if (doesKeyExist(name, m_fConsts)) {
    return m_fConsts.at(name);
  }
  return 0;
}

glm::vec4 vup::ParticleType::getVec4Constant(std::string name)
{
  if (doesKeyExist(name, m_vec4Consts)) {
    return m_vec4Consts.at(name);
  }
  return glm::vec4(0.0);
}

vup::datavalue vup::ParticleType::getData(std::string name)
{
  if (doesKeyExist(name, m_data)) {
    return m_data.at(name);
  }
  return std::pair<datatype, std::string>(EMPTY, "");
}

std::vector<std::string> vup::ParticleType::splitVec4(const char* v) {
  std::vector<std::string> vec4;
  do {
    const char *begin = v;
    while (*v != ',' && *v) {
      v++;
    }
    vec4.push_back(std::string(begin, v));
  } while (0 != *v++);
  return vec4;
}
