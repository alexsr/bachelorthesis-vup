// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_FILEREADER_H
#define VUP_FILEREADER_H

#include "vup/defs.h"
#include "vup/Exceptions/FileNotFoundException.h"
#include <string>
#include <fstream>
#include <iostream>

namespace vup {

class FileReader
{
public:
  FileReader(std::string path);
  ~FileReader();
  std::string getSource() { return m_source; }
  const char* getSourceChar() { return m_source.c_str(); }
  int length() { return m_source.length(); }
  bool isLoaded() { return m_loaded; }

private:
  void load(std::string path);

  bool m_loaded;
  int m_size;
  std::string m_path;
  std::string m_source;

};

}

#endif
