// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include <winsock.h>
#include <detours/detours.h>
#include <winreg.h>
#include <imm.h>
#include <winnt.h>
#include <initguid.h>
#include "d3d8.h"
#include "d3d8wrapper.h"
#include <dinput.h>
#include "dinput8wrapper.h"
#include <shellapi.h>
#include "resource.h"

#include <string>

#endif //PCH_H
