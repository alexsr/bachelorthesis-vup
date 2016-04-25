// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_SHADERCOMPILATIONEXCEPTION_H
#define VUP_SHADERCOMPILATIONEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when the shader source compilation fails.
class ShaderCompilationException : public std::exception {

public:
  ShaderCompilationException(const char* infoLog) throw() : exception("Shader source compilation failed.") {
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
