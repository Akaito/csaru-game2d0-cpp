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

#include "../Utils.h"

// TODO: Speed up Reader by maintaining the current child's index?

namespace Core {

class DataNode;
//class DataMapMutator;

class DataMapReader {
 public:
  explicit DataMapReader(const DataNode* node);
  DataMapReader(const DataMapReader& other);
  DataMapReader& operator=(const DataMapReader& rhs);
  
  //DataMapReader& CopyMutator(const DataMapMutator* mutator);
 
  inline const DataNode* GetCurrentNode(void) const
      { return node_; }
  
  inline const DataNode* GetCurrentNode(void)
      { return node_; }
  
  // RETURNS: -1 if invalidated, 0 if at the root node, 1 if at one of the root
  //  node's children, and so on.
  inline int GetCurrentDepth(void) const
      { return int(node_stack_.size() - 1 + (node_ == NULL ? 0 : 1)); }
  
  inline bool IsValid(void) const
      { return node_ != NULL; }
  
  ///////
  // navigation (begin)
  
  // return to the parent node.
  // NOTE: If this is used on the root node, the Reader becomes invalidated.
  DataMapReader& PopNode(void);
  
  // if there are no children, the current node will become NULL.  You must
  //  PopNode to back out of this state.
  DataMapReader& ToFirstChild(void);
  
  // if there are no children, the current node will become NULL.  You must
  //  PopNode to back out of this state.
  DataMapReader& ToLastChild(void);
  
  // if there is no child at the given index, the current node will become
  //  NULL.  You must PopNode to back out of this state.
  DataMapReader& ToChild(int index);
  
  // if no child with such a name exists, the current node will become
  //  NULL.  You must PopNode to back out of this state.
  DataMapReader& ToChild(const char* name);
  
  // theoretically slow.  Nodes have no knowledge of their siblings, so we
  //  actually PopNode(-ish), then ToChild(prev_index + 1).  prev_index must
  //  also be calculated at this time by walking the current parent's children.
  // NOTE: If this is used on the root node, the Reader becomes invalidated.
  DataMapReader& ToNextSibling(void);
  
  // theoretically slow.  Nodes have no knowledge of their siblings, so we
  //  actually PopNode(-ish), then ToChild(prev_index - 1).  prev_index must
  //  also be calculated at this time by walking the current parent's children.
  // NOTE: If this is used on the root node, the Reader becomes invalidated.
  DataMapReader& ToPreviousSibling(void);
  
  // navigation (end)
  ///////
  // reading (begin)
  
  const char* ReadName(void) const;
  bool        ReadBool(void) const;
  int         ReadInt(void) const;
  float       ReadFloat(void) const;
  const char* ReadString(void) const;
  void        ReadWString(std::wstring * wstrOut) const; // TODO : Deprecate this by always using wide chars!
  
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
  
  // reading (end)
  ///////
  
 protected:
  void PushNode(const DataNode* node);
  
  const DataNode* node_;
  std::vector<const DataNode*> node_stack_;
  
 private:
};

} // namespace Core
