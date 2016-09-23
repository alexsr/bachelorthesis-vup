#include "ParticleType.h"

vup::ParticleType::ParticleType(std::string name, typeIdentifiers typeValues)
{
  m_name = name;
  m_typeSpecificIdentifiers = typeValues;
}

vup::ParticleType::~ParticleType()
{
}

void vup::ParticleType::addData(std::string name, vup::DataSpecification t, std::string v)
{
  if (std::find(m_identifiers.begin(), m_identifiers.end(), name) != m_identifiers.end()) {
    throw new CorruptDataException("", "Identifier is already taken.");
  }
  m_identifiers.push_back(name);
  m_data[name] = DataValue(t, v);
}

vup::DataValue vup::ParticleType::getData(std::string name)
{
  if (doesKeyExist(name, m_data)) {
    return m_data.at(name);
  }
  vup::DataSpecification t;
  return DataValue(t, "");
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
