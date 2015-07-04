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

#include "DataNode.h"
#include <assert.h>

#if _MSC_VER > 1000
#pragma warning(push)
// such-and-such is unsafe.  I'm intentionally not using the 'secure' functions
//  provided by Microsoft to avoid platform dependency.  The original, standard
//  functions are used instead.  Where "Safe" class methods are used I'm
//  manually doing what the 'secure' functions would be doing.
#pragma warning(disable:4996)
#endif

namespace Core {

DataNode::DataNode(const DataNode& other) {
  /*
  memcpy(name_, other.name_, kNameSize);
  // NOTE: This assumes that the string_data_ is of 1-byte chars, and is the
  //  largest data member
  memcpy(string_data_, other.string_data_, kNameSize);
  //*/
  
  //memcpy(this, &other, sizeof(DataNode));
  
  SetName(other.name_);
  SetType(other.type_);
  // NOTE: This assumes that the string_data_ is the largest data member
  memcpy(data_.string_, other.data_.string_, sizeof(data_.string_));
  children_ = other.children_;
}



DataNode& DataNode::operator=(const DataNode& rhs) {
  /*
  memcpy(name_, rhs.name_, kNameSize);
  type_ = rhs.type_;
  // NOTE: This assumes that the string_data_ is of 1-byte chars
  memcpy(string_data_, other.string_data_, kNameSize);
  //*/
  
  //memcpy(this, &rhs, sizeof(DataNode));
  
  SetName(rhs.name_);
  SetType(rhs.type_);
  // NOTE: This assumes that the string_data_ is the largest data member
  memcpy(data_.string_, rhs.data_.string_, sizeof(data_.string_));
  children_ = rhs.children_;
  
  return *this;
}



DataNode::DataNode(const char* name, DataNodeTypes type) {
  SetName(name);
  SetType(type);
}



DataNode::DataNode(const char* name, int int_data) {
  SetName(name);
  SetInt(int_data);
}



DataNode::DataNode(const char* name, float float_data) {
  SetName(name);
  SetFloat(float_data);
}



DataNode::DataNode(const char* name, const char* string_data) {
  SetName(name);
  SetString(string_data);
}



DataNode::DataNode(const char* name, bool bool_data) {
  SetName(name);
  SetBool(bool_data);
}



void DataNode::Initialize(void) {
  strcpy(name_, "_INIT_NAME_");
  type_ = kString;
  strcpy(data_.string_, "_INIT_DATA_");
}



void DataNode::Sanitize(void) {
  name_[kNameSize-1] = '\0';
  //if (type_ == kString)
    data_.string_[kStringDataSize-1] = '\0';
}



bool DataNode::QueryInt(int* out_int) const {
  if (type_ != kInt)
    return false;
  *out_int = data_.int_;
  return true;
}



bool DataNode::QueryFloat(float* out_float) const {
  if (type_ != kFloat)
    return false;
  *out_float = data_.float_;
  return true;
}



bool DataNode::QueryString(char* out_string, int out_string_size_in_elements)
    const {
  if (type_ != kString)
    return false;
  int copy_counter = 0;
  
  while (copy_counter < kStringDataSize &&
         data_.string_[copy_counter] &&
         copy_counter < out_string_size_in_elements - 1) {
    out_string[copy_counter] = data_.string_[copy_counter];
    ++copy_counter;
  }
  out_string[copy_counter] = '\0';
  
  return true;
}



bool DataNode::QueryBool(bool* out_bool) const {
  if (type_ != kBool)
    return false;
  *out_bool = data_.bool_;
  return true;
}



DataNode* DataNode::SetName(const char* new_name) {
  strcpy(name_, new_name);
  return this;
}



DataNode* DataNode::SetNameSecure(const char* new_name, int size_in_elements) {
  int i = 0;
  // write empty string first, in case NULL or empty string was given
  name_[0] = '\0';
  
  if (new_name != NULL) {
    while (i < size_in_elements  &&  i < kNameSize) {
      name_[i] = new_name[i];
      ++i;
    }
    
    // write NULL terminating character
    name_[i] = '\0';
  }
  
  // write NULL at last possible char, to protect against read-until-NULL-char
  //   reading outside our buffer.
  //name_[kNameSize - 1] = '\0';
  return this;
}



DataNode* DataNode::SetType(DataNodeTypes type) {
  type_ = type;
  if (type_ != kObject && type_ != kArray)
    DeleteAllChildren();
  else
		children_.reserve(4);
  return this;
}



DataNode* DataNode::SetInt(int new_int) {
  data_.int_ = new_int;
  SetType(kInt);
  return this;
}



DataNode* DataNode::SetFloat(float new_float) {
  data_.float_ = new_float;
  SetType(kFloat);
  return this;
}



DataNode* DataNode::SetString(const char* new_string) {
  strcpy(data_.string_, new_string);
  SetType(kString);
  return this;
}



DataNode* DataNode::SetStringSecure(const char* new_string,
    int size_in_elements) {
  int i = 0;
  // write empty string first, in case a NULL or empty string was given
  data_.string_[0] = '\0';
  
  if (new_string != NULL) {
    while (i < size_in_elements  &&  i < kStringDataSize) {
      data_.string_[i] = new_string[i];
      ++i;
    }
    
    // NULL-terminate our string after copying
    data_.string_[i] = '\0';
  }
  
  //data_.string_[kStringDataSize - 1] = '\0';
  SetType(kString);
  return this;
}



DataNode* DataNode::SetBool(bool new_bool) {
  SetType(kBool);
  data_.bool_ = new_bool;
  return this;
}



const DataNode* DataNode::GetChildByName(const char* name) const {
  int child_count = children_.size();
  for (int i = 0;  i < child_count;  ++i) {
    // if we find a match, return it
    if (!strcmp(children_[i].GetName(), name))
      return &children_[i];
  }
  return NULL;
}



DataNode* DataNode::GetChildByName(const char* name) {
  int child_count = children_.size();
  for (int i = 0;  i < child_count;  ++i) {
    // if we find a match, return it
    if (!strcmp(children_[i].GetName(), name))
      return &children_[i];
  }
  return NULL;
}



DataNode* DataNode::AppendNewChild(void) {
  //children_.resize(children_.size() + 1);
	children_.push_back(Core::DataNode());
  if (type_ != kObject && type_ != kArray)
    SetType(kObject);
  return &children_.back();
}



DataNode* DataNode::InsertNewChild(int index) {
#ifdef _DEBUG
  assert(index >= 0 && "DataNode::InsertNewChild() called with a negative "
         "index.");
  assert(index <= GetChildCount() && "DataNode::InsertNewChild() called with "
         "invalid index.  Index is greater to the number of children of this "
         "DataNode.");
#endif
  
  // copy all children from [index, last_child] in reverse order
  return &*children_.insert(children_.begin() + index, DataNode());
  // reset the child at [index]
  //  (currently unnecessary.  std::vector::insert() takes a copy of a
  //  default-constructed DataNode above)
  //children_[index].SetType(kNull);
  //children_[index].string_data_[0] = '\0';
  // return a pointer to the 'new' child at [index]
  //  (currently being returned above.  The returned iterator is dereferenced,
  //  then the address of that DataNode is taken)
  //return &children_[index];
}



void DataNode::DeleteLastChild(void) {
  if (!children_.empty()) {
    children_.pop_back();
  }
}



DataNode* DataNode::DeleteAllChildren(void) {
  children_.clear();
  return this;
}

} // namespace Core

#if _MSC_VER > 1000
#pragma warning(pop)
#endif
