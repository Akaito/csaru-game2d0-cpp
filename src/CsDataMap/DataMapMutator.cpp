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

#include "DataMapMutator.h"
#include <assert.h>
#include "DataNode.h"
#include "../Utils.h"

#define DATAMAPMUTATOR_BREAK_ON_INVALIDATING_ACTIONS 1
#define DATAMAPMUTATOR_BASIC_SAFETY_CHECKS 1
#define DATAMAPMUTATOR_EXTRA_SAFETY_CHECKS 1

namespace Core {

DataMapMutator::DataMapMutator(DataNode* data_node)
    : node_(data_node) {
}



DataMapMutator::DataMapMutator(const DataMapMutator& other)
    : node_(other.node_) {
  node_stack_ = other.node_stack_;
}



DataMapMutator& DataMapMutator::operator=(const DataMapMutator& rhs) {
  node_ = rhs.node_;
  node_stack_ = rhs.node_stack_;
  return *this;
}



DataMapMutator& DataMapMutator::PushNode(DataNode* node) {
  node_stack_.push_back(node_);
  node_ = node;
  return *this;
}



DataMapMutator& DataMapMutator::PopNode(void) {
  // if at the root node, invalidate this Mutator
  if (node_stack_.empty()) {
#if DATAMAPMUTATOR_BREAK_ON_INVALIDATING_ACTIONS
    assert(false && "DataMapMutator::PopNode() called, but this Mutator was "
           "already at the root node.");
#endif
    node_ = NULL;
  // otherwise, go up one node in the stack
  } else {
    node_ = node_stack_.back();
    node_stack_.pop_back();
  }
  return *this;
}


DataNode* DataMapMutator::GetParentNode(void) {
	if (node_stack_.empty())
		return NULL;
	return node_stack_.back();
}



DataMapMutator& DataMapMutator::ToFirstChild(void) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::ToFirstChild() called, but node_ == NULL.");
#endif
  
  DataNode* child = node_->GetChildSafe(0);
  // this is a mutator.  If there are no children, create one
  if (child == NULL)
    child = node_->AppendNewChild();
  
  PushNode(child);
  return *this;
}



DataMapMutator& DataMapMutator::ToLastChild(void) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::ToLastChild() called, but node_ == NULL.");
#endif
  
  // this is a mutator.  If there are no children, create one
  if (node_->GetChildCount() == 0)
    PushNode(node_->AppendNewChild());
  else
    PushNode(node_->GetChildFast(node_->GetChildCount() - 1));
  return *this;
}



DataMapMutator& DataMapMutator::ToChild(int index) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::ToChild(int index) called, "
         "but node_ == NULL.");
  assert(index >= 0 && "DataMapMutator::ToChild(int index) called with a "
         "negative index.");
#endif
  
  // this is a mutator.  If there are not enough children, create them
  while (node_->GetChildCount() <= index)
    node_->AppendNewChild();
  
  PushNode(node_->GetChildFast(index));
  return *this;
}



DataMapMutator& DataMapMutator::ToChild(const char* name) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::ToChild(const char* name) called, "
         "but node_ == NULL.");
#endif
  
  DataNode* desired_child = node_->GetChildByName(name);
  
  // this is a mutator.  If there is no such child, create one
  if (desired_child == NULL)
    PushNode(node_->AppendNewChild()->SetName(name));
  else
    PushNode(desired_child); 
  return *this;
}



DataMapMutator& DataMapMutator::ToNextSibling(void) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::ToNextSibling() called, but node_ == NULL.");
#endif
#if DATAMAPMUTATOR_BREAK_ON_INVALIDATING_ACTIONS
  assert(!node_stack_.empty() &&
         "DataMapMutator::ToNextSibling() called, but node_ is the root.  "
         "Root nodes are not allowed to have siblings.");
#endif
  
  DataNode* parent = node_stack_.back();
  // calculate the index we are currently at
  int prev_index = 0;
  // NOTE: If the top of the node_stack_ does not have as one of its children
  //  the current node_, this will loop infinitely.
  while (parent->GetChildFast(prev_index) != node_)
    ++prev_index;
  
  DataNode* sibling = parent->GetChildSafe(prev_index + 1);
  
  // this is a mutator.  If there is no next sibling, create one
  if (sibling == NULL)
    node_ = parent->AppendNewChild();
  else
    node_ = sibling;
  return *this;
}



