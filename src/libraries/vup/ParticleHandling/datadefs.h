// Bachelor thesis Particleframework
// Author: Alexander Scheid-Rehder
// Email: alexsr@uni-koblenz.de

#ifndef VUP_DATADEFS_H
#define VUP_DATADEFS_H

namespace vup {

typedef std::vector<float> floatdata;
typedef std::vector<glm::vec4> vec4data;

enum datatype { EMPTY, INT, FLOAT, VEC4 };

struct DataSpecification {
  int loc = 0;
  datatype format = vup::EMPTY;
  int instances = 1;
};

typedef std::pair<vup::DataSpecification, std::string> datavalue;

typedef std::map<std::string, datavalue> datamap;
typedef std::vector<std::string> identifiers;
typedef std::map<std::string, DataSpecification> typeIdentifiers;


}

#endif
