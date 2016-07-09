#include "DataLoader.h"

vup::DataLoader::DataLoader(const char * path)
{
  FileReader fr(path);
}

vup::DataLoader::~DataLoader()
{
}
