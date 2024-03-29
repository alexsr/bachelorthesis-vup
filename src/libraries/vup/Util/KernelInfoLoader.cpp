#include "KernelInfoLoader.h"

vup::KernelInfoLoader::KernelInfoLoader(std::string path)
{
  load(path);
}

vup::KernelInfoLoader::~KernelInfoLoader()
{
}

void vup::KernelInfoLoader::load(std::string path)
{
  m_path = path;
  FileReader fr(m_path);
  if (fr.isLoaded()) {
    std::cout << "Kernel info file is loaded." << std::endl;
    rapidjson::Document doc;
    if (doc.Parse(fr.getSourceChar()).HasParseError() != 0) {
      throw new CorruptDataException(m_path, "Parsing failed!");
    }
    if (!doc.HasMember("kernels") || !doc["kernels"].IsArray()) {
      throw new CorruptDataException(m_path, "No kernels specified!");
    }
    m_iterations = 1;
    if (doc.HasMember("iterations") && doc["iterations"].IsInt()) {
      m_iterations = doc["iterations"].GetInt();
    }
    int pos = 0;
    rapidjson::Value a = doc["kernels"].GetArray();
    // Loop through all kernel info.
    for (rapidjson::Value::ValueIterator it = a.Begin(); it != a.End(); ++it) {
      if (!it->HasMember("name") || !it->IsObject()) {
        throw new CorruptDataException(m_path, "Missing kernel name.");
      }
      rapidjson::Value o = it->GetObject();
      std::string name = o["name"].GetString();
      std::cout << "Loading kernel " << name << "." << std::endl;
      // If a kernel is already defined, every additional occurence just adds an additional
      // kernel execution call at the next position.
      // If a kernel is mentioned twice in the info file, it is executed twice.
      // The execution order of kernels is the same as the order they appear in the info file.
      if (doesKeyExist(name, m_kernelInfos)) {
        m_kernelInfos[name].pos.push_back(pos);
        std::cout << "Next position of " << name << ": " << pos << std::endl;
        pos++;
        continue;
      }
      KernelInfo kinf;
      // If a kernel is an init kernel, it is used to initialize specific values and only run once
      // at the start of the simulation.
      if (o.HasMember("init") && o["init"].IsBool()) {
        kinf.init = o["init"].GetBool();
      }
      else {
        kinf.pos.push_back(pos);
        std::cout << "Position of " << name << ": " << pos << std::endl;
        pos++;
      }
      if (o.HasMember("onStructure") && o["onStructure"].IsBool()) {
        kinf.onStructure = o["onStructure"].GetBool();
      }
      if (!kinf.onStructure) {
        if (!o.HasMember("onSystems") || !o["onSystems"].IsArray()) {
          throw new CorruptDataException(m_path, "Systems kernel is run on have to be declared.");
        }
        if (!o.HasMember("onTypes") || !o["onTypes"].IsArray()) {
          throw new CorruptDataException(m_path, "Types kernel is run on have to be declared.");
        }
        if (o["onSystems"].Size() == 0 && o["onTypes"].Size() == 0) {
          kinf.global = true;
          std::cout << name << " is a global kernel." << std::endl;
        }
        else {
          std::cout << name << " is run on: " << std::endl;
          rapidjson::Value sys = o["onSystems"].GetArray();
          std::cout << "- Systems: ";
          for (rapidjson::Value::ValueIterator sysit = sys.Begin(); sysit != sys.End(); ++sysit) {
            if (!sysit->IsString()) {
              throw new CorruptDataException(m_path, "System kernel names have to be strings.");
            }
            kinf.onSystems.push_back(sysit->GetString());
            std::cout << sysit->GetString() << ", ";
          }
          rapidjson::Value types = o["onTypes"].GetArray();
          std::cout << "\n- Types: ";
          for (rapidjson::Value::ValueIterator typesit = types.Begin(); typesit != types.End(); ++typesit) {
            if (!typesit->IsString()) {
              throw new CorruptDataException(m_path, "Types kernel names have to be strings.");
            }
            kinf.onTypes.push_back(typesit->GetString());
            std::cout << typesit->GetString() << ", ";
          }
        }
      }
      else {
        std::cout << name << " is run on the speedup structure." << std::endl;
      }
      if (o.HasMember("constants") && o["constants"].IsObject()) {
        // Kernel constants are float only.
        std::map<std::string, float> constants;
        std::cout << "\nKernel constants:" << std::endl;
        rapidjson::Value c = o["constants"].GetObject();
        for (rapidjson::Value::ConstMemberIterator cit = c.MemberBegin(); cit != c.MemberEnd(); ++cit) {
          if (cit->value.IsNumber()) {
            constants[cit->name.GetString()] = cit->value.GetFloat();
            std::cout << cit->name.GetString() << " = " << toString(cit->value.GetFloat()) << std::endl;
          }
        }
        kinf.constants = constants;
      }
      m_kernelInfos[name] = kinf;
    }
  }
}
