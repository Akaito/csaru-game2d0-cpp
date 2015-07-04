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

#pragma once

#include "DataNode.h"
#include "DataMapMutator.h"
#include "DataMapReader.h"

namespace Core {

class DataMap {
 public:
  DataMap(void);
  
  //explicit DataMap(DataNode* root);
  
  void Clear(void);
  
  DataMapReader GetReader(void) const;
  DataMapMutator GetMutator(void);
  
  //bool ReadFromFile(char const* filename, Formats format = kJson);
 
 private:
  //bool ReadJsonFromFile(FILE* file, DataMapMutator* mutator);
  
  DataNode root_node_;
  
  DISALLOW_COPY_AND_ASSIGN(DataMap)
};

} // namespace Core