DataMapMutator& DataMapMutator::ToPreviousSibling(void) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::ToPreviousSibling() called, but node_ == "
      "NULL.");
#endif
#if DATAMAPMUTATOR_BREAK_ON_INVALIDATING_ACTIONS
  assert(!node_stack_.empty() &&
         "DataMapMutator::ToPreviousSibling() called, but node_ is the root.  "
         "Root nodes are not allowed to have siblings.");
#endif
  
  DataNode* parent = node_stack_.back();
  // calculate the index we are currently at
  int prev_index = -1;
  // NOTE: If the top of the node_stack_ does not have as one of its children
  //  the current node_, this will loop infinitely.
  while (parent->GetChildFast(prev_index) != node_)
    ++prev_index;
  
  DataNode* sibling = parent->GetChildSafe(prev_index);
  
  // this is a mutator.  If there is no next sibling, create one
  if (sibling == NULL)
    node_ = parent->InsertNewChild(0);
  else
    node_ = sibling;
  return *this;
}


bool DataMapMutator::IsFirstChild(void) {
  // if no parent, is first child
  if (node_stack_.size() <= 1)
    return true;
  return (node_stack_.back()->GetChildFast(0) == node_);
}



DataMapMutator& DataMapMutator::SetToObjectType(void) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::SetToObjectType() called, but node_ == NULL.");
#endif
  
  node_->SetType(DataNode::kObject);
  return *this;
}



DataMapMutator& DataMapMutator::SetToArrayType(void) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::SetToArrayType() called, but node_ == NULL.");
#endif
  
  node_->SetType(DataNode::kArray);
  return *this;
}



DataMapMutator& DataMapMutator::SetToBooleanType(void) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::SetToBooleanType() called, but node_ == NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::SetToBooleanType() called, but "
      "node_ is the root of a DataMap.  Roots must be of the Object or Array "
      "type.");
#endif
  
  // SetBool() implies SetType()
  node_->SetType(DataNode::kBool);
  // not using SetBool(), because this is just a type-changing function.
  //  For performance reasons, we'll assume the user will still set the data.
  //node_->SetBool(false);
  return *this;
}



DataMapMutator& DataMapMutator::SetToNullType(void) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::SetToNullType() called, but node_ == NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::SetToNullType() called, but "
      "node_ is the root of a DataMap.  Roots must be of the Object or Array "
      "type.");
#endif
  
  node_->SetType(DataNode::kNull);
  return *this;
}



DataMapMutator& DataMapMutator::CreateChild(char const* name) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::CreateChild() called, but node_ == NULL.");
#endif
  
  DataNode* child = node_->AppendNewChild();
  if (name != nullptr)
	  child->SetName(name);
  return *this;
}


DataMapMutator& DataMapMutator::CreateChildSafe(char const* name, size_t name_len) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::CreateChildSafe() called, but node_ == NULL.");
	assert(name && "DataMapMutator::CreateChildSafe() called, but name == NULL.");
#endif
  
  DataNode* child = node_->AppendNewChild();
  child->SetNameSecure(name, name_len);
  return *this;
}


DataMapMutator& DataMapMutator::CreateAndGotoChild(char const* name) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::CreateAndGotoChild() called, but node_ == NULL.");
#endif
  
  DataNode* child = node_->AppendNewChild();
  if (name != nullptr)
	  child->SetName(name);
  PushNode(child);
  return *this;
}


DataMapMutator& DataMapMutator::CreateAndGotoChildSafe(char const* name,
                                                       size_t name_len) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::CreateAndGotoChildSafe() called, but node_ == NULL.");
	//assert(name && "DataMapMutator::CreateAndGotoChildSafe() called, but name == NULL.");
#endif
  
  DataNode* child = node_->AppendNewChild();
  child->SetNameSecure(name, name_len);
  PushNode(child);
  return *this;
}



void DataMapMutator::WriteName(char const* name) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::WriteName() called, but node_ == NULL.");
	assert(name && "DataMapMutator::WriteName() called, but name == NULL.");
#endif
  
  node_->SetName(name);
}



void DataMapMutator::WriteNameSecure(char const* name,
    int size_in_elements) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::WriteNameSecure() called, but "
      "node_ == NULL.");
#endif
  
  node_->SetNameSecure(name, size_in_elements);
}



void DataMapMutator::Write(bool bool_value) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::Write(bool) called, but node_ == NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::Write(bool) called, but "
      "node_ is currently the root.  The root node of a DataMap must be of "
      "either the Object or Array type.");
