/*
    This file is part of Mithos
    Copyright (C) 2025 Lawrence Sebald

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3 as
    published by  the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define CINTERFACE
#include <ole2.h>
#include "d3d8.h"

/* From dllmain.cpp */
extern DWORD display;

static IDirect3D8Vtbl real_vtbl;
static IDirect3D8Vtbl hooked_vtbl;

HRESULT STDMETHODCALLTYPE hooked_CreateDevice(IDirect3D8 *This, UINT Adapter,
                                              D3DDEVTYPE DeviceType,
                                              HWND hFocusWindow,
                                              DWORD BehaviorFlags,
                                              D3DPRESENT_PARAMETERS *ppp,
                                              IDirect3DDevice8 **pprdi) {
    if(display > 0) {
        ppp->Windowed = TRUE;
        ppp->FullScreen_RefreshRateInHz = 0;
        ppp->FullScreen_PresentationInterval = 0;
    }

    return real_vtbl.CreateDevice(This, Adapter, DeviceType, hFocusWindow,
                                  BehaviorFlags, ppp, pprdi);
}

IDirect3D8 *hook_d3d8(IDirect3D8 *real) {
    /* Make a copy of the vtable and modify the function pointer(s) we care
       about in the copy, since they can't be modified in the original (it
       crashes if we try on Windows 11, at least). */
    hooked_vtbl = real_vtbl = *real->lpVtbl;
    hooked_vtbl.CreateDevice = &hooked_CreateDevice;
    real->lpVtbl = &hooked_vtbl;

    return real;
}
