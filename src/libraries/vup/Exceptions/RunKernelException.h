// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_RUNKERNELEXCEPTION_H
#define VUP_RUNKERNELEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when the buffer creation fails.
class RunKernelException : public std::exception {

public:
  RunKernelException(std::string name, int error) throw() {
    m_error = error;
    m_name = name;
    m_msg = "Running kernel " + m_name + " returned error " + std::to_string(m_error);
  };
  int getError() {
    return m_error;
  }
  std::string getName() {
    return m_name;
  }
  virtual const char* what() const throw () {
    return m_msg.c_str();
  }
private:
  int m_error;
  std::string m_name;
  std::string m_msg;
};

}

#endif