#endif
  
  node_->SetBool(bool_value);
}



void DataMapMutator::Write(char const* name, bool bool_value) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::Write(bool) called, but node_ == NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::Write(char const*, bool) "
      "called, but node_ is currently the root.  The root node of a DataMap "
      "must be of either the Object or Array type.");
#endif
  
  node_->SetName(name);
  node_->SetBool(bool_value);
}



void DataMapMutator::Write(int int_value) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::Write(int) called, but node_ == NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::Write(int) called, but "
      "node_ is currently the root.  The root node of a DataMap must be of "
      "either the Object or Array type.");
#endif
  
  node_->SetInt(int_value);
}



void DataMapMutator::Write(char const* name, int int_value) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::Write(name, int) called, but node_ == "
      "NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::Write(char const*, int) "
      "called, but node_ is currently the root.  The root node of a DataMap "
      "must be of either the Object or Array type.");
#endif
  
  node_->SetName(name);
  node_->SetInt(int_value);
}



void DataMapMutator::Write(float float_value) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::Write(float) called, but node_ == "
      "NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::Write(float) called, but "
      "node_ is currently the root.  The root node of a DataMap must be of "
      "either the Object or Array type.");
#endif
  
  node_->SetFloat(float_value);
}



void DataMapMutator::Write(char const* name, float float_value) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::Write(name, float) called, but node_ == "
      "NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::Write(char const*, float) "
      "called, but node_ is currently the root.  The root node of a DataMap "
      "must be of either the Object or Array type.");
#endif
  
  node_->SetName(name);
  node_->SetFloat(float_value);
}



void DataMapMutator::Write(char const* string_value) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::Write(char const*) called, but node_ == "
      "NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::Write(char const*) called, "
      "but node_ is currently the root.  The root node of a DataMap must be "
      "of either the Object or Array type.");
#endif
  
  node_->SetString(string_value);
}



void DataMapMutator::Write(char const* name,
    char const* string_value) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::Write(char const*, char const*) called, "
      "but node_ == NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::Write(char const*, "
      "char const*) called, but node_ is currently the root.  The root node "
      "of a DataMap must be of either the Object or Array type.");
#endif
  
  node_->SetName(name);
  node_->SetString(string_value);
}



void DataMapMutator::WriteSafe(char const* name,
    int name_size_in_elements, bool bool_value) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::WriteSafe(char const*, int, int) called, "
      "but node_ == NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::WriteSafe(char const*, "
      "int, bool) called, but node_ is currently the root.  The root node of "
      "a DataMap must be of either the Object or Array type.");
#endif
  
  node_->SetNameSecure(name, name_size_in_elements);
  node_->SetBool(bool_value);
}



void DataMapMutator::WriteSafe(char const* name,
    int name_size_in_elements, int int_value) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::WriteSafe(char const*, int, int) called, "
      "but node_ == NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::WriteSafe(char const*, "
      "int, int) called, but node_ is currently the root.  The root node of a "
      "DataMap must be of either the Object or Array type.");
#endif
  
  node_->SetNameSecure(name, name_size_in_elements);
  node_->SetInt(int_value);
}



void DataMapMutator::WriteSafe(char const* name, int name_size_in_elements,
    float float_value) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::WriteSafe(char const*, int, float) "
      "called, but node_ == NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::WriteSafe(char const*, "
      "int, float) called, but node_ is currently the root.  The root node of "
      "a DataMap must be of either the Object or Array type.");
#endif
  
  node_->SetNameSecure(name, name_size_in_elements);
  node_->SetFloat(float_value);
}



void DataMapMutator::WriteSafe(char const* string_value,
    int value_size_in_elements) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::WriteSafe(char const*, int) called, but "
      "node_ == NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::WriteSafe(char const*, "
      "int) called, but node_ is currently the root.  The root node of a "
      "DataMap must be of either the Object or Array type.");
#endif
  
  node_->SetStringSecure(string_value, value_size_in_elements);
}



void DataMapMutator::WriteSafe(char const* name, int name_size_in_elements,
    char const* string_value, int value_size_in_elements) {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::WriteSafe(char const*, int, char const*, "
      "int) called, but node_ == NULL.");
  assert(!node_stack_.empty() && "DataMapMutator::WriteSafe(char const*, "
      "int, char const*, int) called, but node_ is currently the root.  The "
      "root node of a DataMap must be of either the Object or Array type.");
#endif
  
  node_->SetNameSecure(name, name_size_in_elements);
  node_->SetStringSecure(string_value, value_size_in_elements);
}



