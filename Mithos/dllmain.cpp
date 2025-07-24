/*
    This file is part of Mithos
    Copyright (C) 2012, 2013, 2025 Lawrence Sebald

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

#include "pch.h"
#pragma comment(lib, "imm32.lib")

static HWND (WINAPI *real_CreateWindowExA)(DWORD, LPCSTR, LPCSTR, DWORD, int,
                                           int, int, int, HWND, HMENU,
                                           HINSTANCE, LPVOID) = CreateWindowExA;
static BOOL (WINAPI *real_SetForegroundWindow)(HWND hWnd) = SetForegroundWindow;
static BOOL (WINAPI *real_SetCursorPos)(int, int) = SetCursorPos;
BOOL (WINAPI *real_GetCursorPos)(LPPOINT) = GetCursorPos;
static WNDPROC orig_WndProc = NULL;

static IDirect3D8 *(WINAPI *real_Direct3DCreate8)(UINT) = NULL;
static HRESULT (WINAPI *real_DirectInput8Create)(HINSTANCE, DWORD, REFIID,
                                                 LPVOID *, LPUNKNOWN) = NULL;

static BOOL hooked_all = FALSE;
static std::string server_host;
static DWORD disableime, mapfix, musicpatch, v1names, cusshack, showMouse;
DWORD display, winheight = 768, winwidth = 1024;
BOOL is_wine = FALSE;

HWND mainwindow;

/* https://list.winehq.org/pipermail/wine-devel/2008-September/069387.html */
static void detect_wine(void) {
    HMODULE hnd = GetModuleHandleA("ntdll.dll");

    if(!hnd)
        return;

    if(GetProcAddress(hnd, "wine_get_version"))
        is_wine = TRUE;
}

static LSTATUS open_reg_key(HKEY &key) {
    LPCTSTR k = TEXT("Software\\Sylverant\\Yggdrasill\\Mithos");
    return RegOpenKeyEx(HKEY_CURRENT_USER, k, 0, KEY_READ, &key);
}

static LSTATUS reg_str(HKEY key, LPCSTR val, std::string &out) {
    DWORD t;
    LSTATUS err;
    CHAR data[256];
    DWORD len = 256;

    ZeroMemory(data, len);
    err = RegQueryValueExA(key, val, NULL, &t, (LPBYTE)data, &len);

    if((err == ERROR_SUCCESS && t != REG_SZ) || len > 256)
        return ERROR_APP_DATA_CORRUPT;

    out = data;
    return err;
}

static LSTATUS reg_dword(HKEY key, LPCSTR val, DWORD &out) {
    DWORD t;
    LSTATUS err;
    DWORD len = sizeof(out);

    err = RegQueryValueExA(key, val, NULL, &t, (LPBYTE)&out, &len);
    if((err == ERROR_SUCCESS && t != REG_DWORD) || len > sizeof(DWORD))
        return ERROR_APP_DATA_CORRUPT;

    return err;
}

static bool find_hooked_functions(void) {
    void *f;

    if(!(f = DetourFindFunction("d3d8.dll", "Direct3DCreate8")))
        return false;
    real_Direct3DCreate8 = (IDirect3D8 *(WINAPI *)(UINT))f;

    if(!(f = DetourFindFunction("dinput8.dll", "DirectInput8Create")))
        return false;
    real_DirectInput8Create = (HRESULT (WINAPI *)(HINSTANCE, DWORD, REFIID,
                                                  LPVOID *, LPUNKNOWN))f;

    return true;
}

static bool read_prefs(void) {
    HKEY key;

    if(open_reg_key(key) != ERROR_SUCCESS)
        return false;

    if(reg_str(key, "ServerAddr", server_host) != ERROR_SUCCESS)
        return false;

    if(reg_dword(key, "DisableIME", disableime) != ERROR_SUCCESS)
        disableime = 1;

    if(reg_dword(key, "DisplayMode", display) != ERROR_SUCCESS)
        display = 0;

    if(reg_dword(key, "ShowMouse", showMouse) != ERROR_SUCCESS)
        showMouse = 0;

    if(reg_dword(key, "WindowHeight", winheight) != ERROR_SUCCESS)
        winheight = 960;

    if(reg_dword(key, "WindowWidth", winwidth) != ERROR_SUCCESS)
        winwidth = 1280;

    if(reg_dword(key, "MusicPatch", musicpatch) != ERROR_SUCCESS)
        musicpatch = 0;

    if(reg_dword(key, "MapFix", mapfix) != ERROR_SUCCESS)
        mapfix = 0;

    if(reg_dword(key, "V1Names", v1names) != ERROR_SUCCESS)
        v1names = 0;

    if(reg_dword(key, "Cusshack", cusshack) != ERROR_SUCCESS)
        cusshack = 0;

    RegCloseKey(key);
    return true;
}

