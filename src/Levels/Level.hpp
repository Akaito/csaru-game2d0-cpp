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

#pragma once

class Level {
public: // Types and Constants
    enum class ETileCollision : unsigned char {
        None = 0,
        Solid,
        BottomHalf,
        TERM
    };

    struct TileData {
        ETileCollision collision;
        unsigned      legendIndex;
    };

    struct TileLegend {
        SpriteAnimation sprite;
        ETileCollision   collision;

        TileLegend () :
            collision(ETileCollision::None)
        {}
    };

private: // Data
    std::wstring            m_name;
    std::string             m_sourceFilepath;
    unsigned                m_width;
    unsigned                m_height;
    TileData *              m_tiles;
    std::vector<TileLegend> m_legend;
    unsigned                m_tileWidth;
    unsigned                m_tileHeight;

private: // Helpers
    bool Resize (unsigned width, unsigned height);
    void Reset ();

public:
    Level ();

    bool BuildFromDatafile (const char * filepath);
    void Reload ();

    unsigned GetWidthInTiles () const  { return m_width; }
    unsigned GetHeightInTiles () const { return m_height; }
    unsigned GetTileWidth () const     { return m_tileWidth; }
    unsigned GetTileHeight () const    { return m_tileHeight; }
    const TileData * GetTileData (unsigned x, unsigned y) const;
    const TileData * EnumTileData (unsigned * indexInOut) const;
    bool GetTilePos (unsigned tileIndex, unsigned * xOut, unsigned * yOut) const;

    void Update (float dt);
    void Render (const Transform & levelTransform);
};