void DataMapMutator::Walk(int count) {
  for (int i = 0;  i < count;  ++i)
    ToNextSibling();
}



void DataMapMutator::WriteWalkSafeBooleanValue(char const* name,
                                               int name_size_in_elements,
                                               bool value)
{
	WriteNameSecure(name, name_size_in_elements);
	// SetBool() implies SetToBooleanType()
	//SetToBooleanType();
	node_->SetBool(value);
	ToNextSibling();
}


void DataMapMutator::WriteWalkSafeIntegerValue(char const* name,
                                               int name_size_in_elements,
                                               int value)
{
	WriteNameSecure(name, name_size_in_elements);
	node_->SetInt(value);
	ToNextSibling();
}


void DataMapMutator::DeleteLastChildren(int count) {
  for (int i = 0;  i < count;  ++i)
    node_->DeleteLastChild();
}



const char* DataMapMutator::ReadName(void) const {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::ReadName() called, but node_ == NULL.");
#endif
  
  return node_->GetName();
}



bool DataMapMutator::ReadBool(void) const {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::ReadBool() called, but node_ == NULL.");
#endif
#if DATAMAPMUTATOR_EXTRA_SAFETY_CHECKS
  assert(node_->GetType() == DataNode::kBool && "DataMapMutator::ReadBool() "
      "called, but node_'s type is not kBool.");
#endif
  
  return node_->GetBool();
}



int DataMapMutator::ReadInt(void) const {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::ReadInt() called, but node_ == NULL.");
#endif
#if DATAMAPMUTATOR_EXTRA_SAFETY_CHECKS
  assert(node_->GetType() == DataNode::kInt && "DataMapMutator::ReadInt() "
      "called, but node_'s type is not kInt.");
#endif
  
  return node_->GetInt();
}



float DataMapMutator::ReadFloat(void) const {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::ReadFloat() called, but node_ == NULL.");
#endif
#if DATAMAPMUTATOR_EXTRA_SAFETY_CHECKS
  assert(node_->GetType() == DataNode::kFloat && "DataMapMutator::ReadFloat() "
      "called, but node_'s type is not kFloat.");
#endif
  
  return node_->GetFloat();
}



const char* DataMapMutator::ReadString(void) const {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::ReadString() called, but node_ == NULL.");
#endif
#if DATAMAPMUTATOR_EXTRA_SAFETY_CHECKS
  assert(node_->GetType() == DataNode::kString && "DataMapMutator::ReadString() "
      "called, but node_'s type is not kString.");
#endif
  
  return node_->GetString();
}



bool DataMapMutator::ReadBoolSafe(bool* out_bool) const {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(out_bool && "DataMapMutator::ReadBoolSafe() called, but out_bool "
      "== NULL.");
  assert(node_ && "DataMapMutator::ReadBoolSafe() called, but node_ == NULL.");
#endif
  
  return node_->QueryBool(out_bool);
}



bool DataMapMutator::ReadIntSafe(int* out_int) const {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(out_int && "DataMapMutator::ReadIntSafe() called, but out_int "
         "== NULL.");
  assert(node_ && "DataMapMutator::ReadIntSafe() called, but node_ == NULL.");
#endif
  
  return node_->QueryInt(out_int);
}



bool DataMapMutator::ReadFloatSafe(float* out_float) const {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(out_float && "DataMapMutator::ReadFloatSafe() called, but out_float "
         "== NULL.");
  assert(node_ && "DataMapMutator::ReadFloatSafe() called, but node_ "
         "== NULL.");
#endif
  
  return node_->QueryFloat(out_float);
}



bool DataMapMutator::ReadStringSafe(char* out_string,
                                    int buffer_size_in_elements) const {
#if DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
  assert(out_string && "DataMapMutator::ReadStringSafe() called, but "
         "out_string == NULL.");
  assert(node_ && "DataMapMutator::ReadStringSafe() called, but node_ "
         "== NULL.");
#endif
  
  return node_->QueryString(out_string, buffer_size_in_elements);
}

} // namespace Core

#undef DATAMAPMUTATOR_EXTRA_SAFETY_CHECKS
#undef DATAMAPMUTATOR_BASIC_SAFETY_CHECKS
#undef DATAMAPMUTATOR_BREAK_ON_INVALIDATING_ACTIONS
