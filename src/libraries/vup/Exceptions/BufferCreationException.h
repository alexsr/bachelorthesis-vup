// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_BUFFERCREATIONEXCEPTION_H
#define VUP_BUFFERCREATIONEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when any buffer creation fails.
class BufferCreationException : public std::exception {

public:
  BufferCreationException(std::string name, int error) throw() {
    m_error = error;
    m_name = name;
    m_msg = "Buffer creation of " + m_name + " returned error " + std::to_string(m_error);
  };
  int getError() {
    return m_error;
  }
  std::string getBufferName() {
    return m_name;
  }
  virtual const char* what() const throw () {
    return m_msg.c_str();
  }
private:
  int m_error; // OpenCL error code
  std::string m_name;
  std::string m_msg;
};

}

#endif
