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

/*
#ifndef nullptr
#define nullptr 0
#endif
*/

#define ASSERT assert

namespace Core {


// djb2 hash from http://www.cse.yorku.ca/~oz/hash.html
//unsigned long Djb2Hash(unsigned char* str);
uint32_t Djb2Hash(const char* str);


// MS GUID
typedef GUID Uuid;
bool GenerateUuidV4 (Uuid * uuid);


/* C++11 provides a static_assert
//
// static_assert()  (if it doesn't already exist)
//

template <bool>
struct StaticAssert
{
};

template <>
struct StaticAssert<true>
{
  static const char A_STATIC_ASSERT_HAS_FAILED = 0;
};

// I still don't know why it's necessary to have two macros for this.
#if !defined(JOIN_AS_STRING)
#define USE__JOIN_AS_STRING__NOT_THIS_MACRO(a, b) a##b
#define JOIN_AS_STRING(a, b) USE__JOIN_AS_STRING__NOT_THIS_MACRO(a, b)
//#define JOIN_AS_STRING(a, b) a##b
#endif

// (comment and static_assert/StaticAssert from John Edwards'
//   template metaprogramming lecture)
// at first it seems that the enum shouldn't be necessary.  However, for this
//  to work at right about any place in code (namespace, class declaration,
//  template instantiation, and in functions) we need some way to scope this
//  thing
#if !defined(static_assert)
#define static_assert(test, msg) \
  enum { JOIN_AS_STRING(msg, __LINE__) = Core::StaticAssert<(test)>::A_STATIC_ASSERT_HAS_FAILED }
#endif
*/

} // namespace Core
