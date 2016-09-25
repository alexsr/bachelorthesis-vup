// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_BUFFERWRITINGEXCEPTION_H
#define VUP_BUFFERWRITINGEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when enqueuing writing buffer fails.
class BufferWritingException : public std::exception {

public:
  BufferWritingException(int error) throw() {
    m_error = error;
    m_msg = "Enqueuing writing buffer returned error " + std::to_string(m_error);
  };
  int getError() {
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
