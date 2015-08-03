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

enum EGocType {
    GOC_TYPE_INVALID = 0,
    GOC_TYPE_COMP_TEST,
    GOC_TYPE_GAMEPAD,
    GOC_TYPE_SPRITE,
    GOC_TYPE_DEBUG_LINES,
    GOC_TYPE_LEVEL,
    GOC_TYPE_CAMERA,

    // ActionGame Algorithm Maniax
    GOC_TYPE_LEVER_DASH_MAN,
    GOC_TYPE_JUMP,

    GOC_TYPES
};

// GameObjectComponents must be guaranteed to not change address or delete while attached to a GameObject.
class GameObjectComponent {
protected: // Types and data
    EGocType     m_type;
    GameObject * m_owner;

public: // Methods
    GameObjectComponent () : m_type(GOC_TYPE_INVALID), m_owner(nullptr) {
    }

    virtual ~GameObjectComponent () {}

    virtual void Update (float dt) { ref(dt); }
    virtual void Render ()         {}

    EGocType     GetType ()                    { return m_type; }
    GameObject * GetOwner ()                   { return m_owner; }
    void         SetOwner (GameObject * owner) { m_owner = owner; }
};
