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

#include "DataMapReader.h"

namespace Core {

class DataMapReaderSimple {

private: // Data

    DataMapReader m_reader;
    int           m_errorDepth;

public: // Construction

    DataMapReaderSimple (const Core::DataMapReader & reader);
    virtual ~DataMapReaderSimple ();
    
public: // Queries / Commands

    DataMapReader GetReader () const { return m_reader; }
    
    bool IsValid () const;
    
    bool ToChild (const char * name);
    bool ToFirstChild ();
    bool ToNextSibling ();
    bool ToParent ();
    
    bool EnterArray (const char * name); // Go's to child of <name>'s first child
    void ExitArray (); // Go to parent's parent
    
    bool Bool (const char * name) const;
    bool Bool (const char * name, bool defaultValue) const;
    
    int Int (const char * name) const;
    int Int (const char * name, int defaultValue) const;
    
    float Float (const char * name) const;
    float Float (const char * name, float defaultValue) const;
    
    std::string String (const char * name) const;
    std::string String (const char * name, const std::string & defaultValue) const;
    
    std::wstring WString (const char * name) const;
    std::wstring WString (const char * name, const std::wstring & defaultValue) const;

};

} // namespace Core
