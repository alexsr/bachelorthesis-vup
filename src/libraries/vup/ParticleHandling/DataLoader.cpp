#include "DataLoader.h"

vup::DataLoader::DataLoader(const char* path)
{
  //m_floatConstants = std::map<std::string, float>();
  //m_intConstants = std::map<std::string, int>();
  m_overallParticleCount = 0;
  load(path);
}

vup::DataLoader::~DataLoader()
{
}

void vup::DataLoader::load(const char * path)
{
  /*
  m_floatConstants.clear();
  m_intConstants.clear();*/
  m_path = path;
  FileReader fr(m_path);
  if (fr.isLoaded()) {
    std::cout << "VUP Particle Data file is loaded." << std::endl;
    rapidjson::Document doc;
    if (doc.Parse(fr.getSourceChar()).HasParseError() != 0) {
      throw new CorruptDataException(m_path, "Parsing failed!");
    }
    if (!doc.HasMember("size") || !doc["size"].IsNumber()) {
      throw new CorruptDataException(m_path, "No particle size specified!");
    }
    if (!doc.HasMember("types") || !doc["types"].IsArray()) {
      throw new CorruptDataException(m_path, "No particle types specified!");
    }
    if (!doc.HasMember("systems") || !doc["systems"].IsArray()) {
      throw new CorruptDataException(m_path, "No particle systems specified!");
    }
    if (!doc.HasMember("variables") || !doc["variables"].IsObject()) {
      throw new CorruptDataException(m_path, "Global variables have to be declared.");
    }
    extractVars(doc["variables"].GetObject(), m_globalIdentifiers);
    if (!doc.HasMember("interopVariables") || !doc["interopVariables"].IsObject()) {
      throw new CorruptDataException(m_path, "Interop variables have to be declared.");
    }
    extractVars(doc["interopVariables"].GetObject(), m_interopIdentifiers);
    m_size = doc["size"].GetFloat();
    extractTypes(doc["types"]);
    extractSystems(doc["systems"]);
    if (doc.HasMember("speedupstructure") && doc["speedupstructure"].IsObject()) {
      extractSpeedupStructure(doc["speedupstructure"]);
    }
  }
}

std::map<std::string, vup::ParticleSystem> vup::DataLoader::getSystemsOfType(std::string type)
{
  if (doesKeyExist(type, m_systems)) {
    return m_systems[type];
  }
  throw new CorruptDataException(m_path, "Type not found.");
}

int vup::DataLoader::getTypeParticleCount(std::string type)
{
  if (doesKeyExist(type, m_typeParticleCount)) {
    return m_typeParticleCount[type];
  }
  return 0;
}

void vup::DataLoader::extractTypes(rapidjson::Value &a)
{
  for (rapidjson::Value::ValueIterator it = a.Begin(); it != a.End(); ++it) {
    if (!it->HasMember("name") || !it->IsObject()) {
      throw new CorruptDataException(m_path, "Missing type name.");
    }
    rapidjson::Value o = it->GetObject();
    std::string type = o["name"].GetString();
    if (doesKeyExist(type, m_types)) {
      throw new CorruptDataException(m_path, "Multiple declerations of type " + type + " found.");
    }
    if (!o.HasMember("variables") || !o["variables"].IsObject()) {
      throw new CorruptDataException(m_path, "Type spanning variables have to be declared.");
    }
    typeIdentifiers typeVars;
    extractVars(o["variables"].GetObject(), typeVars);
    vup::ParticleType p(type, typeVars);
    m_typeParticleCount[type] = 0;
    std::cout << "Loading type " << type << std::endl;
    if (o.HasMember("data")) {
      if (!o["data"].IsObject()) {
        throw new CorruptDataException(m_path, "Data has to be a JSON Object");
      }
      std::cout << "Data:" << std::endl;
      rapidjson::Value c = o["data"].GetObject();
      for (rapidjson::Value::ConstMemberIterator cit = c.MemberBegin(); cit != c.MemberEnd(); ++cit) {
        
        vup::DataSpecification spec = getDataSpec(cit->name.GetString(), typeVars);
        if (!cit->value.IsArray()) {
          throw new CorruptDataException(m_path, "Data has to be declared in an array.");
        }
        if (spec.format == vup::INT || spec.format == vup::FLOAT) {
          if (cit->value.GetArray().Size() != 1) {
            throw new CorruptDataException(m_path, "Int data has only one value.");
          }
          std::cout << cit->name.GetString() << " = " << cit->value.GetArray()[0].GetFloat() << std::endl;
          p.addData(cit->name.GetString(), spec, toString(cit->value.GetArray()[0].GetFloat()));
        }
        else if (spec.format == vup::VEC4) {
          std::string vec = "";
          if (cit->value.GetArray().Size() > 4) {
            throw new CorruptDataException(m_path, "Type specific data vectors have to be vec4.");
          }
          rapidjson::SizeType i = 0;
          for (i = 0; i < cit->value.GetArray().Size(); i++) {
            if (!cit->value.GetArray()[i].IsNumber()) {
              throw new CorruptDataException(m_path, "Data array data values have to be numeric.");
            }
            vec += toString(cit->value.GetArray()[i].GetFloat()) + ",";
          }
          while (i < 4) {
            i++;
            vec += "0,";
          }
          vec = vec.substr(0, vec.length() - 1);
          vec += "";
          std::cout << cit->name.GetString() << " = " << vec << std::endl;
          p.addData(cit->name.GetString(), spec, vec);
        }
        else {
          std::cout << "Type not recognized: skipping " << cit->name.GetString() << std::endl;
        }
      }
    }
    m_types[type] = p;
  }
}

