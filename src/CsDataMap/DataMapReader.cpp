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

#include "DataMapReader.h"
#include <assert.h>
#include "DataNode.h"

#define DATAMAPREADER_BREAK_ON_INVALIDATING_ACTIONS 0
#define DATAMAPREADER_BASIC_SAFETY_CHECKS 1
#define DATAMAPREADER_EXTRA_SAFETY_CHECKS 1

namespace Core {

DataMapReader::DataMapReader(const DataNode* node)
    : node_(node) {
}



DataMapReader::DataMapReader(const DataMapReader& other)
    : node_(other.node_) {
  node_stack_ = other.node_stack_;
}



DataMapReader& DataMapReader::operator=(const DataMapReader& rhs) {
  node_ = rhs.node_;
  node_stack_ = rhs.node_stack_;
  return *this;
}



DataMapReader& DataMapReader::PopNode(void) {
  // if at the root node, invalidate this Reader
  if (node_stack_.empty()) {
#if DATAMAPREADER_BREAK_ON_INVALIDATING_ACTIONS
    assert(false && "DataMapReader::PopNode() called, but this Reader was "
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



DataMapReader& DataMapReader::ToFirstChild(void) {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapReader::ToFirstChild() called, but node_ == NULL.");
#endif
  
  const DataNode* child = node_->GetChildSafe(0);
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(child && "DataMapReader::ToFirstChild() called, but node_ has no "
	                "children.");
#endif
  PushNode(child);
  return *this;
}



DataMapReader& DataMapReader::ToLastChild(void) {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapReader::ToLastChild() called, but node_ == NULL.");
#endif
  
  int child_count = node_->GetChildCount();
  PushNode(node_->GetChildSafe(child_count - 1));
  return *this;
}



DataMapReader& DataMapReader::ToChild(int index) {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapReader::ToChild(int index) called, "
         "but node_ == NULL.");
  assert(index >= 0 && "DataMapReader::ToChild(int index) called with a "
         "negative index.");
#endif
  
  PushNode(node_->GetChildSafe(index));
  return *this;
}



DataMapReader& DataMapReader::ToChild(const char* name) {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapReader::ToChild(const char* name) called, "
         "but node_ == NULL.");
#endif
  
  const DataNode* desired_child = node_->GetChildByName(name);
  PushNode(desired_child); 
  return *this;
}



DataMapReader& DataMapReader::ToNextSibling(void) {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapMutator::ToNextSibling() called, but "
         "node_ == NULL.");
#endif
#if DATAMAPREADER_BREAK_ON_INVALIDATING_ACTIONS
  assert(!node_stack_.empty() &&
         "DataMapReader::ToNextSibling() called, but node_ is the root.  "
         "Root nodes are not allowed to have siblings.");
#endif
  
  // if on root node, invalidate
  if (node_stack_.empty()) {
    node_ = NULL;
    return *this;
  }
  const DataNode* parent = node_stack_.back();
  // calculate the index we are currently at
  int prev_index = 0;
  // NOTE: If the top of the node_stack_ does not have as one of its children
  //  the current node_, this will loop infinitely.
  while (parent->GetChildFast(prev_index) != node_)
    ++prev_index;
  
  node_ = parent->GetChildSafe(prev_index + 1);
  return *this;
}


DataMapReader& DataMapReader::ToPreviousSibling(void) {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapReader::ToPreviousSibling() called, but node_ == "
      "NULL.");
#endif
#if DATAMAPREADER_BREAK_ON_INVALIDATING_ACTIONS
  assert(!node_stack_.empty() &&
         "DataMapReader::ToPreviousSibling() called, but node_ is the root.  "
         "Root nodes are not allowed to have siblings.");
#endif
  
  // if on root node, invalidate
  if (node_stack_.empty()) {
    node_ = NULL;
    return *this;
  }
  const DataNode* parent = node_stack_.back();
  // calculate the index we are currently at
  int prev_index = -1;
  // NOTE: If the top of the node_stack_ does not have as one of its children
  //  the current node_, this will loop infinitely.
  while (parent->GetChildFast(prev_index) != node_)
    ++prev_index;
  
  node_ = parent->GetChildSafe(prev_index);
  return *this;
}



