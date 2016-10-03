// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_BOILERPLATEEXCEPTION_H
#define VUP_BOILERPLATEEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when any boilerplate action fails, like creation of context.
class BoilerplateException : public std::exception {

public:
  BoilerplateException(std::string object, int error) throw() {
    m_error = error;
    m_object = object;
    m_msg = m_object + " caused error " + std::to_string(m_error);
  };
  int getError() {
    return m_error;
  }
  std::string getErrorObject() {
    return m_object;
  }
  virtual const char* what() const throw () {
    return m_msg.c_str();
  }
private:
  int m_error; // OpenCL error code
  std::string m_object;
  std::string m_msg;
};

}

#endif