static bool get_section_hdrs(DWORD &sechdr, WORD &numSecs) {
    IMAGE_DOS_HEADER *mz;
    IMAGE_FILE_HEADER *pe;
    DWORD peaddr, tmp;

    /* Read the MZ (DOS) header */
    mz = (IMAGE_DOS_HEADER *)0x00400000;

    if(mz->e_magic != IMAGE_DOS_SIGNATURE)
        return false;

    peaddr = (DWORD)mz->e_lfanew + 0x00400000;

    /* Read the PE (Windows) header */
    memcpy(&tmp, (const void *)peaddr, 4);
    if(tmp != IMAGE_NT_SIGNATURE)
        return false;

    peaddr += 4;
    pe = (IMAGE_FILE_HEADER *)peaddr;

    if(pe->Machine != IMAGE_FILE_MACHINE_I386)
        return false;

    sechdr = peaddr + pe->SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER);
    numSecs = pe->NumberOfSections;
    return true;
}

static uint32_t find_string(DWORD secSz, const uint8_t *data,
                            const char *search) {
    const uint8_t *s = data;
    size_t len = strlen(search) + 1;

    while(s < data + secSz - len) {
        if(!memcmp(s, search, len))
            return (uint32_t)s;
        ++s;
    }

    return 0;
}

static uint32_t find_ptr(DWORD secSz, const uint8_t *data, uint32_t search,
                         uint32_t st) {
    const uint8_t *s = data;

    if(st)
        s = (const uint8_t *)(st + 1);

    while(s < data + secSz + 4) {
        if(!memcmp(s, &search, 4))
            return (uint32_t)s;
        ++s;
    }

    return 0;
}

#define NUM_SERVER_ADDRS 6

static const char *server_addrs[NUM_SERVER_ADDRS] = {
    "pso20.sonic.isao.net",
    "sg207634.sonicteam.com",
    "pso-mp01.sonic.isao.net",
    "gsproduc.ath.cx",
    "psobb.dyndns.org",
    "sylverant.net"
};

static bool patch_server_addr(DWORD secSz, uint8_t *data) {
    DWORD saddr, eaddr;
    int num_srvs = 0, i;
    size_t lens[NUM_SERVER_ADDRS], maxlen = 0, inc;

    saddr = 0;
    eaddr = secSz;

    for(i = 0; i < NUM_SERVER_ADDRS; ++i) {
        lens[i] = strlen(server_addrs[i]);

        if(lens[i] > maxlen)
            maxlen = lens[i];
    }

    while(saddr < eaddr + maxlen && num_srvs < 3) {
        inc = 1;

        for(i = 0; i < NUM_SERVER_ADDRS; ++i) {
            if(!memcmp(data + saddr, server_addrs[i], lens[i])) {
                memcpy(data + saddr, server_host.c_str(),
                       server_host.length() + 1);
                ++num_srvs;
                inc = lens[i];
                break;
            }
        }

        saddr += inc;
    }

    if(num_srvs == 3)
        return true;

    return false;
}

static bool music_patch(DWORD secSz, uint8_t *data) {
    uint32_t mambo, chu, duel1, duel2, mamboptr, chu2byo;
    
    if(!(mambo = find_string(secSz, data, "mambo.adx")))
        return false;

    if(!(chu = find_string(secSz, data, "chu_f.adx")))
        return false;

    if(!(duel1 = find_string(secSz, data, "duel1.adx")))
        return false;

    if(!(duel2 = find_string(secSz, data, "duel2.adx")))
        return false;

    /* If these aren't found, then the patch is probably already applied. */
    mamboptr = find_ptr(secSz, data, mambo, 0);
    chu2byo = find_ptr(secSz, data, chu, 0);

    if(!mamboptr && !chu2byo)
        return true;
    else if(!mamboptr || !chu2byo)
        return false;

    memcpy((void *)mamboptr, &duel1, 4);
    memcpy((void *)chu2byo, &duel2, 4);
    return true;
}

#define NUM_MAPS 4
static const char *maps[NUM_MAPS] = {
    "map_acave01_05",
    "map_acave03_05",
    "map_amachine01_05",
    "map_amachine02_05"
};

