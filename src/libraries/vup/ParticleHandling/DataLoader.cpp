#include "DataLoader.h"

vup::DataLoader::DataLoader(const char* path)
{
  m_floatConstants = std::map<std::string, float>();
  m_intConstants = std::map<std::string, int>();
  m_particleAmount = 0;
  load(path);
}

vup::DataLoader::~DataLoader()
{
}

void vup::DataLoader::load(const char * path)
{
  m_floatConstants.clear();
  m_intConstants.clear();
  m_path = path;
  FileReader fr(m_path);
  if (fr.isLoaded()) {
    std::cout << "VUP Particle Data file is loaded." << std::endl;
    std::string source = fr.getSource();
    readData(source);
  }
}

float vup::DataLoader::getFloatConst(std::string name)
{
  if (doesKeyExist(name, m_floatConstants)) {
    return m_floatConstants[name];
  }
  std::cout << "WARNING: " << name << " not found by data loader." << std::endl;
  return 0;
}

int vup::DataLoader::getIntConst(std::string name)
{
  if (doesKeyExist(name, m_intConstants)) {
    return m_intConstants[name];
  }
  std::cout << "WARNING: " << name << " not found by data loader." << std::endl;
  return 0;
}

glm::vec4 vup::DataLoader::getVec4Const(std::string name)
{
  if (doesKeyExist(name, m_vec4Constants)) {
    return m_vec4Constants[name];
  }
  std::cout << "WARNING: " << name << " not found by data loader." << std::endl;
  return glm::vec4(0.0);
}

std::vector<float> vup::DataLoader::getFloatDataset(std::string name)
{
  if (doesKeyExist(name, m_floatDatasets)) {
    return m_floatDatasets[name];
  }
  std::cout << "WARNING: " << name << " not found by data loader." << std::endl;
  return std::vector<float>(0);
}

std::vector<int> vup::DataLoader::getIntDataset(std::string name)
{
  if (doesKeyExist(name, m_intDatasets)) {
    return m_intDatasets[name];
  }
  std::cout << "WARNING: " << name << " not found by data loader." << std::endl;
  return std::vector<int>(0);
}

std::vector<glm::vec4> vup::DataLoader::getVec4Dataset(std::string name)
{
  if (doesKeyExist(name, m_vec4Datasets)) {
    return m_vec4Datasets[name];
  }
  std::cout << "WARNING: " << name << " not found by data loader." << std::endl;
  return std::vector<glm::vec4>(0);
}

std::vector<std::string> vup::DataLoader::tokenizeLine(const char * src, char split)
{
  std::vector<std::string> line;
  do
  {
    const char *begin = src;

    while (*src != split && *src)
      src++;
    line.push_back(std::string(begin, src));
  } while (0 != *src++);
  return line;
}

void vup::DataLoader::readData(std::string source)
{
  std::istringstream lineStream(source);
  std::string line;
  std::vector<std::vector<std::string>> tokenizedLines;
  std::getline(lineStream, line);
  if (line != "VUP PARTICLE DATA") {
    throw vup::CorruptDataException(m_path, "Missing file header.");
  }
  while (std::getline(lineStream, line)) {
    tokenizedLines.push_back(tokenizeLine(line.c_str(), ' '));
  }
  if (tokenizedLines.size() <= 0) {
    throw vup::CorruptDataException(m_path, "No data in file.");
  }
  std::vector<std::string> tknLine = tokenizedLines.at(0);
  if (tknLine.size() == 1 && isInt(tknLine.at(0))) {
    m_particleAmount = std::atoi(tknLine.at(0).c_str());
  }
  else {
    throw vup::CorruptDataException(m_path, "Only particle number allowed in line 1.", 1);
  }
  std::cout << "Particle amount: " << m_particleAmount << std::endl;
  int i = 1;
  if (tokenizedLines.size() > i) {
    tknLine = tokenizedLines.at(i);
  }
  else {
    return;
  }
  while (tknLine.at(0) == "") {
    i++;
    if (tokenizedLines.size() > i) {
      tknLine = tokenizedLines.at(i);
    }
    else {
      return;
    }
  }
  if (tknLine.at(0) == "CONSTANTS") {
    i++;
    while (tokenizedLines.at(i).size() > 1 && tokenizedLines.size() > i) {
      handleConstantLine(tokenizedLines.at(i), i);
      i++;
    }
  }
  if (tokenizedLines.size() > i) {
    if (tokenizedLines.at(i).at(0) == "DATA") {
      handleData(std::vector<std::vector<std::string>>(tokenizedLines.begin() + i + 1, tokenizedLines.end()));
    }
  }
  else {
    return;
  }
  std::cout << std::endl;
}

