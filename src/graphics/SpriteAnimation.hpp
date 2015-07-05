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

struct SpritesheetFrame;

class SpriteAnimation {
private: // Data
    Spritesheet * m_spritesheet;
    unsigned      m_animIndex;
    unsigned      m_frameIndex;
    float         m_timeOnFrameSeconds;

public:
    SpriteAnimation () :
        m_spritesheet(nullptr),
        m_animIndex(0),
        m_frameIndex(0),
        m_timeOnFrameSeconds(0.0f)
    {}

    SpriteAnimation (const SpriteAnimation & other) :
        m_spritesheet(other.m_spritesheet),
        m_animIndex(other.m_animIndex),
        m_frameIndex(other.m_frameIndex),
        m_timeOnFrameSeconds(other.m_timeOnFrameSeconds)
    {}

    // Commands
    void Update (float dt);
    void Render (const XMMATRIX & world, const XMMATRIX & viewProjection);

    void SetSheet (Spritesheet * sheet);
    void SetFrameIndex (unsigned index);
    void SetAnimIndex (unsigned index);

    // Queries
    Spritesheet * GetSheet () { return m_spritesheet; }

    unsigned GetAnimationIndex () const { return m_animIndex; }

    const SpritesheetFrame * GetCurrentFrame () const;

};