static uint8_t *map_patch_one(DWORD secSz, uint8_t *data, const char *mapbase,
                              const char *map, uint8_t *stab, uint32_t mapptr) {
    uint32_t map00_loc, mapbase_loc, map00_ptr, map00_ptr_ptr, val;

    /* Find all the pointers we care about... 
       map00_loc = location of mapbase_00 string (i.e, map_acave01_00)
       mapbase_loc = location of mapbase string (i.e, map_acave01)
       map00_ptr = location of pointer to map00_loc
       map00_ptr_ptr = location of pointer to pointer to map00_loc */
    if(!(map00_loc = find_string(secSz, data, map)))
        return NULL;

    if(!(mapbase_loc = find_string(secSz, data, mapbase)))
        return NULL;

    if(!(map00_ptr = find_ptr(secSz, data, map00_loc, 0)))
        return NULL;

    if(!(map00_ptr_ptr = find_ptr(secSz, data, map00_ptr - 4, 0)))
        return NULL;

    /* Copy out the existing mapset to the block of memory we allocated in
       the map_patch function. */
    memcpy(stab, (void *)(map00_ptr - 4), 40);

    /* Update the in-game pointer tables to point to the new block with the
       correct number of maps. */
    val = (uint32_t)stab;
    memcpy((void *)map00_ptr_ptr, &val, 4);
    val = 6;
    memcpy((void *)(map00_ptr_ptr + 4), &val, 4);

    stab += 40;
    memcpy(stab, &mapbase_loc, 4);
    memcpy(stab + 4, &mapptr, 4);

    return stab + 8;
}

static bool map_patch(DWORD secSz, uint8_t *data) {
    uint8_t *stab, *mp;
    int i;
    uint8_t *maplocs[NUM_MAPS];

    /* Check if the user has already applied the mapfix patch manually.
       Assume that if the cave 1 map set is full that all of them are. */
    if(find_string(secSz, data, "map_acave01_05"))
        return true;

    /* Make space to put our strings... */
    stab = (uint8_t *)VirtualAlloc(NULL, 512, MEM_COMMIT, PAGE_READWRITE);
    if(!stab)
        return false;

    mp = stab;
    for(i = 0; i < NUM_MAPS; ++i) {
        size_t len = strlen(maps[i]) + 1;

        maplocs[i] = mp;
        memcpy(mp, maps[i], len);
        mp += len;
    }

    /* Update the map tables for ultimate difficulty... Don't bother
       cleaning up if anything breaks because the game will probably be
       very broken anyway. */
    mp = map_patch_one(secSz, data, "map_acave01", "map_acave01_00", mp,
                       (uint32_t)maplocs[0]);
    if(!mp)
        return false;

    mp = map_patch_one(secSz, data, "map_acave03", "map_acave03_00", mp,
                       (uint32_t)maplocs[1]);
    if(!mp)
        return false;

    mp = map_patch_one(secSz, data, "map_amachine01", "map_amachine01_00", mp,
                       (uint32_t)maplocs[2]);
    if(!mp)
        return false;

    mp = map_patch_one(secSz, data, "map_amachine02", "map_amachine02_00", mp,
                       (uint32_t)maplocs[3]);
    if(!mp)
        return false;

    return true;
}

static bool name_patch(DWORD secSz, uint8_t *data) {
    uint32_t ptr = 0;

    while((ptr = find_ptr(secSz, data, 0xffffae35, ptr))) {
        /* See if this is one of the two "mov dword ptr [edi+18h], ffffae35h"
           occurences, and if it is, if it is the right one -- that is to say
           the one followed immediately by a call instruction... */
        uint8_t *p = (uint8_t *)ptr;

        if(p[-3] == 0xc7 && p[-2] == 0x47 && p[-1] == 0x18 && p[4] == 0xe8) {
            DWORD old;

            /* Make sure we can write to the page... */
            if(!VirtualProtectEx(GetCurrentProcess(), p, 2,
                                 PAGE_EXECUTE_READWRITE, &old))
                return false;

            p[0] = 0xff;
            p[1] = 0xff;

            /* Re-protect the page properly. */
            if(!VirtualProtectEx(GetCurrentProcess(), p, 2, old, &old))
                return false;

            return true;
        }
    }

    return false;
}

static const BYTE SECTION_TEXT[8] = { '.', 't', 'e', 'x', 't' };
static const BYTE SECTION_DATA[8] = { '.', 'd', 'a', 't', 'a' };
static const BYTE SECTION_DATA1[8] = { '.', 'd', 'a', 't', 'a', '1' };
static const BYTE SECTION_RDATA[8] = { '.', 'r', 'd', 'a', 't', 'a' };

