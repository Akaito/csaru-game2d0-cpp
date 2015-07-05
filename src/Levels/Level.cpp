/*
The MIT License (MIT)

Copyright (c) 2015 Christopher Higgins Barrett

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Level.hpp"

#include <DataMap.hpp>
#include <JsonParserCallbackForDataMap.hpp>
#include <DataMapReaderSimple.hpp>

//==============================================================================
Level::Level () :
    m_width(0),
    m_height(0),
    m_tiles(nullptr)
{}

//==============================================================================
bool Level::BuildFromDatafile (const char * filepath) {
    
    Reset();

    CSaruContainer::DataMap                 dataMap;
    CSaruJson::JsonParserCallbackForDataMap callback(dataMap.GetMutator());

    FILE * file;
    fopen_s(&file, filepath, "rt");
    
    CSaruJson::JsonParser parser;
    if (!parser.ParseEntireFile(
        file,
        NULL,
        0,
        &callback
    )) {
        ASSERT(0 && "Failed to parse spritesheet file.");
        fclose(file);
        return false;
    }
    fclose(file);
    file = nullptr;
    
    CSaruContainer::DataMapReader reader = dataMap.GetReader();
    
    reader.ToChild("level");
    if (!reader.IsValid())
        return false;

    char  tempStr[512];
    WCHAR tempWStr[512];
        
    // Get spritesheet name
    reader.ToChild("name");
    if (!reader.IsValid())
        return false;
    if (!reader.ReadStringSafe(tempStr, arrsize(tempStr)))
        return false;
    m_name.clear();
    swprintf_s(tempWStr, L"%S", tempStr);
    m_name = tempWStr;

    // Get size and allocate
    reader.PopNode().ToChild("width");
    if (!reader.IsValid())
        return false;
    const unsigned width = reader.ReadInt();
    reader.PopNode().ToChild("height");
    if (!reader.IsValid())
        return false;
    const unsigned height = reader.ReadInt();

    if (!width || !height)
        return false;
    Resize(width, height);
        
    // Prepare to read in rows
    reader.PopNode().ToChild("rows");
    if (!reader.IsValid())
        return false;
    
    // Try reading in each row
    unsigned y = 0;
    for (reader.ToFirstChild(); reader.IsValid(); reader.ToNextSibling()) {
    
        CSaruContainer::DataMapReader rowReader(reader);
        rowReader.ToFirstChild();
        for (unsigned x = 0; x < width; ++x) {
            TileData & tileData = m_tiles[y * width + x];
            tileData.collision = static_cast<TileCollision>(rowReader.ReadIntWalk());
        }

        ++y;
    }
    
    // Store source filename
    for (const char * src = filepath; *src; ++src)
        m_sourceFilepath.push_back(*src);
    
    return true;

}

//==============================================================================
void Level::Reset () {

    delete [] m_tiles;
    m_tiles  = nullptr;

    m_width  = 0;
    m_height = 0;
    m_name.clear();
    m_sourceFilepath.clear();

}

//==============================================================================
bool Level::Resize (unsigned width, unsigned height) {

    if (width == m_width && height == m_height)
        return true;

    TileData * newTiles = new TileData [width * height];
    ASSERT(newTiles);
    if (!newTiles)
        return false;

    memset(newTiles, 0, sizeof(TileData) * width * height);

    if (m_tiles) {
        const unsigned minWidth  = MIN(m_width, width);
        const unsigned minHeight = MIN(m_height, height);
        for (unsigned y = 0; y < minHeight; ++y) {
            memcpy(newTiles + y * width, m_tiles + y * m_width, minWidth);
        }
    }

    m_width  = width;
    m_height = height;

    delete [] m_tiles;
    m_tiles = newTiles;

    return true;

}
