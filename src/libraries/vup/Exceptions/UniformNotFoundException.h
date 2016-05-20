// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_UNIFORMNOTFOUNDEXCEPTION_H
#define VUP_UNIFORMNOTFOUNDEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when the shader source compilation fails.
class UniformNotFoundException : public std::exception {

public:
  UniformNotFoundException(const char* uniform) throw() : exception("Uniform not found.") {
    m_uniform = uniform;
  };
  const char* getUniform() {
    return m_uniform;
  }
private:
  const char* m_uniform;

};

}

#endif
