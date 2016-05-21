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
  FileReader(const char* path);
  ~FileReader();
  const char* getSourceChar() { return m_source.c_str(); }
  bool isLoaded() { return m_loaded; }

private:
  std::string load(const char* path);

  bool m_loaded;
  int m_size;
  const char* m_path;
  std::string m_source;

};

}

#endif
