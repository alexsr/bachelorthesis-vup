// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_BUFFERNOTFOUNDEXCEPTION_H
#define VUP_BUFFERNOTFOUNDEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when the buffer is not found.
class BufferNotFoundException : public std::exception {

public:
  BufferNotFoundException(std::string name) throw() {
    m_name = name;
    m_msg = "Buffer " + m_name + " is not found.";
  };
  std::string getName() {
    return m_name;
  }
  virtual const char* what() const throw () {
    return m_msg.c_str();
  }
private:
  std::string m_name;
  std::string m_msg;

};

}

#endif
