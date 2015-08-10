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
//   meant for the Windows OS.
#ifdef _MSC_VER

#include "Utils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Objbase.h> // CoCreateGuid()

namespace Core {

// thanks to http://en.wikipedia.org/wiki/Page_(computer_memory)
//  for C-based examples on how to do this
int GetSystemPageSize(void) {
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  return system_info.dwPageSize;
}


bool GenerateUuidV4 (Uuid * uuid) {
    GUID msGuid;
    if (S_OK != CoCreateGuid(&msGuid)) {
        memset(uuid, 0, sizeof(*uuid));
        return false;
    }

    static_assert(sizeof(Uuid) == sizeof(GUID), "Uuid must match MS GUID in size!");
    memcpy(uuid, &msGuid, sizeof(*uuid));

    return true;
}

} // namespace Core

// clean up after ourselves
#undef WIN32_LEAN_AND_MEAN
#endif
