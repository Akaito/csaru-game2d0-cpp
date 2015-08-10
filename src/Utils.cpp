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
// (Note exception to above in Djb2Hash below.)

#include "Utils.h"

namespace Core
{

std::uint32_t Djb2Hash (const char * str)
{
  // code *exactly* from http://www.cse.yorku.ca/~oz/hash.html
  //   EXCEPTION: Changed "unsigned long" to "uint32_t".  C++'s standard states
  //   only unsigned long's range, not its size in bits or bytes.  That range
  //   corresponds to a 4-byte unsigned integer (if each byte is 8 bits).
  std::uint32_t hash = 5381;
  int c;
  
  while (c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  
  return hash;
}

} // namespace Core
