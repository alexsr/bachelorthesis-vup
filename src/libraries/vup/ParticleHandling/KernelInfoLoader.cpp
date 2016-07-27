#include "DataLoader.h"

vup::DataLoader::DataLoader(const char* path)
{
  //m_floatConstants = std::map<std::string, float>();
  //m_intConstants = std::map<std::string, int>();
  //m_particleAmount = 0;
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
    if (!doc.HasMember("loc") || !doc["loc"].IsInt()) {
      throw new CorruptDataException(m_path, "No interop start location specified!");
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
    for (rapidjson::Value::ConstMemberIterator typeIter = doc["variables"].GetObject().MemberBegin(); typeIter != doc["variables"].GetObject().MemberEnd(); ++typeIter) {
      if (!typeIter->value.IsInt()) {
        throw new CorruptDataException(m_path, "Data format has to be an integer.");
      }
      if (typeIter->value.GetInt() == 1) {
        m_globalIdentifiers[typeIter->name.GetString()] = vup::FLOAT;
      }
      else if (typeIter->value.GetInt() <= 4 && typeIter->value.GetInt() > 1) {
        m_globalIdentifiers[typeIter->name.GetString()] = vup::VEC4;
      }
      else {
        throw new CorruptDataException(m_path, "Data format incorrect.");
      }
    }
    if (!doc.HasMember("interopVariables") || !doc["interopVariables"].IsObject()) {
      throw new CorruptDataException(m_path, "Interop variables have to be declared.");
    }
    for (rapidjson::Value::ConstMemberIterator typeIter = doc["interopVariables"].GetObject().MemberBegin(); typeIter != doc["interopVariables"].GetObject().MemberEnd(); ++typeIter) {
      if (!typeIter->value.IsInt()) {
        throw new CorruptDataException(m_path, "Data format has to be an integer.");
      }
      if (typeIter->value.GetInt() == 1) {
        m_interopIdentifiers[typeIter->name.GetString()] = vup::FLOAT;
      }
      else if (typeIter->value.GetInt() <= 4 && typeIter->value.GetInt() > 1) {
        m_interopIdentifiers[typeIter->name.GetString()] = vup::VEC4;
      }
      else {
        throw new CorruptDataException(m_path, "Data format incorrect.");
      }
    }
    m_size = doc["size"].GetFloat();
    m_loc = doc["loc"].GetInt();
    extractTypes(doc["types"]);
    extractSystems(doc["systems"]);
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
    for (rapidjson::Value::ConstMemberIterator typeIter = o["variables"].GetObject().MemberBegin(); typeIter != o["variables"].GetObject().MemberEnd(); ++typeIter) {
      if (!typeIter->value.IsInt()) {
        throw new CorruptDataException(m_path, "Data format has to be an integer.");
      }
      if (typeIter->value.GetInt() == 1) {
        typeVars[typeIter->name.GetString()] = vup::FLOAT;
      }
      else if (typeIter->value.GetInt() <= 4 && typeIter->value.GetInt() > 1) {
        typeVars[typeIter->name.GetString()] = vup::VEC4;
      }
      else {
        throw new CorruptDataException(m_path, "Data format incorrect.");
      }
    }
    vup::ParticleType p(type, typeVars);
    m_typeParticleCount[type] = 0;
    std::cout << "Loading type " << type << std::endl;
    if (o.HasMember("constants")) {
      if (!o["constants"].IsObject()) {
        throw new CorruptDataException(m_path, "Constants has to be a JSON Object");
      }
      std::cout << "Constants:" << std::endl;
      rapidjson::Value c = o["constants"].GetObject();
      for (rapidjson::Value::ConstMemberIterator cit = c.MemberBegin(); cit != c.MemberEnd(); ++cit) {
        if (cit->value.IsNumber()) {
          p.addConstant(cit->name.GetString(), vup::datatype::FLOAT, toString(cit->value.GetFloat()));
          std::cout << cit->name.GetString() << " = " << toString(cit->value.GetFloat()) << std::endl;
        }
        else if (cit->value.IsArray()) {
          std::string vec = "";
          if (cit->value.GetArray().Size() > 4) {
            throw new CorruptDataException(m_path, "Constanct vectors have to be vec4.");
          }
          rapidjson::SizeType i = 0;
          for (i = 0; i < cit->value.GetArray().Size(); i++) {
            if (!cit->value.GetArray()[i].IsNumber()) {
              throw new CorruptDataException(m_path, "Constant array data values have to be numeric.");
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
          p.addConstant(cit->name.GetString(), vup::datatype::VEC4, vec);
        }
        else {
          std::cout << "Type not recognized: skipping " << cit->name.GetString() << std::endl;
        }
      }
    }
    if (o.HasMember("data")) {
      if (!o["data"].IsObject()) {
        throw new CorruptDataException(m_path, "Data has to be a JSON Object");
      }
      std::cout << "Data:" << std::endl;
      rapidjson::Value c = o["data"].GetObject();
      for (rapidjson::Value::ConstMemberIterator cit = c.MemberBegin(); cit != c.MemberEnd(); ++cit) {
        if (cit->value.IsNumber()) {
          p.addData(cit->name.GetString(), vup::datatype::FLOAT, toString(cit->value.GetFloat()));
          std::cout << cit->name.GetString() << " = " << toString(cit->value.GetFloat()) << std::endl;
        }
        else if (cit->value.IsArray()) {
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
          p.addData(cit->name.GetString(), vup::datatype::VEC4, vec);
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
    m_typeParticleCount[type] += count;
    vup::ParticleSystem p(name, count, m_types[type]);
    std::cout << "Loading " << name << ": " << count << " particles of type " << type << std::endl;
   /* if (o.HasMember("constants")) {
      if (!o["constants"].IsObject()) {
        throw new CorruptDataException(m_path, "Constants has to be a JSON Object");
      }
      std::cout << "Constants:" << std::endl;
      rapidjson::Value c = o["constants"].GetObject();
      for (rapidjson::Value::ConstMemberIterator cit = c.MemberBegin(); cit != c.MemberEnd(); ++cit) {
        if (cit->value.IsNumber()) {
          p.migrateConstant(cit->name.GetString(), vup::datatype::FLOAT, toString(cit->value.GetFloat()));
          std::cout << cit->name.GetString() << " = " << toString(cit->value.GetFloat()) << std::endl;
        }
        else if (cit->value.IsArray()) {
          std::string vec = "";
          if (cit->value.GetArray().Size() > 4) {
            throw new CorruptDataException(m_path, "Constanct vectors have to be vec4.");
          }
          rapidjson::SizeType i = 0;
          for (i = 0; i < cit->value.GetArray().Size(); i++) {
            if (!cit->value.GetArray()[i].IsNumber()) {
              throw new CorruptDataException(m_path, "Constant array data values have to be numeric.");
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
          p.migrateConstant(cit->name.GetString(), vup::datatype::VEC4, vec);
        }
        else {
          std::cout << "Type not recognized: skipping " << cit->name.GetString() << std::endl;
        }
      }
    }*/
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
          if (!dataset.HasMember("format") || !dataset["format"].IsInt()) {
            throw new CorruptDataException(m_path, "No format specified.");
          }
          if (!dataset.HasMember("values") || !dataset["values"].IsArray()) {
            throw new CorruptDataException(m_path, "No values specified.");
          }
          int frequency = dataset["frequency"].GetInt();
          int format = dataset["format"].GetInt();
          rapidjson::Value values = dataset["values"].GetArray();
          if (values.Size() <= 0 && frequency * format == values.Size()) {
            throw new CorruptDataException(m_path, "Wrong number of data in values array.");
          }
          if (format == 1) {
            std::vector<float> datavec;
            for (int i = 0; i < count; i++) {
              int f = i % frequency;
              int offset = f * format;
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
            std::cout << dataIdentifier << ": [";
            for (float x : datavec) {
              std::cout << x << ",";
            }
            std::cout << "]" << std::endl;
          }
          else if (format <= 4 && format > 0) {
            std::vector<glm::vec4> datavec;
            for (int i = 0; i < count; i++) {
              int f = i % frequency;
              int offset = f * format;
              glm::vec4 result(0);
              for (int h = 0; h < format; h++) {
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
            std::cout << dataIdentifier << ": [";
            for (glm::vec4 x : datavec) {
              std::cout << "(" << x.x << "," << x.y << "," << x.z << "," << x.w << "),";
            }
            std::cout << "]" << std::endl;
          }
          else {
            throw new CorruptDataException(m_path, "Data format not supported.");
          }
          /*if (cit->value.IsObject()) {
            p.addData(cit->name.GetString(), vup::datatype::FLOAT, toString(cit->value.GetFloat()));
            std::cout << cit->name.GetString() << " = " << toString(cit->value.GetFloat()) << std::endl;
          }
          else if (cit->value.IsArray()) {
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
            p.addData(cit->name.GetString(), vup::datatype::VEC4, vec);*/
        }
        else {
          throw new CorruptDataException(m_path, "Datasets have to be a JSON object.");
        }
      }
    }
    m_systems[type][name] = p;
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
