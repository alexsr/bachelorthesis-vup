#include "SpeedupStructure.h"

vup::SpeedupStructure::SpeedupStructure(std::string name, int count)
{
  m_name = name;
  m_count = count;  
}

vup::SpeedupStructure::~SpeedupStructure()
{
}

void vup::SpeedupStructure::addData(std::string name, std::vector<int> d)
{
  if (std::find(m_identifiers.begin(), m_identifiers.end(), name) != m_identifiers.end()) {
    std::cout << "Overriding type specification of " << name << std::endl;
  }
  else {
    m_identifiers.push_back(name);
    m_dataidentifiers.push_back(name);
  }
  m_data[name] = d;
}

void vup::SpeedupStructure::addIntConstant(std::string name, int v)
{
  if (std::find(m_identifiers.begin(), m_identifiers.end(), name) != m_identifiers.end()) {
    std::cout << "Overriding type specification of " << name << std::endl;
  }
  else {
    m_identifiers.push_back(name);
    m_constantidentifiers.push_back(name);
  }
  m_intConstants[name] = v;
}

void vup::SpeedupStructure::addFloatConstant(std::string name, float v)
{
  if (std::find(m_identifiers.begin(), m_identifiers.end(), name) != m_identifiers.end()) {
    std::cout << "Overriding type specification of " << name << std::endl;
  }
  else {
    m_identifiers.push_back(name);
    m_constantidentifiers.push_back(name);
  }
  m_floatConstants[name] = v;
}

void vup::SpeedupStructure::addVec4Constant(std::string name, glm::vec4 v)
{
  if (std::find(m_identifiers.begin(), m_identifiers.end(), name) != m_identifiers.end()) {
    std::cout << "Overriding type specification of " << name << std::endl;
  }
  else {
    m_identifiers.push_back(name);
    m_constantidentifiers.push_back(name);
  }
  m_vec4Constants[name] = v;
}

int vup::SpeedupStructure::getIntConstant(std::string name)
{
  if (doesKeyExist(name, m_intConstants)) {
    return m_intConstants[name];
  }
  throw new CorruptDataException("", "Int constant does not exist.");
}

float vup::SpeedupStructure::getFloatConstant(std::string name)
{
  if (doesKeyExist(name, m_floatConstants)) {
    return m_floatConstants[name];
  }
  throw new CorruptDataException("", "Float constant does not exist.");
}

glm::vec4 vup::SpeedupStructure::getVec4Constant(std::string name)
{
  if (doesKeyExist(name, m_vec4Constants)) {
    return m_vec4Constants[name];
  }
  throw new CorruptDataException("", "Vec4 constant does not exist.");
}

bool vup::SpeedupStructure::doesKeyExist(std::string key, dataMap m)
{
  dataMap::iterator it = m.find(key);
  return it != m.end();
}