static bool cusshack_patch(IMAGE_SECTION_HEADER *hdrs, WORD ns) {
    WORD i;
    uint32_t strloc = 0, ptrloc;
    static const char censor[] = "#!@%#!@%#!@%#!@%#!@%#!@%#!@%#!@%#!@%";

    for(i = 0; i < ns; ++i) {
        if(!memcmp(hdrs[i].Name, SECTION_DATA, 8)) {
            uint8_t *data = (uint8_t *)(0x00400000 + hdrs[i].VirtualAddress);

            if(!(strloc = find_string(hdrs[i].SizeOfRawData, data, censor)))
                return false;

            break;
        }
    }

    /* This should never happen, but just to make the compiler happy... */
    if(strloc == 0)
        return false;

    for(i = 0; i < ns; ++i) {
        if(!memcmp(hdrs[i].Name, SECTION_TEXT, 8)) {
            uint8_t *data = (uint8_t *)(0x00400000 + hdrs[i].VirtualAddress);
            uint8_t *ptr;

            if(!(ptrloc = find_ptr(hdrs[i].SizeOfRawData, data, strloc, 0)))
                return false;

            /* See if the instruction located 46 bytes up from the pointer is a
               jz instruction as we expect. */
            ptr = (uint8_t *)ptrloc;
            if(ptr[-46] == 0x74) {
                DWORD old;

                /* Turn it into two nops instead. */
                if(!VirtualProtectEx(GetCurrentProcess(), ptr - 46, 2,
                                     PAGE_EXECUTE_READWRITE, &old))
                    return false;

                ptr[-46] = 0x90;
                ptr[-45] = 0x90;

                if(!VirtualProtectEx(GetCurrentProcess(), ptr - 46, 2, old,
                                     &old))
                    return false;

                return true;
            }

            break;
        }
    }

    return false;
}

static bool patch_pso(void) {
    DWORD shdr;
    WORD ns, i;
    IMAGE_SECTION_HEADER *hdrs;

    detect_wine();

    if(!get_section_hdrs(shdr, ns)) {
        MessageBoxA(NULL, "Failed to patch read program headers",
                    "Failed to read program headers", 0);
        return false;
    }

    /* Go to the array of section headers... */
    hdrs = (IMAGE_SECTION_HEADER *)shdr;

    /* Apply patches to each section as needed. Yes, this is not pretty. */
    for(i = 0; i < ns; ++i) {
        if(!memcmp(hdrs[i].Name, SECTION_DATA, 8)) {
            uint8_t *data = (uint8_t *)(0x00400000 + hdrs[i].VirtualAddress);

            if(!patch_server_addr(hdrs[i].SizeOfRawData, data)) {
                MessageBoxA(NULL, "Failed to patch server address", "FAIL", 0);
                return false;
            }

            if(musicpatch && !music_patch(hdrs[i].SizeOfRawData, data)) {
                MessageBoxA(NULL, "Failed to apply music patch", "FAIL", 0);
                return false;
            }

            if(mapfix && !map_patch(hdrs[i].SizeOfRawData, data)) {
                MessageBoxA(NULL, "Failed to apply mapfix patch", "FAIL", 0);
                return false;
            }
        }
        else if(!memcmp(hdrs[i].Name, SECTION_TEXT, 8)) {
            uint8_t *data = (uint8_t *)(0x00400000 + hdrs[i].VirtualAddress);

            if(v1names && !name_patch(hdrs[i].SizeOfRawData, data)) {
                MessageBoxA(NULL, "Failed to apply V1/GC name color patch",
                            "FAIL", 0);
                return false;
            }
        }
    }

    if(cusshack && !cusshack_patch(hdrs, ns)) {
        MessageBoxA(NULL, "Failed to apply cusshack patch", "FAIL", 0);
        return false;
    }

    return true;
}

static HWND fsWnd = NULL;

