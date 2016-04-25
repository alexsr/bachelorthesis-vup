// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_PROGRAMCOMPILATIONEXCEPTION_H
#define VUP_PROGRAMCOMPILATIONEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when the shader program compilation fails.
class ProgramCompilationException : public std::exception {

public:
  ProgramCompilationException(const char* infoLog) throw() : exception("Shader source compilation failed.") {
    m_infoLog = infoLog;
  };
  const char* getInfoLog() {
    return m_infoLog;
  }
private:
  const char* m_infoLog;

};

}

#endif