void vup::DataLoader::extractSystems(rapidjson::Value &a)
{
  for (rapidjson::Value::ValueIterator it = a.Begin(); it != a.End(); ++it) {
    if (!it->HasMember("name") || !it->IsObject()) {
      throw new CorruptDataException(m_path, "Missing particle system name.");
    }
    if (!it->HasMember("type")) {
      throw new CorruptDataException(m_path, "Missing particle type.");
    }
    if (!it->HasMember("particles")) {
      throw new CorruptDataException(m_path, "No particle count specified.");
    }
    rapidjson::Value o = it->GetObject();
    std::string name = o["name"].GetString();
    std::string type = o["type"].GetString();
    if (!o["particles"].IsInt()) {
      throw new CorruptDataException(m_path, "Particle count should be an integer.");
    }
    if (!doesKeyExist(type, m_types)) {
      throw new CorruptDataException(m_path, "Type " + type + " not found.");
    }
    int count = o["particles"].GetInt();
    m_overallParticleCount += count;
    m_typeParticleCount[type] += count;
    vup::ParticleSystem p(name, count, m_types[type]);
    std::cout << "Loading " << name << ": " << count << " particles of type " << type << std::endl;
    if (o.HasMember("data")) {
      if (!o["data"].IsObject()) {
        throw new CorruptDataException(m_path, "Data has to be a JSON Object");
      }
      std::cout << "Data:" << std::endl;
      rapidjson::Value c = o["data"].GetObject();
      for (rapidjson::Value::ConstMemberIterator cit = c.MemberBegin(); cit != c.MemberEnd(); ++cit) {
        if (cit->value.IsObject()) {
          std::string dataIdentifier = cit->name.GetString();
          rapidjson::Value dataset = c[dataIdentifier.c_str()].GetObject();
          if (!dataset.HasMember("frequency") || !dataset["frequency"].IsInt()) {
            throw new CorruptDataException(m_path, "No frequency specified.");
          }
          if (!dataset.HasMember("values") || !dataset["values"].IsArray()) {
            throw new CorruptDataException(m_path, "No values specified.");
          }
          int frequency = dataset["frequency"].GetInt();
          vup::DataSpecification spec = getDataSpec(dataIdentifier, m_types[type].getTypeSpecificIdentifiers());
          rapidjson::Value values = dataset["values"].GetArray();
          if (spec.format == vup::FLOAT) {
            if (values.Size() <= 0 && frequency == values.Size()) {
              throw new CorruptDataException(m_path, "Wrong number of data in values array.");
            }
            std::vector<float> datavec;
            for (int i = 0; i < count * spec.instances; i++) {
              int f = i % (frequency * spec.instances);
              int offset = f;
              float v = 0.0;
              if (values[offset].IsString()) {
                std::string valuestring = values[offset].GetString();
                if (valuestring.substr(0, 6) == "random") {
                  v = createFloatRandom(valuestring.substr(6).c_str());
                }
                else {
                  throw new CorruptDataException(m_path, "Values cannot be of type string unless they contain random.");
                }
              }
              else if (values[offset].IsNumber()) {
                v = values[offset].GetFloat();
              }
              else {
                throw new CorruptDataException(m_path, "Incorrect type for value.");
              }
              datavec.push_back(v);
            }
            p.addData(dataIdentifier, datavec);
            std::cout << "Data " << dataIdentifier << " loaded." << std::endl;
          }
          else if (spec.format == vup::INT) {
            if (values.Size() <= 0 && frequency == values.Size()) {
              throw new CorruptDataException(m_path, "Wrong number of data in values array.");
            }
            std::vector<int> datavec;
            for (int i = 0; i < count * spec.instances; i++) {
              int f = i % (frequency * spec.instances);
              int offset = f;
              float v = 0.0;
              if (values[offset].IsString()) {
                std::string valuestring = values[offset].GetString();
                if (valuestring.substr(0, 6) == "random") {
                  v = (int) createFloatRandom(valuestring.substr(6).c_str());
                }
                else {
                  throw new CorruptDataException(m_path, "Values cannot be of type string unless they contain random.");
                }
              }
              else if (values[offset].IsNumber()) {
                v = values[offset].GetInt();
              }
              else {
                throw new CorruptDataException(m_path, "Incorrect type for value.");
              }
              datavec.push_back(v);
            }
            p.addData(dataIdentifier, datavec);
            std::cout << "Data " << dataIdentifier << " loaded." << std::endl;
          }
          else if (spec.format == vup::VEC4) {
            if (values.Size() <= 0) {
              throw new CorruptDataException(m_path, "Wrong number of data in values array.");
            }
            std::vector<glm::vec4> datavec;
            for (int i = 0; i < count * spec.instances; i++) {
              int f = i % (frequency * spec.instances);
              int offset = f * 4;
              glm::vec4 result(0);
              for (int h = 0; h < 4; h++) {
                float v = 0.0;
                if (values[offset+h].IsString()) {
                  std::string valuestring = values[offset+h].GetString();
                  if (valuestring.substr(0, 6) == "random") {
                    v = createFloatRandom(valuestring.substr(6).c_str());
                  }
                  else {
                    throw new CorruptDataException(m_path, "Values cannot be of type string unless they contain random.");
                  }
                }
                else if (values[offset+h].IsNumber()) {
                  v = values[offset+h].GetFloat();
                }
                else {
                  throw new CorruptDataException(m_path, "Incorrect type for value.");
                }
                result[h] = v;
              }
              datavec.push_back(result);
            }
            p.addData(dataIdentifier, datavec);
            std::cout << "Data " << dataIdentifier << " loaded." << std::endl;
          }
          else {
            throw new CorruptDataException(m_path, "Data format not supported.");
          }
        }
        else {
          throw new CorruptDataException(m_path, "Datasets have to be a JSON object.");
        }
      }
    }
    m_systems[type][name] = p;
  }
}

