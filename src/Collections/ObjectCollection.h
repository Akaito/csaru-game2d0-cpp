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

namespace CSaru {

struct IObjectCollection {
    struct Handle {
        unsigned index;
        unsigned generation;

        Handle () : index(unsigned(-1)), generation(0) {}
    };

    struct Entry {
        unsigned generation;
        void *   object;
    };

    virtual ~IObjectCollection () {};
};


template <typename T_ObjectType, unsigned T_Capacity, bool T_ObjectTypeIsPod = false>
class CObjectCollection : public IObjectCollection {
    static_assert(
        sizeof(T_ObjectType *) == sizeof(void *),
        "ObjectLibrary only supports objects with pointers castable to void*."
    );

private: // Data and types
    Entry    m_entries[T_Capacity];
    // [0, m_freeEntryIndex) are allocated.  [m_freeEntryIndex, T_Capacity) are free.
    unsigned m_entryIndices[T_Capacity];
    unsigned m_freeEntryIndex;

protected: // Helpers
    void Remove (unsigned objectIndex) {
        if (!T_ObjectTypeIsPod)
            delete static_cast<T_ObjectType *>(m_entries[m_entryIndices[objectIndex]].object);
        m_entries[m_entryIndices[objectIndex]].object = nullptr;

        unsigned tailEntryIndex = m_entryIndices[--m_freeEntryIndex];
        m_entryIndices[m_freeEntryIndex] = m_entryIndices[objectIndex];
        m_entryIndices[objectIndex]      = tailEntryIndex;
    }

    void ResetEntryIndices () {
        for (unsigned i = 0; i < T_Capacity; ++i)
            m_entryIndices[i] = i;
    }

public: // Methods
    CObjectCollection () : m_freeEntryIndex(0) {
        memset(m_entries, 0, sizeof(m_entries));
        ResetEntryIndices();
    }

    virtual ~CObjectCollection () {
        RemoveAll();
    }

    unsigned Count () const    { return m_freeEntryIndex; }
    unsigned Capacity () const { return T_Capacity; }

    Handle Add (T_ObjectType * object) {
        Handle handle;
        if (m_freeEntryIndex >= T_Capacity)
            return handle;

        Entry & entry = m_entries[m_entryIndices[m_freeEntryIndex]];
        ++entry.generation;
        entry.object = object;

        handle.index      = m_entryIndices[m_freeEntryIndex++];
        handle.generation = entry.generation;
        return handle;
    }

    T_ObjectType * Get (const Handle & handle) {
        if (handle.index >= T_Capacity)
            return nullptr;

        Entry & entry = m_entries[handle.index];
        if (entry.generation != handle.generation)
            return nullptr;

        return static_cast<T_ObjectType *>(entry.object);
    }

    void Remove (const Handle & handle) {
        if (!Get(handle))
            return;

        Remove(handle.index);
    }

    void RemoveAll () {
        while (m_freeEntryIndex)
            Remove(0);
        ResetEntryIndices();
    }
};

};