const char* DataMapReader::ReadName(void) const {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapReader::ReadName() called, but node_ == NULL.");
#endif
  
  return node_->GetName();
}



bool DataMapReader::ReadBool(void) const {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapReader::ReadBool() called, but node_ == NULL.");
#endif
#if DATAMAPREADER_EXTRA_SAFETY_CHECKS
  assert(node_->GetType() == DataNode::kBool && "DataMapReader::ReadBool() "
      "called, but node_'s type is not kBool.");
#endif
  
  return node_->GetBool();
}



int DataMapReader::ReadInt(void) const {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapReader::ReadInt() called, but node_ == NULL.");
#endif
#if DATAMAPREADER_EXTRA_SAFETY_CHECKS
  assert(node_->GetType() == DataNode::kInt && "DataMapReader::ReadInt() "
      "called, but node_'s type is not kInt.");
#endif
  
  return node_->GetInt();
}



float DataMapReader::ReadFloat(void) const {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapReader::ReadFloat() called, but node_ == NULL.");
#endif
#if DATAMAPREADER_EXTRA_SAFETY_CHECKS
  assert(node_->GetType() == DataNode::kFloat && "DataMapReader::ReadFloat() "
      "called, but node_'s type is not kFloat.");
#endif
  
  return node_->GetFloat();
}



const char* DataMapReader::ReadString(void) const {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapReader::ReadString() called, but node_ == NULL.");
#endif
#if DATAMAPREADER_EXTRA_SAFETY_CHECKS
  assert(node_->GetType() == DataNode::kString &&
         "DataMapReader::ReadString() called, but node_'s "
         "type is not kString.");
#endif
  
  return node_->GetString();
}



void DataMapReader::ReadWString(std::wstring * wstrOut) const {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(node_ && "DataMapReader::ReadString() called, but node_ == NULL.");
#endif
#if DATAMAPREADER_EXTRA_SAFETY_CHECKS
  assert(node_->GetType() == DataNode::kString &&
         "DataMapReader::ReadString() called, but node_'s "
         "type is not kString.");
#endif
  
  // Conversion below from http://stackoverflow.com/questions/6691555/converting-narrow-string-to-wide-string
  std::string str = node_->GetString();
  wstrOut->resize(str.length());
  std::copy(str.begin(), str.end(), wstrOut->begin());
}



bool DataMapReader::ReadBoolSafe(bool* out_bool) const {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(out_bool && "DataMapReader::ReadBoolSafe() called, but out_bool "
      "== NULL.");
  assert(node_ && "DataMapReader::ReadBoolSafe() called, but node_ == NULL.");
#endif
  
  return node_->QueryBool(out_bool);
}



bool DataMapReader::ReadIntSafe(int* out_int) const {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(out_int && "DataMapReader::ReadIntSafe() called, but out_int "
         "== NULL.");
  assert(node_ && "DataMapReader::ReadIntSafe() called, but node_ == NULL.");
#endif
  
  return node_->QueryInt(out_int);
}



bool DataMapReader::ReadFloatSafe(float* out_float) const {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(out_float && "DataMapReader::ReadFloatSafe() called, but out_float "
         "== NULL.");
  assert(node_ && "DataMapReader::ReadFloatSafe() called, but node_ "
         "== NULL.");
#endif
  
  return node_->QueryFloat(out_float);
}



bool DataMapReader::ReadStringSafe(char* out_string,
                                   int buffer_size_in_elements) const {
#if DATAMAPREADER_BASIC_SAFETY_CHECKS
  assert(out_string && "DataMapReader::ReadStringSafe() called, but "
         "out_string == NULL.");
  assert(node_ && "DataMapReader::ReadStringSafe() called, but node_ "
         "== NULL.");
#endif
  
  return node_->QueryString(out_string, buffer_size_in_elements);
}



void DataMapReader::PushNode(const DataNode* node) {
  node_stack_.push_back(node_);
  node_ = node;
}

} // namespace Core

#undef DATAMAPREADER_EXTRA_SAFETY_CHECKS
#undef DATAMAPREADER_BASIC_SAFETY_CHECKS
#undef DATAMAPREADER_BREAK_ON_INVALIDATING_ACTIONS
