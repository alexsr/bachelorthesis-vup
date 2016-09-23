// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_DATADEFS_H
#define VUP_DATADEFS_H

namespace vup {

enum datatype { EMPTY, INT, FLOAT, VEC4 };

// Stores specification of particle data.
struct DataSpecification {
  // loc is interop variable specific and determines the OpenGL layout location
  // of its corresponding vbo.
  int loc = 0;
  datatype format = vup::EMPTY;
  // Some variables have multiple instances per data object
  int instances = 1;
};

// Stores the specification and the value as a string.
// The value is later converted into a data type corresponding to spec.format.
struct DataValue {
  DataSpecification spec;
  std::string value;
  DataValue() : spec(DataSpecification()), value("") {};
  DataValue(DataSpecification s, std::string v) : spec(s), value(v) {};
};

typedef std::map<std::string, DataValue> dataMap;
typedef std::vector<std::string> identifiers;
typedef std::map<std::string, DataSpecification> typeIdentifiers;

}

#endif
