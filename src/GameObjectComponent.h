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

// GameObjectComponents must be guaranteed to not change address or delete while attached to a GameObject.
class GameObjectComponent {
public: // Types
    typedef unsigned GlobalTypeId;
    static const unsigned s_InvalidGlobalTypeId = 0;

protected: // Data
    GlobalTypeId m_typeId;
    GameObject * m_owner;

public: // Methods
    GameObjectComponent (GlobalTypeId typeId = s_InvalidGlobalTypeId) : m_typeId(typeId), m_owner(nullptr)
    {}

    GameObjectComponent (unsigned short moduleId, unsigned short componentTypeId) :
        m_typeId(moduleId << 16 | componentTypeId),
        m_owner(nullptr)
    {}

    virtual ~GameObjectComponent () {}

    virtual void Update (float dt) { ref(dt); }
    virtual void Render ()         {}

    GlobalTypeId GetGlobalTypeId () const      { return m_typeId; }
    unsigned short  GetLocalTypeId () const       { return m_typeId & 0xFFFF; }
    unsigned short  GetModuleId () const          { return m_typeId >> 16; }
    GameObject *    GetOwner ()                   { return m_owner; }
    void            SetOwner (GameObject * owner) { m_owner = owner; }
};