void vup::DataLoader::extractSpeedupStructure(rapidjson::Value &o)
{
  if (!o.HasMember("name") || !o.IsObject()) {
    throw new CorruptDataException(m_path, "Missing speedup structure name.");
  }
  if (!o.HasMember("size")) {
    throw new CorruptDataException(m_path, "No structure size specified.");
  }
  std::string name = o["name"].GetString();
  if (!o["size"].IsInt()) {
    throw new CorruptDataException(m_path, "Size should be an integer.");
  }
  int count = o["size"].GetInt();
  m_speedupStructure = vup::SpeedupStructure(name, count);
  std::cout << "Loading speedup structure " << name << ": " << count << " capacity." << std::endl;
  if (!o.HasMember("constants") || !o["constants"].IsObject()) {
    throw new CorruptDataException(m_path, "Constants has to be a JSON Object");
  }
  std::cout << "Constants:" << std::endl;
  rapidjson::Value c = o["constants"].GetObject();
  for (rapidjson::Value::ConstMemberIterator cit = c.MemberBegin(); cit != c.MemberEnd(); ++cit) {
    std::string constantName = cit->name.GetString();
    if (cit->value.IsNumber()) {
      if (cit->value.IsInt()) {
        m_speedupStructure.addIntConstant(constantName, cit->value.GetInt());
        std::cout << constantName << " = " << toString(cit->value.GetInt()) << std::endl;
      }
      else {
        m_speedupStructure.addFloatConstant(constantName, cit->value.GetFloat());
        std::cout << constantName << " = " << toString(cit->value.GetFloat()) << std::endl;
      }

    }
    else if (cit->value.IsArray()) {
      glm::vec4 vec(0.0);
      if (cit->value.GetArray().Size() > 4) {
        throw new CorruptDataException(m_path, "Constanct vectors have to be vec4.");
      }
      for (rapidjson::SizeType i = 0; i < cit->value.GetArray().Size(); i++) {
        if (!cit->value.GetArray()[i].IsNumber()) {
          throw new CorruptDataException(m_path, "Constant array data values have to be numeric.");
        }
        vec[i] = cit->value.GetArray()[i].GetFloat();
      }
      m_speedupStructure.addVec4Constant(constantName, vec);
      std::cout << constantName << " = (" << toString(vec.x) << ", " << toString(vec.y) << ", " << toString(vec.z) << ", " << toString(vec.w) << ")" << std::endl;
    }
    else {
      std::cout << "Type not recognized: skipping " << cit->name.GetString() << std::endl;
    }
  }
  if (!o.HasMember("data") || !o["data"].IsObject()) {
      throw new CorruptDataException(m_path, "Data has to be a JSON Object");
  }
  std::cout << "Data:" << std::endl;
  rapidjson::Value d = o["data"].GetObject();
  for (rapidjson::Value::ConstMemberIterator cit = d.MemberBegin(); cit != d.MemberEnd(); ++cit) {
    if (cit->value.IsInt()) {
      int instances = cit->value.GetInt();
      std::vector<int> data(count * instances, 0);
      m_speedupStructure.addData(cit->name.GetString(), data);
      std::cout << cit->name.GetString() << " with " << instances << " instances." << std::endl;
    }
    else {
      throw new CorruptDataException(m_path, "Speedup data has to hold instance count.");
    }
  }
}

