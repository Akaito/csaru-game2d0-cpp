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

#include "BitmapFont.h"
#include "../CsDataMap/DataMap.h"
#include "../CsDataMap/Json/JsonParserCallbackForDataMap.h"
#include "../CsDataMap/DataMapReaderSimple.h"

//==============================================================================
BitmapFont::BitmapFont () {
}

//==============================================================================
BitmapFont::~BitmapFont () {
}

//==============================================================================
bool BitmapFont::BuildFromDataFile (const WCHAR * filepath) {

    Clear();

    m_sourceFile = filepath;
    
    Core::DataMap                      dataMap;
    Core::JsonParserCallbackForDataMap callback(dataMap.GetMutator());
    
    Core::JsonParser2 parser;
    if (!parser.ParseEntireFile(
        filepath,
        NULL,
        0,
        &callback
    )) {
        ASSERT(0 && "Failed to parse spritesheet file.");
        return false;
    }
    
    Core::DataMapReader       reader       = dataMap.GetReader();
    Core::DataMapReaderSimple simpleReader = reader;
    
    /*
    reader.ToChild("spritesheet");
    if (!reader.IsValid())
        return false;
        
    // Get spritesheet name
    reader.ToChild("name");
    if (!reader.IsValid())
        return false;
    reader.ReadWString(&m_name);
    
    // Get image filepath
    reader.PopNode().ToChild("imageFile");
    if (!reader.IsValid()) {
        Reset();
        return false;
    }
    reader.ReadWString(&m_imageFilepath);
        
    // Prepare to read in animations
    reader.PopNode().ToChild("animations");
    if (!reader.IsValid()) {
        Reset();
        return false;
    }
    
    // Try reading in each animation
    for (reader.ToFirstChild(); reader.IsValid(); reader.ToNextSibling()) {
    
        Core::DataMapReader animReader(reader);
        m_animations.push_back(Animation());
        Animation & anim = m_animations.back();
        
        anim.FromDataMap(reader);
    }
    
    if (!PrepareGraphicsResources())
        return false;
    
    */
    return true;

}

//==============================================================================
void BitmapFont::Clear () {

    m_sourceFile.clear();

}
