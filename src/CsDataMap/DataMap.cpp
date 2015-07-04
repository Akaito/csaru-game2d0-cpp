/*
Copyright 2013 Christopher Higgins Barrett

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "DataMap.h"

#if _MSC_VER > 1000
#pragma warning(push)
// disable fprintf unsecure, and other such warnings
#pragma warning(disable:4996)
#endif

namespace Core {

DataMap::DataMap(void) {
  root_node_.SetType(DataNode::kNull);
  root_node_.SetName("UNNAMED");
}



/*
DataMap::DataMap(DataNode* root) : root_node_(root) {
}
//*/



void DataMap::Clear(void) {
	root_node_.DeleteAllChildren();
}



DataMapReader DataMap::GetReader(void) const {
  return DataMapReader(&root_node_);
}



DataMapMutator DataMap::GetMutator(void) {
  return DataMapMutator(&root_node_);
}



/*
bool DataMap::ReadFromFile(char const* filename, Formats format) {
  if (filename == NULL) {
#ifdef _DEBUG
    fprintf(stderr, "DataMap::ReadFromFile() called, but filename == NULL.\n");
#endif
    return false;
  }
  
  FILE* file = NULL;
  
  // open file, and check for success.  This varies by Format
  switch (format) {
    case kJson:
      file = fopen(filename, "rt");
      break;
  }
  
  if (file == NULL) {
#ifdef _DEBUG
    fprintf(stderr, "DataMap::ReadFromFile() failed to open desired file.  "
            "File was [%s].\n", filename);
#endif
    return false;
  }
  
  DataMapMutator mutator(&root_node_);
  // read from file
  bool read_result = false;
  switch (format) {
    case kJson:
      read_result = ReadJsonFromFile(file, &mutator);
      break;
  }
  
  fclose(file);
  return read_result;
}
//*/

} // namespace Core

#if _MSC_VER > 100
#pragma warning(pop)
#endif
