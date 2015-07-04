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

// this file implements Utils.hpp functions.  But, this cpp file is only
//   meant for POSIX-based systems.
#ifndef _MSC_VER

#include "Utils.h"

namespace codesaru {

// thanks to
//  http://publib.boulder.ibm.com/infocenter/zos/v1r12/index.jsp?topic=/com.ibm.zos.r12.bpxbd00/rtsys.htm
//  for help with what to #include for sysconf()
#define _POSIX_SOURCE
#include <unistd.h>

namespace codesaru {

// thanks to http://en.wikipedia.org/wiki/Page_(computer_memory)
//  for C-based examples on how to do this
int32_t GetSystemPageSize(void) {
  return static_cast<int32_t>(sysconf(_SC_PAGESIZE));
}

} // namespace codesaru

#endif
