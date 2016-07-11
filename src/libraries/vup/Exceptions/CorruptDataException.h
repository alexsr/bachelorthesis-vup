// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_CORRUPTDATAEXCEPTION_H
#define VUP_CORRUPTDATAEXCEPTION_H

#include <exception>
#include <string>

namespace vup {

// Is thrown when the file is not found.
class CorruptDataException : public std::exception {

public:
  CorruptDataException(std::string path, std::string msg = "", int line = 1) throw() {
    m_path = path;
    m_msg = "Corrupt data in file: " + m_path + "\n"
      + "Error at line " + std::to_string(line) + ".\n" + msg;
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
