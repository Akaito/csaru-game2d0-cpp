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

#include "JsonParserCallbackForDataMap.h"
#include "../DataNode.h"

namespace Core {

JsonParserCallbackForDataMap::JsonParserCallbackForDataMap(
    const DataMapMutator& mutator) : m_mutator(mutator) {
}


void JsonParserCallbackForDataMap::BeginObject(const char* name,
                                               size_t name_len) {
  m_mutator.WriteNameSecure(name, name_len);
  m_mutator.SetToObjectType();
  m_mutator.CreateAndGotoChildSafe(NULL, 0);
}


void JsonParserCallbackForDataMap::EndObject(void) {
	// we only want to clean the last child if it was a temporary write-location
	//  created while parsing.  If we've just bubbled back up from lower nodes,
	//  the last child should _not_ be deleted.
	if (m_mutator.GetCurrentNode()->GetType() == DataNode::kUnused) {
	  m_mutator.ToParent();
	  m_mutator.DeleteLastChildren(1);
	}
	else
		m_mutator.ToParent();
	
	// if in a child object, prepare for more data to be written out
	if (m_mutator.GetCurrentDepth() >= 2)
		m_mutator.Walk(1);
}


void JsonParserCallbackForDataMap::BeginArray(const char* name,
                                              size_t name_len) {
  m_mutator.WriteNameSecure(name, name_len);
  m_mutator.SetToArrayType();
  m_mutator.CreateAndGotoChildSafe(NULL, 0);
}


void JsonParserCallbackForDataMap::EndArray(void)
{
  //m_mutator.ToParent();
  //m_mutator.DeleteLastChildren(1);
  
	// we only want to clean the last child if it was a temporary write-location
	//  created while parsing.  If we've just bubbled back up from lower nodes,
	//  the last child should _not_ be deleted.
	if (m_mutator.GetCurrentNode()->GetType() == DataNode::kUnused) {
	  m_mutator.ToParent();
	  m_mutator.DeleteLastChildren(1);
	}
	else
		m_mutator.ToParent();
	
	m_mutator.Walk(1);
}


void JsonParserCallbackForDataMap::GotString(const char* name, size_t name_len,
                                             const char* value,
                                             size_t value_len)
{
  m_mutator.WriteWalkSafe(name, static_cast<int>(name_len),
                          value, static_cast<int>(value_len));
}


void JsonParserCallbackForDataMap::GotFloat(const char* name, size_t name_len,
                                            float value)
{
	m_mutator.WriteNameSecure(name, name_len);
	m_mutator.Write(value);
	m_mutator.Walk(1);
}


void JsonParserCallbackForDataMap::GotInteger(const char* name,
                                              size_t name_len, int value)
{
	m_mutator.WriteNameSecure(name, name_len);
	m_mutator.Write(value);
	m_mutator.Walk(1);
}


void JsonParserCallbackForDataMap::GotBoolean(const char* name,
                                              size_t name_len, bool value)
{
	m_mutator.WriteWalkSafeBooleanValue(name, static_cast<int>(name_len), value);
}


void JsonParserCallbackForDataMap::GotNull(const char* name, size_t name_len)
{
  m_mutator.WriteWalkSafeNullValue(name, static_cast<int>(name_len));
}


void JsonParserCallbackForDataMap::SetMutator(const DataMapMutator& mutator)
{
  m_mutator = mutator;
}

} // namespace Core