void vup::DataLoader::handleConstantLine(std::vector<std::string> line, int lineIndex)
{
  if (line.size() < 3) {
    std::cout << "DATA WARNING AT " << lineIndex << ": Constant handling failed." << std::endl;
    return;
  }
  std::string name = line.at(0);
  std::string type = line.at(1);
  if (type == "1f") {
    if (doesKeyExist(name, m_floatConstants)) {
      std::cout << "WARNING: " << name << " is already a float constant." << std::endl;
      return;
    }
    if (isFloat(line.at(2))) {
      float value = std::atof(line.at(2).c_str());
      m_floatConstants[name] = value;
      std::cout << "Constant " << name << " loaded: " << value << std::endl;
    }
    else {
      std::cout << "WARNING: " << line.at(2) << " is not a float." << std::endl;
    }
  }
  else if (type == "4f") {
    if (doesKeyExist(name, m_vec4Constants)) {
      std::cout << "WARNING: " << name << " is already a vec4 constant." << std::endl;
      return;
    }
    if (line.size() == 6) {
      glm::vec4 result(0.0, 0.0, 0.0, 0.0);
      for (int i = 2; i < 6; i++) {
        if (isFloat(line.at(i))) {
          result[i-2] = std::atof(line.at(i).c_str());
        }
        else {
          std::cout << "WARNING: " << line.at(i) << " is not a float." << std::endl;
        }
      }
      m_vec4Constants[name] = result;
      std::cout << "Constant " << name << " loaded: " << result.x << ", " << result.y << ", " << result.z << ", " << result.w << ", " << std::endl;
    }
    else {
      std::cout << "WARNING: Too few values in line " << lineIndex << "." << std::endl;
    }
  }
  else if (type == "1i") {
    if (doesKeyExist(name, m_intConstants)) {
      std::cout << "WARNING: " << name << " is already a int constant." << std::endl;
      return;
    }
    if (isInt(line.at(2))) {
      m_floatConstants[name] = std::atoi(line.at(2).c_str());
    }
    else {
      std::cout << "WARNING: " << line.at(2) << " is not an int." << std::endl;
    }
  }
  else {
    std::cout << "TYPE " << type << " NOT RECOGNIZED AT " << lineIndex << "." << std::endl;
  }
}

