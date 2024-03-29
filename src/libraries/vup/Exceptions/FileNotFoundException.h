// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_FILENOTFOUNDEXCEPTION_H
#define VUP_FILENOTFOUNDEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when the file is not found.
class FileNotFoundException : public std::exception {

public:
  FileNotFoundException(std::string path) throw() {
    m_path = path;
    m_msg = "File not found: " + m_path;
  }
  std::string getPath() {
    return m_path;
  }
  virtual const char* what() const throw () {
    return m_msg.c_str();
  }
private:
  std::string m_path;
  std::string m_msg;

};

}

#endif
