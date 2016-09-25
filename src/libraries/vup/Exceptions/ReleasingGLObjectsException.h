// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_RELEASINGGLOBJECTSEXCEPTION_H
#define VUP_RELEASINGGLOBJECTSEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when releasing GL objects fails.
class ReleasingGLObjectsException : public std::exception {

public:
  ReleasingGLObjectsException(int error) throw() {
    m_error = error;
    m_msg = "Releasing GLObjects returned error " + m_error;
  };
  int getPath() {
    return m_error;
  }
  virtual const char* what() const throw () {
    return m_msg.c_str();
  }
private:
  int m_error; // OpenCL error code
  std::string m_msg;

};

}

#endif
