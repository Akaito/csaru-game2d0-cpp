#pragma once

#pragma warning(disable: 4800) // Forcing value to blue 'true' or 'false' (performance warning)

#include <assert.h>

#include <D3D11.h>
#include <D3DX11.h>
#include <DxErr.h>

#if !defined(DIRECTINPUT_VERSION)
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include <xnamath.h>
#include <XInput.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <map>

#include "Utils.h"

#include "Dx11DemoBase.hpp"
#include "graphics/GraphicsMgr.h"
#include "Transform.h"

#define MIN_SIGNED_SHORT   -32768
#define MAX_SIGNED_SHORT    32767
#define MAX_UNSIGNED_SHORT  65535

#define MAX_UNSIGNED_CHAR 255
