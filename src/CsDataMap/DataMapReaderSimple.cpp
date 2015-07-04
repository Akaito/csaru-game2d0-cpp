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

#include "DataMapReaderSimple.h"

namespace Core {

//==============================================================================
DataMapReaderSimple::DataMapReaderSimple (const Core::DataMapReader & reader) :
    m_reader(reader),
    m_errorDepth(0)
{
}

//==============================================================================
DataMapReaderSimple::~DataMapReaderSimple () {
}

//==============================================================================
bool DataMapReaderSimple::Bool (const char * name) const {

    DataMapReader temp(m_reader);
    temp.ToChild(name);
    ASSERT(temp.IsValid());

    bool result;
    if (!m_reader.ReadBoolSafe(&result)) {
        ASSERT(0 && "Non-bool node!");
        result = false;
    }
    
    return result;

}

//==============================================================================
bool DataMapReaderSimple::Bool (const char * name, bool defaultValue) const {

    DataMapReader temp(m_reader);
    temp.ToChild(name);
    
    if (!temp.IsValid())
        return defaultValue;

    bool result;
    if (!temp.ReadBoolSafe(&result))
        result = defaultValue;
    
    return result;

}

//==============================================================================
bool DataMapReaderSimple::EnterArray (const char * name) {

    if (!ToChild(name))
        return false;
        
    if (!ToFirstChild()) {
        ToParent();
        return false;
    }
    
    return true;

}

//==============================================================================
void DataMapReaderSimple::ExitArray () {

    if (!ToParent()) {
        ASSERT(0 && "Misuse of ExitArray; no parent.");
        return;
    }

    if (!ToParent())
        ASSERT(0 && "Misuse of ExitArray; no grand-parent.");

}

//==============================================================================
float DataMapReaderSimple::Float (const char * name) const {

    DataMapReader temp(m_reader);
    temp.ToChild(name);
    ASSERT(temp.IsValid());

    float result;
    if (!temp.ReadFloatSafe(&result)) {
        ASSERT(0 && "Non-float node!");
        result = 0.0f;
    }
    
    return result;

}

//==============================================================================
float DataMapReaderSimple::Float (const char * name, float defaultValue) const {

    DataMapReader temp(m_reader);
    temp.ToChild(name);
    
    if (!temp.IsValid())
        return defaultValue;
    
    float result;
    if (!temp.ReadFloatSafe(&result))
        result = defaultValue;
    
    return result;

}

//==============================================================================
int DataMapReaderSimple::Int (const char * name) const {

    DataMapReader temp(m_reader);
    temp.ToChild(name);
    ASSERT(temp.IsValid());

    int result;
    if (!temp.ReadIntSafe(&result)) {
        ASSERT(0 && "Non-int node!");
        result = 0;
    }
    
    return result;

}

//==============================================================================
int DataMapReaderSimple::Int (const char * name, int defaultValue) const {

    DataMapReader temp(m_reader);
    temp.ToChild(name);
    
    if (!temp.IsValid())
        return defaultValue;
    
    int result;
    if (!temp.ReadIntSafe(&result))
        result = defaultValue;
    
    return result;

}

//==============================================================================
bool DataMapReaderSimple::IsValid () const {

    if (m_errorDepth)
        return false;
    
    return m_reader.IsValid();

}

//==============================================================================
std::string DataMapReaderSimple::String (const char * name) const {

    DataMapReader temp(m_reader);
    temp.ToChild(name);
    ASSERT(temp.IsValid());

    char result[128];
    if (!temp.ReadStringSafe(result, sizeof(result)/sizeof(result[0]))) {
        ASSERT(0 && "Non-string node!");
        return "ERROR";
    }
    
    return result;

}

//==============================================================================
std::string DataMapReaderSimple::String (
    const char * name,
    const std::string & defaultValue
) const {

    DataMapReader temp(m_reader);
    temp.ToChild(name);
    
    if (!temp.IsValid())
        return defaultValue;
    
    char result[128];
    if (!temp.ReadStringSafe(result, sizeof(result)/sizeof(result[0])))
        return defaultValue;
    
    return result;

}

//==============================================================================
bool DataMapReaderSimple::ToChild (const char * name) {

    // If already invalid, just increase error depth so we can wait to come
    // back out of an erroneous state.
    if (!IsValid()) {
        ++m_errorDepth;
        return false;
    }

    m_reader.ToChild(name);
    
    if (!m_reader.IsValid()) {
        ++m_errorDepth;
        return false;
    }
    
    return true;

}

//==============================================================================
bool DataMapReaderSimple::ToFirstChild () {

    // If already invalid, just increase error depth so we can wait to come
    // back out of an erroneous state.
    if (!IsValid()) {
        ++m_errorDepth;
        return false;
    }

    m_reader.ToFirstChild();
    
    if (!m_reader.IsValid()) {
        ++m_errorDepth;
        return false;
    }
    
    return true;

}

//==============================================================================
bool DataMapReaderSimple::ToNextSibling () {

    if (!IsValid())
        return false;

    m_reader.ToNextSibling();
    
    return m_reader.IsValid();

}

//==============================================================================
bool DataMapReaderSimple::ToParent () {

    // Reduce error depth, and see if we're back into a good state again.
    if (m_errorDepth) {
        --m_errorDepth;
        return m_errorDepth == 0;
    }

    m_reader.PopNode();
    
    if (!m_reader.IsValid()) {
        --m_errorDepth;
        return false;
    }
    
    return true;

}

//==============================================================================
std::wstring DataMapReaderSimple::WString (const char * name) const {

    DataMapReader temp(m_reader);
    temp.ToChild(name);
    ASSERT(temp.IsValid());

    char result[128];
    if (!temp.ReadStringSafe(result, sizeof(result)/sizeof(result[0]))) {
        ASSERT(0 && "Non-string node!");
        return L"ERROR";
    }
    
    std::wstring wresult;
    for (const char * c = result; *c; ++c)
        wresult.push_back(*c);
    
    return wresult;

}

//==============================================================================
std::wstring DataMapReaderSimple::WString (
    const char * name,
    const std::wstring & defaultValue
) const {

    DataMapReader temp(m_reader);
    temp.ToChild(name);
    
    if (!temp.IsValid())
        return defaultValue;
    
    char result[128];
    if (!temp.ReadStringSafe(result, sizeof(result)/sizeof(result[0])))
        return defaultValue;
    
    std::wstring wresult;
    for (const char * c = result; *c; ++c)
        wresult.push_back(*c);
    
    return wresult;

}

} // namespace Core
