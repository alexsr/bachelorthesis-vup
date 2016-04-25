// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_FILENOTFOUNDEXCEPTION_H
#define VUP_FILENOTFOUNDEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when the shader source compilation fails.
class FileNotFoundException : public std::exception {

public:
  FileNotFoundException(const char* path) throw() : exception("File not found: ") {
    m_path = path;
  };
  const char* getPath() {
    return m_path;
  }
private:
  const char* m_path;

};

}

#endif
