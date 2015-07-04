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

// FILE
#include <cstdio>

namespace Core {

class DataMapReader;

class JsonGenerator {
 public:
  // reader is assumed to be valid, and *WILL* be modified
  static bool WriteToFile(DataMapReader* reader, char const* filename);
  
  static bool WriteToStream(DataMapReader* reader, FILE* file);
 
 private:
  static bool WriteIndent(FILE* file, int indent_amount);
  
  static bool WriteJsonToFile(FILE* file, DataMapReader* reader,
                              bool current_node_writes_name);
  
  static void WriteEscapedStringToFile(FILE* file, const char* string);
 
  //DISALLOW_COPY_AND_ASSIGN(JsonGenerator)
  JsonGenerator(void);
  JsonGenerator(const JsonGenerator&);
  void operator=(const JsonGenerator&);
};

} // namespace Core