LRESULT WINAPI hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                              LPARAM lParam) {
    switch(uMsg) {
        case WM_SETCURSOR:
            if(showMouse) {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                ShowCursor(TRUE);
            }
            break;

        case WM_ACTIVATEAPP:
            if(!fsWnd)
                break;

            if(wParam == WA_INACTIVE) {
                ShowWindow(hWnd, SW_MINIMIZE);

                if(fsWnd)
                    ShowWindow(fsWnd, SW_MINIMIZE);
            }
            else {
                if(fsWnd)
                    ShowWindow(fsWnd, SW_SHOWNORMAL);

                ShowWindow(hWnd, SW_SHOWNORMAL);
            }

            break;
    }

    return CallWindowProcA(orig_WndProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK FS_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) {
    switch(uMsg) {
        /* TODO: Somehow this breaks alt+tab slightly... so... yeah. */
        //case WM_ACTIVATE:
        //    /* When the background hider window activates, make sure it is
        //       showing properly, then bring the main window to the top of
        //       the Z order, activating it. This way, when the user clicks
        //       the game icon in the taskbar, it gets focus back immediately. */
        //    if(wParam != WA_INACTIVE && mainwindow) {
        //        ShowWindow(hWnd, SW_SHOWNORMAL);
        //        BringWindowToTop(mainwindow);
        //    }

        //    return 0;

        /* Ignore any mouse click events. */
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:
            return 0;

        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;

        case WM_DESTROY:
            return 0;
    }

    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI hider_window_thread(LPVOID lpParam) {
    WNDCLASSEXA  wndclass;
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HINSTANCE hInstance = (HINSTANCE)lpParam;
    MSG msg;
    HINSTANCE mod = GetModuleHandleA("Mithos.dll");
    HICON icon = LoadIconA(mod, MAKEINTRESOURCEA(IDI_ICON1));

    wndclass.cbSize = sizeof(wndclass);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = FS_WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = icon;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "FSHider";
    wndclass.hIconSm = icon;

    RegisterClassExA(&wndclass);

    fsWnd = real_CreateWindowExA(0, "FSHider", "PSO", WS_POPUP, 0, 0, w,
                                 h, NULL, NULL, hInstance, NULL);
    ShowWindow(fsWnd, SW_SHOWNORMAL);
    UpdateWindow(fsWnd);

    /* Don't send mouse events to the background window... */
    EnableWindow(fsWnd, FALSE);

    while(GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);						
        DispatchMessageA(&msg);
    }

    UnregisterClassA("FSHider", hInstance);
    return 0;
}

HWND WINAPI hooked_CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName,
                                   LPCSTR lpWindowName, DWORD dwStyle, int X,
                                   int Y, int nWidth, int nHeight,
                                   HWND hWndParent, HMENU hMenu,
                                   HINSTANCE hInstance, LPVOID lpParam) {
    HWND rv;

    /* This is just as good of a time as any to apply our patches... */
    if(!patch_pso())
        return NULL;

    /* If we're in windowed mode, calculate the width/height of the window
       based on the specified client area. That way if the user specifies
       say... 1280x960, that's actually what the game display is. */
    if(display == 3) {
        RECT r = { 0, 0, (LONG)winwidth, (LONG)winheight };

        dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        dwExStyle = 0;

        if(!AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle))
            return NULL;

        nWidth = (int)r.right - r.left;
        nHeight = (int)r.bottom - r.top;
    }
    /* This is simple borderless fullscreen mode. Stretch things. */
    else if(display == 2) {
        nWidth = GetSystemMetrics(SM_CXSCREEN);
        nHeight = GetSystemMetrics(SM_CYSCREEN);

        dwStyle = WS_POPUP;
        dwExStyle = 0;
    }
    /* If we're in borderless fullscreen window mode, figure out our window
       size and start up the background window. */
    else if(display == 1) {
        int w = GetSystemMetrics(SM_CXSCREEN);
        int h = GetSystemMetrics(SM_CYSCREEN);

        dwStyle = WS_POPUP;
        dwExStyle = 0;

        nWidth = (int)(h / 3.0 * 4.0);
        nHeight = h;
        X = (w - nWidth) / 2;
        Y = 0;

        /* If we don't have a 4x3 display, then create a window to hide the
           rest of the screen and display the game window on top of it. */
        if(nWidth != w) {
            CreateThread(NULL, 0, hider_window_thread, hInstance, 0, NULL);
            Sleep(100);

            hWndParent = fsWnd;
        }
    }

    rv = real_CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle,
                              X, Y, nWidth, nHeight, hWndParent, hMenu,
                              hInstance, lpParam);

    if(disableime) {
        // https://ftp.zx.net.nz/pub/Patches/ftp.microsoft.com/MISC/KB/en-us/
        // 171/154.HTM
        // Because ImmDisableIME apparently crashes now.
        ImmAssociateContext(rv, NULL);
    }

    /* Save a reference to the window since we might need it for mouse event
       handling in windowed mode. Good thing PSO only calls this once. */
    mainwindow = rv;
    EnableWindow(mainwindow, TRUE);

    HINSTANCE mod = GetModuleHandleA("Mithos.dll");
    HICON icon = LoadIconA(mod, MAKEINTRESOURCEA(IDI_ICON1));

    SetClassLongPtrA(rv, GCLP_HICONSM, (LONG_PTR)icon);
    SetClassLongPtrA(rv, GCLP_HICON, (LONG_PTR)icon);

    /* Hook the WndProc */
    orig_WndProc = (WNDPROC)SetWindowLongPtrA(rv, GWLP_WNDPROC,
                                              (LONG_PTR)hooked_WndProc);

    return rv;
}

