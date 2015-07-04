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
// nullptr
#include "../Utils.h"

namespace Core {

class DataNode;

class DataMapMutator {
 public:
  explicit DataMapMutator(DataNode* data_node);
  DataMapMutator(const DataMapMutator& other);
  DataMapMutator& operator=(const DataMapMutator& rhs);
  
  inline const DataNode* GetCurrentNode(void) const
      { return node_; }
  
  inline DataNode* GetCurrentNode(void)
      { return node_; }
  
  // RETURNS: 0 if invalidated, 1 if at the root node, 2 if at one of the root
  //  node's children, and so on.
  inline int GetCurrentDepth(void) const
      { return int(node_stack_.size() + (node_ == NULL ? 0 : 1)); }
  
  inline bool IsValid(void) const
      { return node_ != NULL; }
  
  //
  // Navigation methods
  //
  
  // goes down from one node to one of its children.  Using this function to
  //  go from the current node to one of its non-children is undefined.  This
  //  class makes no attempt at saving you from this error; for performance
  //  reasons.
  // WARNING: You shouldn't need to use this function!
  DataMapMutator& PushNode(DataNode* node);
  
  // return to the parent node.
  // NOTE: If this is used on the root node, the Mutator becomes invalidated.
  DataMapMutator& PopNode(void);
  // synonym for PopNode.
  // NOTE: If this is used on the root node, the Mutator becomes invalidated.
  inline DataMapMutator& ToParent(void)
      { return PopNode(); }
  
  // NOTE: Advanced use only!
  DataNode* GetParentNode(void);
  
  // if there are no children, one will be created.
  DataMapMutator& ToFirstChild(void);
  
  // if there are no children, one will be created.
  DataMapMutator& ToLastChild(void);
  
  // if there is no child at the given index, null children will be created
  //  until a child at the given index can be pointed at.
  // WARNING: If no child at the given index exists, this invalidates any
  //  DataMapMutators/Readers which are pointing at any of this DataNode's
  //  children.
  DataMapMutator& ToChild(int index);
  
  // if no child with such a name exists, a new null _last_ child will be added
  //  with the given name.
  // WARNING: If no child of the given name exists, this invalidates any
  //  DataMapMutators/Readers which are pointing at any of this DataNode's
  //  children.
  DataMapMutator& ToChild(const char* name);
  
  // theoretically slow.  Nodes have no knowledge of their siblings, so we
  //  actually PopNode(-ish), then ToChild(prev_index + 1).  prev_index must
  //  also be calculated at this time by walking the current parent's children.
  // NOTE: If this is used on the root node, the Mutator becomes invalidated.
  // WARNING: If no next sibling exists, this invalidates any
  //  DataMapMutators/Readers which are pointing at any of this DataNode's
  //  siblings or children.
  DataMapMutator& ToNextSibling(void);
  
  // theoretically slow.  Nodes have no knowledge of their siblings, so we
  //  actually PopNode(-ish), then ToChild(prev_index - 1).  prev_index must
  //  also be calculated at this time by walking the current parent's children.
  // NOTE: If this is used on the root node, the Mutator becomes invalidated.
  // WARNING: If no previous sibling exists, this invalidates any
  //  DataMapMutators/Readers which are pointing at any of this DataNode's
  //  siblings or children.
  DataMapMutator& ToPreviousSibling(void);
  
  bool IsFirstChild(void);
  // synonym for IsFirstChild().
  inline bool IsFirstSibling(void)
      { return IsFirstChild(); }
  
  //
  // Mutation methods
  //
  // Some will have navigation side effects.
  //
  
  // most common type for children.  Same as array, but children are named.
  DataMapMutator& SetToObjectType(void);
  
  // same as object, but children are unnamed.
  DataMapMutator& SetToArrayType(void);
  
  // if children of the current node exist, they will be destroyed.
  // This does not write anything to the Node's data.  It is assumed you will
  //   set the data to be true or false yourself.
  DataMapMutator& SetToBooleanType(void);
  
  // if children of the current node exist, they will be destroyed.
  DataMapMutator& SetToNullType(void);
  
  // New child will be appended to end of any current children.
  //   If the current node wasn't of a sort that could have children, it will
  //   be changed to such (and lose any data it previously held).
  // name [in]: can be NULL.
  // WARNING: This invalidates any DataMapReader/DataMapMutators that were
  //   referring to any of this one's node's children!
  DataMapMutator& CreateChild(char const* name = nullptr);
  
  DataMapMutator& CreateChildSafe(char const* name, size_t name_len);
  
  // name [in]: can be NULL.
  DataMapMutator& CreateAndGotoChild(char const* name = nullptr);
  
  DataMapMutator& CreateAndGotoChildSafe(char const* name, size_t name_len);
  
  void WriteName(char const* name);
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  void WriteNameSecure(char const* name, int size_in_elements);
  
  void Write(                  bool bool_value);
  void Write(char const* name, bool bool_value);
  void Write(                  int int_value);
  void Write(char const* name, int int_value);
  void Write(                  float float_value);
  void Write(char const* name, float float_value);
  void Write(                  char const* string_value);
  void Write(char const* name, char const* string_value);
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  void WriteSafe(char const* name, int name_size_in_elements,
                 bool bool_value);
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  void WriteSafe(char const* name, int name_size_in_elements,
                 int int_value);
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  
  void WriteSafe(char const* name, int name_size_in_elements,
                 float float_value);
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  void WriteSafe(
                 char const* string_value, int value_size_in_elements);
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  void WriteSafe(char const* name, int name_size_in_elements,
                 char const* string_value, int value_size_in_elements);
  
