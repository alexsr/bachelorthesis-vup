// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_CLPROGRAMCOMPILATIONEXCEPTION_H
#define VUP_CLPROGRAMCOMPILATIONEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when the OpenCL program compilation fails.
class CLProgramCompilationException : public std::exception {

public:
  CLProgramCompilationException(std::string path, std::string log) throw() {
    m_path = path;
    m_log = log;
    m_msg = "OpenCL program compilation of " + m_path + " failed.\n" + log;
  };
  std::string getPath() {
    return m_path;
  }
  std::string getLog() {
    return m_log;
  }
  virtual const char* what() const throw () {
    return m_msg.c_str();
  }
private:
  std::string m_path;
  std::string m_log;
  std::string m_msg;

};

}

#endif
