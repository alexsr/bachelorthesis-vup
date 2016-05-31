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
  ProgramCompilationException(std::string infoLog) throw() {
    m_infoLog = infoLog;
    m_msg = "Shader program compilation failed.\n" + m_infoLog;
  };
  std::string getInfoLog() {
    return m_infoLog;
  }
  virtual const char* what() const throw () {
    return m_msg.c_str();
  }
private:
  std::string m_infoLog;
  std::string m_msg;

};

}

#endif