  // Calls ToNextSibling (theoretically slow) count times.
  // NOTE: Since this is a Mutator, children will be created if none exist
  //   to walk over.
  void Walk(int count);
  
  inline void WriteWalk(                  bool bool_value) {
    Write(bool_value);
    ToNextSibling();
  }
  
  inline void WriteWalk(char const* name, bool bool_value) {
    Write(name, bool_value);
    ToNextSibling();
  }
  
  inline void WriteWalk(                  int int_value) {
    Write(int_value);
    ToNextSibling();
  }
  
  inline void WriteWalk(char const* name, int int_value) {
    Write(name, int_value);
    ToNextSibling();
  }
  
  inline void WriteWalk(                  float float_value) {
    Write(float_value);
    ToNextSibling();
  }
  
  inline void WriteWalk(char const* name, float float_value) {
    Write(name, float_value);
    ToNextSibling();
  }
  
  inline void WriteWalk(                  char const* string_value) {
    Write(string_value);
    ToNextSibling();
  }
  
  inline void WriteWalk(char const* name, char const* string_value) {
    Write(name, string_value);
    ToNextSibling();
  }
  
	// size_in_elements should not include the NULL terminator.
	void WriteWalkSafeBooleanValue(char const* name,
	                               int name_size_in_elements,
	                               bool value);
	
	// size_in_elements should not include the NULL terminator.
	void WriteWalkSafeIntegerValue(char const* name, int name_size_in_elements,
	                               int value);
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  inline void WriteWalkSafeNullValue(char const* name,
                                     int name_size_in_elements) {
    WriteNameSecure(name, name_size_in_elements);
    SetToNullType();
    ToNextSibling();
  }
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  inline void WriteWalkSafe(char const* name, int name_size_in_elements,
                            bool bool_value) {
    WriteSafe(name, name_size_in_elements, bool_value);
    ToNextSibling();
  }
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  inline void WriteWalkSafe(char const* name, int name_size_in_elements,
                            int int_value) {
    WriteSafe(name, name_size_in_elements, int_value);
    ToNextSibling();
  }
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  inline void WriteWalkSafe(char const* name, int name_size_in_elements,
                            float float_value) {
    WriteSafe(name, name_size_in_elements, float_value);
    ToNextSibling();
  }
  
  // size_in_elements should not include the NULL terminator.  If new_string
  //  is too large, as much of it as possible will be copied, and the internal
  //  copy will be NULL-terminated.
  inline void WriteWalkSafe(char const* name, int name_size_in_elements,
                            char const* string_value,
                            int value_size_in_elements) {
    WriteSafe(name, name_size_in_elements,
              string_value, value_size_in_elements);
    ToNextSibling();
  }
  
  // Deletes this node's last ((count)) children.
  // NOTE: This invalidates any DataMapMutators/DataMapReaders currently
  //   referring to any deleted children or any of their children.
  void DeleteLastChildren(int count);
  
  //
  // Reading
  //
  // Some will have navigation side effects.
  //
  
  const char* ReadName(void) const;
  bool        ReadBool(void) const;
  int         ReadInt(void) const;
  float       ReadFloat(void) const;
  const char* ReadString(void) const;
  
  // there's no ReadNameSafe().  This would be to copy the name to a given
  //  buffer.
  
  // RETURNS: true on success (and the out parameter is written to).
  //          false otherwise, and the out parameter is not written to.
  bool ReadBoolSafe(bool* out_bool) const;
  
  // RETURNS: true on success (and the out parameter is written to).
  //          false otherwise, and the out parameter is not written to.
  bool ReadIntSafe(int* out_int) const;
  
  // RETURNS: true on success (and the out parameter is written to).
  //          false otherwise, and the out parameter is not written to.
  bool ReadFloatSafe(float* out_float) const;
  
  // RETURNS: true on success (and the out parameter is written to).
  //          false otherwise, and the out parameter is not written to.
  bool ReadStringSafe(char* out_string, int buffer_size_in_elements) const;
  
  inline bool ReadBoolWalk(void) {
    bool result = ReadBool();
    ToNextSibling();
    return result;
  }
  
  inline int ReadIntWalk(void) {
    int result = ReadInt();
    ToNextSibling();
    return result;
  }
  
  inline float ReadFloatWalk(void) {
    float result = ReadFloat();
    ToNextSibling();
    return result;
  }
  
  inline const char* ReadStringWalk(void) {
    const char* result = ReadString();
    ToNextSibling();
    return result;
  }
  
  inline bool ReadBoolWalkSafe(bool* out_bool) {
    bool result = ReadBoolSafe(out_bool);
    ToNextSibling();
    return result;
  }
  
  inline bool ReadIntWalkSafe(int* out_int) {
    bool result = ReadIntSafe(out_int);
    ToNextSibling();
    return result;
  }
  
  inline bool ReadFloatWalkSafe(float* out_float) {
    bool result = ReadFloatSafe(out_float);
    ToNextSibling();
    return result;
  }
  
  inline bool ReadStringWalkSafe(char* out_string,
      int buffer_size_in_elements) {
    bool result = ReadStringSafe(out_string, buffer_size_in_elements);
    ToNextSibling();
    return result;
  }
  
 private:
  DataNode* node_;
  // node_stack_ does *not* contain node_.
  std::vector<DataNode*> node_stack_;
};

} // namespace Core
