// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_KERNELCREATIONEXCEPTION_H
#define VUP_KERNELCREATIONEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when the kernel creation fails.
class KernelCreationException : public std::exception {

public:
  KernelCreationException(std::string name, int error) throw() {
    m_error = error;
    m_name = name;
    m_msg = "Creation of kernel " + m_name + " failed." + std::to_string(m_error);
  }
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
