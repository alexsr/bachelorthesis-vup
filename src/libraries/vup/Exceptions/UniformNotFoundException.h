// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_UNIFORMNOTFOUNDEXCEPTION_H
#define VUP_UNIFORMNOTFOUNDEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when a uniform in not found.
class UniformNotFoundException : public std::exception {

public:
  UniformNotFoundException(std::string uniform) throw() {
    m_uniform = uniform;
    m_msg = "Uniform " + m_uniform + " not found.";
  };
  std::string getUniform() {
    return m_uniform;
  }
  virtual const char* what() const throw () {
    return m_msg.c_str();
  }
private:
  std::string m_uniform;
  std::string m_msg;

};

}

#endif