BOOL WINAPI hooked_SetForegroundWindow(HWND hWnd) {
    if(display)
        return TRUE;

    return real_SetForegroundWindow(hWnd);
}

IDirect3D8 *WINAPI hooked_Direct3DCreate8(UINT version) {
    IDirect3D8 *real = real_Direct3DCreate8(version);

    return hook_d3d8(real);
}

HRESULT WINAPI hooked_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion,
                                         REFIID riidltf, LPVOID *ppvOut,
                                         LPUNKNOWN punkOuter) {
    HRESULT rv = real_DirectInput8Create(hinst, dwVersion, riidltf,
                                         ppvOut, punkOuter);
    *ppvOut = hook_dinput8((IDirectInput8A *)*ppvOut);
    return rv;
}

BOOL WINAPI hooked_SetCursorPos(int X, int Y) {
    if(display == 0)
        return real_SetCursorPos(X, Y);

    return TRUE;
}

BOOL WINAPI hooked_GetCursorPos(LPPOINT lpPoint) {
    POINT loc;
    BOOL rv = real_GetCursorPos(&loc);

    if(rv && display != 0) {
        RECT wndRect;
        GetWindowRect(mainwindow, &wndRect);

        /* Map the cursor pos to a normal 640x480 window */
        if(loc.x < wndRect.left || loc.x > wndRect.right ||
           loc.y < wndRect.top || loc.y > wndRect.bottom) {
            loc.x = 320;
            loc.y = 280;
        }
        else {
            float arx = 640.0f / (wndRect.right - wndRect.left);
            float ary = 480.0f / (wndRect.bottom - wndRect.top);

            loc.x = (int)((loc.x - wndRect.left) * arx);
            loc.y = (int)((loc.y - wndRect.top) * ary);
        }

        *lpPoint = loc;
    }

    return rv;
}

BOOL APIENTRY DllMain(HMODULE hInst, DWORD dwReason, LPVOID lpReserved) {
    if(DetourIsHelperProcess()) {
        return TRUE;
    }

    switch(dwReason) {
        case DLL_PROCESS_ATTACH:
            DetourRestoreAfterWith();

            if(!find_hooked_functions() || !read_prefs()) {
                MessageBoxA(NULL, "Failed to set up dll. No patching done.",
                            "FAIL", 0);
                return TRUE;
            }

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID &)real_CreateWindowExA,
                         hooked_CreateWindowExA);
            DetourAttach(&(PVOID &)real_SetForegroundWindow,
                         hooked_SetForegroundWindow);
            DetourAttach(&(PVOID &)real_SetCursorPos,
                         hooked_SetCursorPos);
            DetourAttach(&(PVOID &)real_GetCursorPos,
                         hooked_GetCursorPos);
            DetourAttach(&(PVOID &)real_Direct3DCreate8,
                         hooked_Direct3DCreate8);
            DetourAttach(&(PVOID &)real_DirectInput8Create,
                         hooked_DirectInput8Create);
            DetourTransactionCommit();
            hooked_all = TRUE;
            break;

        case DLL_PROCESS_DETACH:
            if(!hooked_all)
                return TRUE;

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourDetach(&(PVOID &)real_CreateWindowExA,
                         hooked_CreateWindowExA);
            DetourDetach(&(PVOID &)real_SetForegroundWindow,
                         hooked_SetForegroundWindow);
            DetourDetach(&(PVOID &)real_SetCursorPos,
                         hooked_SetCursorPos);
            DetourAttach(&(PVOID &)real_GetCursorPos,
                         hooked_GetCursorPos);
            DetourDetach(&(PVOID &)real_Direct3DCreate8,
                         hooked_Direct3DCreate8);
            DetourDetach(&(PVOID &)real_DirectInput8Create,
                         hooked_DirectInput8Create);
            DetourTransactionCommit();
            break;
    }

    return TRUE;
}