void vup::DataLoader::handleData(std::vector<std::vector<std::string>> data)
{
  srand(static_cast <unsigned> (time(0)));
  if (data.size() == 0) {
    std::cout << "NO DATA SPECIFIED!" << std::endl;
    return;
  }
  std::vector<std::vector<std::string>> sortedData;
  int n = 0;
  std::vector<std::string> tknLine = data.at(n);
  while (tknLine.at(0) == "") {
    n++;
    if (data.size() > n) {
      tknLine = data.at(n);
    }
    else {
      return;
    }
  }
  sortedData.push_back(data.at(n));
  int s = 1;
  for (int i = n+1; i < data.size(); i++) {
    tknLine = data.at(i);
    if (tknLine.size() == 0 || tknLine.at(0) == "") {
      continue;
    }
    if (!isFloat(tknLine.at(0)) && !isInt(tknLine.at(0))) {
      sortedData.push_back(data.at(i));
      s++;
    }
    else {
      sortedData.at(s - 1).insert(sortedData.at(s - 1).end(), tknLine.begin(), tknLine.end());
    }
  }
  for (int i = 0; i < sortedData.size(); i++) {
    std::vector<std::string> dataset = sortedData.at(i);
    handleDataset(dataset, i);
  }
}
void vup::DataLoader::handleDataset(std::vector<std::string> dataset, int index)
{
  if (dataset.size() <= 3) {
    std::cout << "WARNING: DATASET " << index << " SKIPPED!" << std::endl;
    return;
  }
  std::string name = dataset.at(0);
  int frequency = 0;
  if (!isInt(dataset.at(1))) {
    std::cout << "WARNING: WRONG SYNTAX AT DATASET " << name << std::endl;
    return;
  }
  else {
    frequency = std::atoi(dataset.at(1).c_str());
  }
  std::string type = dataset.at(2);
  if (type == "1f") {
    if (dataset.size() < frequency + 3) {
      std::cout << "WARNING: " << name << " has too few values." << std::endl;
      return;
    }
    if (doesKeyExist(name, m_floatDatasets)) {
      std::cout << "WARNING: " << name << " is already a float dataset." << std::endl;
      return;
    }
    std::vector<float> result;
    for (int f = 0; f < m_particleAmount/frequency; f++) {
      for (int j = 3; j < dataset.size(); j++) {
        float value = 0;
        std::string str = dataset.at(j);
        if (isFloat(str)) {
          value = std::atof(str.c_str());
        }
        else if (str.find("random") == 0) {
          value = createFloatRandom(str.substr(6, str.length() - 6));
        }
        else {
          std::cout << "WARNING: Dataset " << name << " is corrupted." << std::endl;
          return;
        }
        result.push_back(value);
      }
    }
    m_floatDatasets[name] = result;
  }
  else if (type == "4f") {
    if (doesKeyExist(name, m_vec4Datasets)) {
      std::cout << "WARNING: " << name << " is already a vec4 dataset." << std::endl;
      return;
    }
    if ((dataset.size() - 3) % 4 != 0 || dataset.size() != frequency * 4 + 3) {
      std::cout << "WARNING: " << name << " has the wrong amount of values." << std::endl;
      return;
    }
    std::vector<glm::vec4> result;
    for (int f = 0; f < m_particleAmount / frequency; f++) {
      for (int j = 3; j < dataset.size(); j+=4) {
        glm::vec4 value(0.0, 0.0, 0.0, 0.0);
        for (int i = 0; i < 4; i++) {
          std::string str = dataset.at(j + i);
          if (isFloat(str)) {
            value[i] = std::atof(str.c_str());
          }
          else if (str.find("random") == 0) {
            value[i] = createFloatRandom(str.substr(6, str.length() - 6));
          }
          else {
            std::cout << "WARNING: Dataset " << name << " is corrupted." << std::endl;
            return;
          }
        }
        result.push_back(value);
      }
    }
    m_vec4Datasets[name] = result;
  }
  else if (type == "1i") {
    if (doesKeyExist(name, m_intDatasets)) {
      std::cout << "WARNING: " << name << " is already a int constant." << std::endl;
      return;
    }
    if (dataset.size() < frequency + 3) {
      std::cout << "WARNING: " << name << " has too few values." << std::endl;
      return;
    }
    std::vector<int> result;
    for (int f = 0; f < m_particleAmount / frequency; f++) {
      for (int j = 3; j < dataset.size(); j++) {
        if (isInt(dataset.at(j))) {
          int value = std::atof(dataset.at(j).c_str());
          result.push_back(value);
        }
        else {
          std::cout << "WARNING: Dataset " << name << " is corrupted." << std::endl;
          return;
        }
      }
    }
    m_intDatasets[name] = result;
  }
  else if (type == "T" && dataset.size() == 5) {
    std::string actualType = dataset.at(3);
    std::string key = dataset.at(4);
    if (actualType == "1f" && doesKeyExist(key, m_floatDatasets)) {
      if (doesKeyExist(name, m_floatDatasets)) {
        std::cout << "WARNING: " << name << " is already a float dataset." << std::endl;
        return;
      }
      m_floatDatasets[name] = m_floatDatasets[key];
    }
    else if (actualType == "4f" && doesKeyExist(key, m_vec4Datasets)) {
      if (doesKeyExist(name, m_vec4Datasets)) {
        std::cout << "WARNING: " << name << " is already a vec4 dataset." << std::endl;
        return;
      }
      m_vec4Datasets[name] = m_vec4Datasets[key];
    }
    else if (actualType == "1i" && doesKeyExist(key, m_intDatasets)) {
      if (doesKeyExist(name, m_intDatasets)) {
        std::cout << "WARNING: " << name << " is already a int dataset." << std::endl;
        return;
      }
      m_intDatasets[name] = m_intDatasets[key];
    }
    else {
      std::cout << "DATASET " << name << " NOT RECOGNIZED." << std::endl;
      return;
    }
    std::cout << "Copied data from dataset " << key << " to new dataset " << name << "." << std::endl;

  }
  else {
    std::cout << "TYPE " << type << " OF DATASET " << name << " NOT RECOGNIZED." << std::endl;
  }
  std::cout << "Dataset " << name << " loaded." << std::endl;
}

float vup::DataLoader::createFloatRandom(std::string str)
{
  std::vector<std::string> bounds = tokenizeLine(str.c_str(), ',');
  if (bounds.size() == 2 && isFloat(bounds.at(0)) && isFloat(bounds.at(1))) {
    float lower = std::atof(bounds.at(0).c_str());
    float upper = std::atof(bounds.at(1).c_str());
    return randomFloat(lower, upper);
  }
  std::cout << "WARNING: Wrong or missing bounds for random. Writing 0 instead." << std::endl;
  return 0.0;
}
