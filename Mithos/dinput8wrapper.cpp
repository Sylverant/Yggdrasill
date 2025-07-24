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
#include <dinput.h>

/* From dllmain.cpp */
extern DWORD display;
extern HWND mainwindow;
extern BOOL (WINAPI *real_GetCursorPos)(LPPOINT);
extern BOOL is_wine;

static bool hooked_dinput8 = false;
static IDirectInput8AVtbl real_vtbl;
static IDirectInput8AVtbl hooked_vtbl;

static bool hooked_di8_kbd = false;
static IDirectInputDevice8AVtbl real_kb_vtbl;
static IDirectInputDevice8AVtbl hooked_kb_vtbl;

static bool hooked_di8_mouse = false;
static IDirectInputDevice8AVtbl real_mse_vtbl;
static IDirectInputDevice8AVtbl hooked_mse_vtbl;
static int m_oldbutton = 0;

HRESULT STDMETHODCALLTYPE kbd_SetCooperativeLevel(IDirectInputDevice8A *This,
                                                  HWND hWnd, DWORD dwFlags) {
    dwFlags = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND;

    if(display == 0)
        dwFlags |= DISCL_NOWINKEY;

    return real_kb_vtbl.SetCooperativeLevel(This, hWnd, dwFlags);
}

HRESULT STDMETHODCALLTYPE mse_SetCooperativeLevel(IDirectInputDevice8A *This,
                                                  HWND hWnd, DWORD dwFlags) {
    dwFlags = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND;

    return real_mse_vtbl.SetCooperativeLevel(This, hWnd, dwFlags);
}

HRESULT STDMETHODCALLTYPE mse_GetDeviceState(IDirectInputDevice8A *This,
                                             DWORD cbData, LPVOID lpvData) {
    HRESULT rv = real_mse_vtbl.GetDeviceState(This, cbData, lpvData);
    DIMOUSESTATE2 *ms = (DIMOUSESTATE2 *)lpvData;
    RECT wndRect;
    POINT point;

    /* We only care about the case where the user is clicking the mouse button
       for the first time. Basically, ignore these events if they're outside of
       the game's window. */
    if(m_oldbutton == 0 && ms->rgbButtons[0] != 0) {
        GetWindowRect(mainwindow, &wndRect);
        real_GetCursorPos(&point);

        if(point.x < wndRect.left || point.x > wndRect.right ||
            point.y < wndRect.top || point.y > wndRect.bottom) {
            ms->rgbButtons[0] = 0;
            ms->rgbButtons[1] = 0;
        }

        /* The game will call GetCursorPos() in order to keep track of the
           cursor position, so we don't have to mess with it here. */
    }

    m_oldbutton = ms->rgbButtons[0];
    return rv;
}

static HRESULT STDMETHODCALLTYPE hooked_CreateDevice(IDirectInput8A *This,
                                                     REFGUID rguid,
                                                     LPDIRECTINPUTDEVICE8A *did,
                                                     LPUNKNOWN pUnkOuter) {
    HRESULT rv = real_vtbl.CreateDevice(This, rguid, did, pUnkOuter);

    /* We have to hook differently on Windows and Wine for the same reason as
       in hook_dinput8() below. */
    if(rv == DI_OK) {
        if(IsEqualGUID(rguid, GUID_SysKeyboard)) {
            if(!hooked_di8_kbd)
                real_kb_vtbl = *(*did)->lpVtbl;

            if(!is_wine) {
                (*did)->lpVtbl->SetCooperativeLevel = &kbd_SetCooperativeLevel;
            }
            else {
                hooked_kb_vtbl = *(*did)->lpVtbl;
                hooked_kb_vtbl.SetCooperativeLevel = &kbd_SetCooperativeLevel;
                (*did)->lpVtbl = &hooked_kb_vtbl;
            }
            hooked_di8_kbd = true;
        }
        else if(IsEqualGUID(rguid, GUID_SysMouse)) {
            if(!hooked_di8_mouse)
                real_mse_vtbl = *(*did)->lpVtbl;

            if(!is_wine) {
                (*did)->lpVtbl->SetCooperativeLevel = &mse_SetCooperativeLevel;
                (*did)->lpVtbl->GetDeviceState = &mse_GetDeviceState;
            }
            else {
                hooked_mse_vtbl = *(*did)->lpVtbl;
                hooked_mse_vtbl.SetCooperativeLevel = &mse_SetCooperativeLevel;
                hooked_mse_vtbl.GetDeviceState = &mse_GetDeviceState;
                (*did)->lpVtbl = &hooked_mse_vtbl;
            }

            hooked_di8_mouse = true;
        }
    }

    return rv;
}

IDirectInput8A *hook_dinput8(IDirectInput8A *real) {
    /* On Windows, unlike with DirectX8, DirectInput8 requires us to modify the
       vtable in-place, otherwise things break. However, Wine apparently makes
       the vtable const, so we can't modify it there and trying to do so makes
       the game crash... Sigh... */
    if(!hooked_dinput8)
        real_vtbl = *real->lpVtbl;

    if(!is_wine) {
        real->lpVtbl->CreateDevice = &hooked_CreateDevice;
    }
    else {
        hooked_vtbl = *real->lpVtbl;
        hooked_vtbl.CreateDevice = &hooked_CreateDevice;
        real->lpVtbl = &hooked_vtbl;

    }

    hooked_dinput8 = true;

    return real;
}
