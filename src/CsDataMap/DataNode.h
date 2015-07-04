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

#include <vector>

/*
#if _MSC_VER > 1000
#pragma warning(push)
// TODO: Don't use this nonstandard extension
#pragma warning(disable:4201) // nameless struct/union nonstandard extension
#endif
*/

namespace Core {

// ASSUMPTION: Does not contain a vtable. // TODO: Double-check this assertion.
// ASSUMPTION: Uses 1-byte chars.
class DataNode {
 public:
  static const int kNameSize = 28;
  static const int kStringDataSize = 64;
  static const int kMaxDepth = 7;  // TODO: Error on passing MaxDepth
  
  enum DataNodeTypes {
    kUnused = 0,
    kNull,
    kObject,
    kArray,
    kBool,
    kInt,
    kFloat,
    kString
  };
  
  DataNode(void) : type_(kUnused)
      { name_[0] = '\0'; }
  
  // WARNING: potentially VERY SLOW
  DataNode(const DataNode& other);
  
  // WARNING: potentially VERY SLOW
  DataNode& operator=(const DataNode& rhs);
  
  explicit DataNode(const char* name, DataNodeTypes type = kNull);
  explicit DataNode(const char* name, int int_data);
  explicit DataNode(const char* name, float float_data);
  explicit DataNode(const char* name, const char* string_data);
  explicit DataNode(const char* name, bool bool_data);
  
  void Initialize(void);
  //void Reset(void);
  //bool Validate(void) const;
  
  // sets the last char of name_ to NULL.  Additionally, if the type_ is
  //  currently kString, sets the last char of string_data_ to NULL.
  void Sanitize(void);
  
  inline int GetInt(void) const
      { return data_.int_; }
  
  // only writes to out_int if this is of type kInt
  // ASSUMPTION: it is valid to write to out_int (it's not NULL, etc.)
  // RETURNS: true if out_int was written to
  bool QueryInt(int* out_int) const;
  
  inline float GetFloat(void) const
      { return data_.float_; }
  
  // only writes to out_float if this is of type kFloat
  // ASSUMPTION: it is valid to write to out_float (it's not NULL, etc.)
  // RETURNS: true if out_float was written to
  bool QueryFloat(float* out_float) const;
  
  const char* GetString(void) const
      { return data_.string_; }
  
  // only writes to out_string if this is of type kString
  // ASSUMPTION: it is valid to write to out_string (it's not NULL, etc.)
  // RETURNS: true if out_string was written to
  bool QueryString(char* out_string, int out_string_size_in_elements) const;
  
  bool GetBool(void) const
      { return data_.bool_; }
  
  // only writes to out_bool if this is of type kBool
  // ASSUMPTION: it is valid to write to out_bool (it's not NULL, etc.)
  // RETURNS: true if out_bool was written to
  bool QueryBool(bool* out_bool) const;
  
  bool IsNull(void) const
      { return type_ == kNull; }
  
  inline const char* GetName(void) const
      { return name_; }
  
  // new_name must be null-terminated
  DataNode* SetName(const char* new_name);
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  DataNode* SetNameSecure(const char* new_name, int size_in_elements);
  
  inline DataNodeTypes GetType(void) const
      { return type_; }
  
  // also deletes children if this is being changed to a type that cannot have
  //  children
  // WARNING: If this DataNode has any children, then this action invalidates
  //  any DataMapMutators/Readers that happen to be pointing to any of those
  //  children without any way of detecting the invalidation.
  DataNode* SetType(DataNodeTypes type);
  
  // convenience function to check if the current node is either an object
  //   or an array.
  inline bool IsContainerType(void) const
			{ return type_ == kObject || type_ == kArray; }
  
  // WARNING: If this DataNode has any children, then this action invalidates
  //  any DataMapMutators/Readers that happen to be pointing to any of those
  //  children without any way of detecting the invalidation.
  DataNode* SetInt(int new_int);
  
  // WARNING: If this DataNode has any children, then this action invalidates
  //  any DataMapMutators/Readers that happen to be pointing to any of those
  //  children without any way of detecting the invalidation.
  DataNode* SetFloat(float new_float);
  
  // new_string must be null-terminated
  // WARNING: If this DataNode has any children, then this action invalidates
  //  any DataMapMutators/Readers that happen to be pointing to any of those
  //  children without any way of detecting the invalidation.
  DataNode* SetString(const char* new_string);
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  // WARNING: If this DataNode has any children, then this action invalidates
  //  any DataMapMutators/Readers that happen to be pointing to any of those
  //  children without any way of detecting the invalidation.
  DataNode* SetStringSecure(const char* new_string, int size_in_elements);
  
  // WARNING: If this DataNode has any children, then this action invalidates
  //  any DataMapMutators/Readers that happen to be pointing to any of those
  //  children without any way of detecting the invalidation.
  DataNode* SetBool(bool new_bool);
  
  inline int GetChildCount(void) const
      { return static_cast<int>(children_.size()); }
  
  inline bool HasChildren(void) const
			{ return !children_.empty(); }
  
  inline const DataNode* GetChildFast(int index) const
      { return &children_[index]; }
  
  inline DataNode* GetChildFast(int index)
      { return &children_[index]; }
  
  // returns a NULL pointer on invalid indices
  inline const DataNode* GetChildSafe(int index) const {
    if (index < 0 || index >= GetChildCount())
      return NULL;
    return GetChildFast(index);
  }
  
  // returns a NULL pointer on invalid indices
  inline DataNode* GetChildSafe(int index) {
    if (index < 0 || index >= GetChildCount())
      return NULL;
    return GetChildFast(index);
  }
  
  // children are assumed to have unique names (if they have names; array
  //  children don't have names).  If there are duplicates: 1) You are wrong,
  //  and 2) GetChild will always return the first one of the matching name.
  const DataNode* GetChildByName(const char* name) const;
        DataNode* GetChildByName(const char* name);
  
  // also changes type to kObject if this was previously not of a type which
  //  is permitted to have children.
  // WARNING: Invalidates any DataMapMutators/Readers that happen to be
  //  pointing to any of this DataNode's children without any way of detecting
  //  the invalidation.
  DataNode* AppendNewChild(void);
  
  // WARNING: potentially _*VERY SLOW*_.  Must shift all following children in
  //  the children_ array by copying them one at a time.  This will cause them
  //  to copy their name_, data, type, and all their children (which then must
  //  copy all their children, and so on).
  // WARNING: Invalidates any DataMapMutators/Readers that happen to be
  //  pointing to any of this DataNode's children without any way of detecting
  //  the invalidation.
  DataNode* InsertNewChild(int index);
  
  // WARNING: Invalidates any DataMapMutators/Readers that happen to be
  //  pointing to any of this DataNode's children without any way of detecting
  //  the invalidation.
  void DeleteLastChild(void);
  
  // WARNING: Invalidates any DataMapMutators/Readers that happen to be
  //  pointing to any of this DataNode's children without any way of detecting
  //  the invalidation.
  // RETURNS: this.
  DataNode* DeleteAllChildren(void);
  
  char name_[kNameSize];
  DataNodeTypes type_;
  
  union {
    int int_;
    float float_;
    char string_[kStringDataSize];
    bool bool_;
  } data_;
  
  std::vector<DataNode> children_;
  
 private:
};

} // namespace Core
