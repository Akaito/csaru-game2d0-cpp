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

#include "JsonParser2.h"
#include "../DataMapMutator.h"

namespace Core {

class JsonParserCallbackForDataMap : public JsonParser2::CallbackInterface {
 public:
  JsonParserCallbackForDataMap(const DataMapMutator& mutator);
  
  //
  // CallbackInterface implementations
  //
  
  virtual void BeginObject(const char* name, size_t name_len);
  virtual void EndObject(void);
  virtual void BeginArray(const char* name, size_t name_len);
  virtual void EndArray(void);
  virtual void GotString(const char* name, size_t name_len,
                         const char* value, size_t value_len);
  virtual void GotFloat(const char* name, size_t name_len, float value);
  virtual void GotInteger(const char* name, size_t name_len, int value);
  virtual void GotBoolean(const char* name, size_t name_len, bool value);
  virtual void GotNull(const char* name, size_t name_len);
  
  //
  // Data access
  //
  
  void SetMutator(const DataMapMutator& mutator);
 
 private:
  DataMapMutator m_mutator;
};

} // namespace Core
