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

#include "JsonGenerator.h"
#include "../DataMapReader.h"
#include "../DataNode.h"

#if _MSC_VER > 1000
#pragma warning(push)
// unsafe functions warning, such as fopen()
#pragma warning(disable:4996)
#endif

namespace Core {

bool JsonGenerator::WriteToFile(DataMapReader* reader, char const* filename) {
  // check for NULL reader
  if (reader == NULL) {
#ifdef _DEBUG
    fprintf(stderr, "JsonGenerator::WriteToFile() called, "
            "but reader == NULL.\n");
#endif
    return false;
  }
  // check for NULL filename
  if (filename == NULL) {
#ifdef _DEBUG
    fprintf(stderr, "JsonGenerator::WriteToFile() called, "
            "but filename == NULL.\n");
#endif
    return false;
  }
  
  FILE* file = fopen(filename, "wt");
  // check for successful fopen
  if (file == NULL) {
#ifdef _DEBUG
    fprintf(stderr, "JsonGenerator::WriteToFile() failed to open desired file."
            "  File was [%s].\n", filename);
#endif
    return false;
  }
  
  bool write_result = WriteToStream(reader, file);
  
  fclose(file);
  return write_result;
}


bool JsonGenerator::WriteToStream(DataMapReader* reader, FILE* file) {
  // check for NULL reader
  if (reader == NULL) {
#ifdef _DEBUG
    fprintf(stderr, "JsonGenerator::WriteToFile() called, "
            "but reader == NULL.\n");
#endif
    return false;
  }
  
  // check for successful fopen
  if (file == NULL) {
#ifdef _DEBUG
    fprintf(stderr, "JsonGenerator::WriteToStream() was given a NULL file "
            "pointer.\n");
#endif
    return false;
  }
  
  bool write_result = WriteJsonToFile(file, reader, false);
  return write_result;
}


bool JsonGenerator::WriteIndent(FILE* file, int indent_amount) {
  for (int i = 0;  i < indent_amount;  ++i)
    fprintf(file, " ");
  return true;
}


bool JsonGenerator::WriteJsonToFile(FILE* file, DataMapReader* reader,
                                    bool current_node_writes_name) {
  // indent
  WriteIndent(file, reader->GetCurrentDepth() * 2);
  // write name if node isn't root, and its parent isn't an array
  if (current_node_writes_name) {
    //fprintf(file, "\"%s\": ", reader->ReadName());
		fprintf(file, "\"");
		WriteEscapedStringToFile(file, reader->ReadName());
		fprintf(file, "\": ");
	}
  // write data based on current node type
  switch (reader->GetCurrentNode()->GetType()) {
    case DataNode::kNull:
      fprintf(file, "null");
      break;
    
    case DataNode::kObject: {
      fprintf(file, "{\n");
      // objects tend to have children, print them if this one has any
      if (reader->GetCurrentNode()->HasChildren()) {
	      reader->ToFirstChild();
	      /*bool result =*/ WriteJsonToFile(file, reader, true);
	      reader->PopNode();
      }
      // terminate object
      WriteIndent(file, reader->GetCurrentDepth() * 2);
      fprintf(file, "}");
      break;
    }
    
    case DataNode::kArray: {
      fprintf(file, "[\n");
      // arrays tend to have children, print them if this one has any
      if (reader->GetCurrentNode()->HasChildren()) {
	      reader->ToFirstChild();
	      /*bool result =*/ WriteJsonToFile(file, reader, false);
	      reader->PopNode();
      }
      // terminate array
      WriteIndent(file, reader->GetCurrentDepth() * 2);
      fprintf(file, "]");
      break;
    }
    
    case DataNode::kBool:
      if (reader->ReadBool())
        fprintf(file, "true");
      else
        fprintf(file, "false");
      break;
    
    case DataNode::kInt:
      fprintf(file, "%d", reader->ReadInt());
      break;
    
    case DataNode::kFloat:
      fprintf(file, "%f", reader->ReadFloat());
      break;
    
    case DataNode::kString:
			fprintf(file, "\"");
			WriteEscapedStringToFile(file, reader->ReadString());
			fprintf(file, "\"");
      break;
  }
  
  // write siblings, if there are any
  if (reader->ToNextSibling().IsValid()) {
    fprintf(file, ",\n");
    if (!WriteJsonToFile(file, reader, current_node_writes_name))
      return false;
  // otherwise, just terminate the current line
  } else {
    fprintf(file, "\n");
  }
  return true;
}


void JsonGenerator::WriteEscapedStringToFile(FILE* file, const char* string) {
	while (*string) {
		switch (*string) {
			case '"':
				fprintf(file, "\\\"");
				break;
			
			case '\\':
				fprintf(file, "\\\\");
				break;
			
			case 0x08: // backspace
				fprintf(file, "\\b");
				break;
			
			case 0x0C: // formfeed
				fprintf(file, "\\f");
				break;
			
			case 0x0A: // newline
				fprintf(file, "\\n");
				break;
			
			case 0x0D: // carriage return
				fprintf(file, "\\r");
				break;
			
			case 0x09: // horizontal tab
				fprintf(file, "\\t");
				break;
			
			default:
	      fprintf(file, "%c", *string);
    }
    ++string;
  }
}

} // namespace Core

#if _MSC_VER > 1000
#pragma warning(pop)
#endif