vup::datatype vup::DataLoader::evalDatatype(std::string s)
{
  if (s == "1f") {
    return vup::FLOAT;
  }
  else if (s == "1i") {
    return vup::INT;
  }
  else if (s == "4f") {
    return vup::VEC4;
  }
  else {
    throw new CorruptDataException(m_path, "Data format incorrect.");
  }
}

vup::datatype vup::DataLoader::findFormat(std::string f, typeIdentifiers typeVars)
{
  if (!doesKeyExist(f, typeVars) && !doesKeyExist(f, m_globalIdentifiers) && !doesKeyExist(f, m_interopIdentifiers)) {
    throw new CorruptDataException(m_path, "Data is not specified.");
  }
  if (doesKeyExist(f, typeVars)) {
    return typeVars[f].format;
  }
  else if (doesKeyExist(f, m_globalIdentifiers)) {
    return m_globalIdentifiers[f].format;
  }
  else if (doesKeyExist(f, m_interopIdentifiers)) {
    return m_interopIdentifiers[f].format;
  }
  return vup::EMPTY;
}

vup::DataSpecification vup::DataLoader::getDataSpec(std::string f, typeIdentifiers typeVars)
{
  if (!doesKeyExist(f, typeVars) && !doesKeyExist(f, m_globalIdentifiers) && !doesKeyExist(f, m_interopIdentifiers)) {
    throw new CorruptDataException(m_path, "Data is not specified.");
  }
  if (doesKeyExist(f, typeVars)) {
    return typeVars[f];
  }
  else if (doesKeyExist(f, m_globalIdentifiers)) {
    return m_globalIdentifiers[f];
  }
  else if (doesKeyExist(f, m_interopIdentifiers)) {
    return m_interopIdentifiers[f];
  }
  return vup::DataSpecification();
}

void vup::DataLoader::extractVars(rapidjson::Value o, typeIdentifiers &typeMap)
{
  for (rapidjson::Value::ConstMemberIterator typeIter = o.MemberBegin(); typeIter != o.MemberEnd(); ++typeIter) {
    if (!typeIter->value.IsObject()) {
      throw new CorruptDataException(m_path, "Var specification has to be an object.");
    }
    rapidjson::Value var = o[typeIter->name.GetString()].GetObject();
    if (var.HasMember("loc") && var["loc"].IsInt()) {
      typeMap[typeIter->name.GetString()].loc = var["loc"].GetInt();
    }
    if (!var.HasMember("format") || !var["format"].IsString()) {
      throw new CorruptDataException(m_path, "Variable format has to be a string.");
    }    
    typeMap[typeIter->name.GetString()].format = evalDatatype(var["format"].GetString());
    if (!var.HasMember("instances") || !var["instances"].IsInt()) {
      throw new CorruptDataException(m_path, "Variable instances has to be an int.");
    }
    typeMap[typeIter->name.GetString()].format = evalDatatype(var["format"].GetString());
    typeMap[typeIter->name.GetString()].instances = var["instances"].GetInt();
  }
}

float vup::DataLoader::createFloatRandom(const char* str)
{
  std::vector<std::string> bounds;
  do {
    const char *begin = str;
    while (*str != ',' && *str) {
      str++;
    }
    bounds.push_back(std::string(begin, str));
  } while (0 != *str++);
  if (bounds.size() == 2 && isFloat(bounds.at(0)) && isFloat(bounds.at(1))) {
    float lower = std::atof(bounds.at(0).c_str());
    float upper = std::atof(bounds.at(1).c_str());
    return randomFloat(lower, upper);
  }
  throw new CorruptDataException(m_path, "Random bounds have to be of type float.");
}
