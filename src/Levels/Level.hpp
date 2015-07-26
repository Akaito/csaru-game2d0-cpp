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

#include "../graphics/SpriteAnimation.hpp"

class Level {
public: // Types and Constants
    enum class TileCollision : unsigned char {
        None = 0,
        Solid,
        BottomHalf,
        TERM
    };

    struct TileData {
        TileCollision collision;
        unsigned      legendIndex;
    };

    struct TileLegend {
        SpriteAnimation sprite;
        TileCollision   collision;

        TileLegend () :
            collision(TileCollision::None)
        {}
    };

private: // Data
    std::wstring            m_name;
    std::string             m_sourceFilepath;
    unsigned                m_width;
    unsigned                m_height;
    TileData *              m_tiles;
    std::vector<TileLegend> m_legend;

private: // Helpers
    bool Resize (unsigned width, unsigned height);
    void Reset ();

public:
    Level ();

    bool BuildFromDatafile (const char * filepath);
    void Reload ();

    void Update (float dt);
    void Render (const Transform & levelTransform);
};