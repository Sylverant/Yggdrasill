/* Cut down version of d3d8.h/d3d8types.h/d3d8caps.h from the Microsoft DirectX SDK */

/*==========================================================================;
 *
 *  Copyright (C) Microsoft Corporation.  All Rights Reserved.
 *
 *  File:   d3d8.h
 *  Content:    Direct3D include file
 *
 ****************************************************************************/

#ifndef _D3D8_H_
#define _D3D8_H_

#ifndef DIRECT3D_VERSION
#define DIRECT3D_VERSION         0x0800
#endif  //DIRECT3D_VERSION

 // include this file content only if compiling for DX8 interfaces
#if(DIRECT3D_VERSION >= 0x0800)


/* This identifier is passed to Direct3DCreate8 in order to ensure that an
 * application was built against the correct header files. This number is
 * incremented whenever a header (or other) change would require applications
 * to be rebuilt. If the version doesn't match, Direct3DCreate8 will fail.
 * (The number itself has no meaning.)*/

#define D3D_SDK_VERSION 220


#include <stdlib.h>

#define COM_NO_WINDOWS_H
#include <objbase.h>

#include <windows.h>

#if !defined(HMONITOR_DECLARED) && (WINVER < 0x0500)
#define HMONITOR_DECLARED
DECLARE_HANDLE(HMONITOR);
#endif

#define D3DAPI WINAPI

/*
 * Interface IID's
 */
#if defined( _WIN32 ) && !defined( _NO_COM)

 /* IID_IDirect3D8 */
 /* {1DD9E8DA-1C77-4d40-B0CF-98FEFDFF9512} */
DEFINE_GUID(IID_IDirect3D8, 0x1dd9e8da, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0x95, 0x12);

#endif

#ifdef __cplusplus

interface IDirect3D8;
interface IDirect3DDevice8;

interface IDirect3DResource8;
interface IDirect3DBaseTexture8;
interface IDirect3DTexture8;
interface IDirect3DVolumeTexture8;
interface IDirect3DCubeTexture8;

interface IDirect3DVertexBuffer8;
interface IDirect3DIndexBuffer8;

interface IDirect3DSurface8;
interface IDirect3DVolume8;

interface IDirect3DSwapChain8;

#endif


typedef interface IDirect3D8                IDirect3D8;
typedef interface IDirect3DDevice8          IDirect3DDevice8;
typedef interface IDirect3DResource8        IDirect3DResource8;
typedef interface IDirect3DBaseTexture8     IDirect3DBaseTexture8;
typedef interface IDirect3DTexture8         IDirect3DTexture8;
typedef interface IDirect3DVolumeTexture8   IDirect3DVolumeTexture8;
typedef interface IDirect3DCubeTexture8     IDirect3DCubeTexture8;
typedef interface IDirect3DVertexBuffer8    IDirect3DVertexBuffer8;
typedef interface IDirect3DIndexBuffer8     IDirect3DIndexBuffer8;
typedef interface IDirect3DSurface8         IDirect3DSurface8;
typedef interface IDirect3DVolume8          IDirect3DVolume8;
typedef interface IDirect3DSwapChain8       IDirect3DSwapChain8;

/********************************************************
*
*                      d3d8types.h
*
*********************************************************/

#include <float.h>

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201) // anonymous unions warning
#if defined(_X86_) || defined(_IA64_)
#pragma pack(4)
#endif

/* Direct3D8 Device types */
typedef enum _D3DDEVTYPE {
    D3DDEVTYPE_HAL = 1,
    D3DDEVTYPE_REF = 2,
    D3DDEVTYPE_SW = 3,

    D3DDEVTYPE_FORCE_DWORD = 0x7fffffff
} D3DDEVTYPE;

/* Multi-Sample buffer types */
typedef enum _D3DMULTISAMPLE_TYPE {
    D3DMULTISAMPLE_NONE = 0,
    D3DMULTISAMPLE_2_SAMPLES = 2,
    D3DMULTISAMPLE_3_SAMPLES = 3,
    D3DMULTISAMPLE_4_SAMPLES = 4,
    D3DMULTISAMPLE_5_SAMPLES = 5,
    D3DMULTISAMPLE_6_SAMPLES = 6,
    D3DMULTISAMPLE_7_SAMPLES = 7,
    D3DMULTISAMPLE_8_SAMPLES = 8,
    D3DMULTISAMPLE_9_SAMPLES = 9,
    D3DMULTISAMPLE_10_SAMPLES = 10,
    D3DMULTISAMPLE_11_SAMPLES = 11,
    D3DMULTISAMPLE_12_SAMPLES = 12,
    D3DMULTISAMPLE_13_SAMPLES = 13,
    D3DMULTISAMPLE_14_SAMPLES = 14,
    D3DMULTISAMPLE_15_SAMPLES = 15,
    D3DMULTISAMPLE_16_SAMPLES = 16,

    D3DMULTISAMPLE_FORCE_DWORD = 0x7fffffff
} D3DMULTISAMPLE_TYPE;


#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |       \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif /* defined(MAKEFOURCC) */


typedef enum _D3DFORMAT {
    D3DFMT_UNKNOWN = 0,

    D3DFMT_R8G8B8 = 20,
    D3DFMT_A8R8G8B8 = 21,
    D3DFMT_X8R8G8B8 = 22,
    D3DFMT_R5G6B5 = 23,
    D3DFMT_X1R5G5B5 = 24,
    D3DFMT_A1R5G5B5 = 25,
    D3DFMT_A4R4G4B4 = 26,
    D3DFMT_R3G3B2 = 27,
    D3DFMT_A8 = 28,
    D3DFMT_A8R3G3B2 = 29,
    D3DFMT_X4R4G4B4 = 30,
    D3DFMT_A2B10G10R10 = 31,
    D3DFMT_G16R16 = 34,

    D3DFMT_A8P8 = 40,
    D3DFMT_P8 = 41,

    D3DFMT_L8 = 50,
    D3DFMT_A8L8 = 51,
    D3DFMT_A4L4 = 52,

    D3DFMT_V8U8 = 60,
    D3DFMT_L6V5U5 = 61,
    D3DFMT_X8L8V8U8 = 62,
    D3DFMT_Q8W8V8U8 = 63,
    D3DFMT_V16U16 = 64,
    D3DFMT_W11V11U10 = 65,
    D3DFMT_A2W10V10U10 = 67,

    D3DFMT_UYVY = MAKEFOURCC('U', 'Y', 'V', 'Y'),
    D3DFMT_YUY2 = MAKEFOURCC('Y', 'U', 'Y', '2'),
    D3DFMT_DXT1 = MAKEFOURCC('D', 'X', 'T', '1'),
    D3DFMT_DXT2 = MAKEFOURCC('D', 'X', 'T', '2'),
    D3DFMT_DXT3 = MAKEFOURCC('D', 'X', 'T', '3'),
    D3DFMT_DXT4 = MAKEFOURCC('D', 'X', 'T', '4'),
    D3DFMT_DXT5 = MAKEFOURCC('D', 'X', 'T', '5'),

    D3DFMT_D16_LOCKABLE = 70,
    D3DFMT_D32 = 71,
    D3DFMT_D15S1 = 73,
    D3DFMT_D24S8 = 75,
    D3DFMT_D16 = 80,
    D3DFMT_D24X8 = 77,
    D3DFMT_D24X4S4 = 79,


    D3DFMT_VERTEXDATA = 100,
    D3DFMT_INDEX16 = 101,
    D3DFMT_INDEX32 = 102,

    D3DFMT_FORCE_DWORD = 0x7fffffff
} D3DFORMAT;

/* Display Modes */
typedef struct _D3DDISPLAYMODE {
    UINT            Width;
    UINT            Height;
    UINT            RefreshRate;
    D3DFORMAT       Format;
} D3DDISPLAYMODE;

/* SwapEffects */
typedef enum _D3DSWAPEFFECT {
    D3DSWAPEFFECT_DISCARD = 1,
    D3DSWAPEFFECT_FLIP = 2,
    D3DSWAPEFFECT_COPY = 3,
    D3DSWAPEFFECT_COPY_VSYNC = 4,

    D3DSWAPEFFECT_FORCE_DWORD = 0x7fffffff
} D3DSWAPEFFECT;


/* RefreshRate pre-defines */
#define D3DPRESENT_RATE_DEFAULT         0x00000000
#define D3DPRESENT_RATE_UNLIMITED       0x7fffffff


/* Resize Optional Parameters */
typedef struct _D3DPRESENT_PARAMETERS_ {
    UINT                BackBufferWidth;
    UINT                BackBufferHeight;
    D3DFORMAT           BackBufferFormat;
    UINT                BackBufferCount;

    D3DMULTISAMPLE_TYPE MultiSampleType;

    D3DSWAPEFFECT       SwapEffect;
    HWND                hDeviceWindow;
    BOOL                Windowed;
    BOOL                EnableAutoDepthStencil;
    D3DFORMAT           AutoDepthStencilFormat;
    DWORD               Flags;

    /* Following elements must be zero for Windowed mode */
    UINT                FullScreen_RefreshRateInHz;
    UINT                FullScreen_PresentationInterval;

} D3DPRESENT_PARAMETERS;

// Values for D3DPRESENT_PARAMETERS.Flags

#define D3DPRESENTFLAG_LOCKABLE_BACKBUFFER  0x00000001

/* Types */
typedef enum _D3DRESOURCETYPE {
    D3DRTYPE_SURFACE = 1,
    D3DRTYPE_VOLUME = 2,
    D3DRTYPE_TEXTURE = 3,
    D3DRTYPE_VOLUMETEXTURE = 4,
    D3DRTYPE_CUBETEXTURE = 5,
    D3DRTYPE_VERTEXBUFFER = 6,
    D3DRTYPE_INDEXBUFFER = 7,


    D3DRTYPE_FORCE_DWORD = 0x7fffffff
} D3DRESOURCETYPE;

#define MAX_DEVICE_IDENTIFIER_STRING        512
typedef struct _D3DADAPTER_IDENTIFIER8 {
    char            Driver[MAX_DEVICE_IDENTIFIER_STRING];
    char            Description[MAX_DEVICE_IDENTIFIER_STRING];

#ifdef _WIN32
    LARGE_INTEGER   DriverVersion;            /* Defined for 32 bit components */
#else
    DWORD           DriverVersionLowPart;     /* Defined for 16 bit driver components */
    DWORD           DriverVersionHighPart;
#endif

    DWORD           VendorId;
    DWORD           DeviceId;
    DWORD           SubSysId;
    DWORD           Revision;

    GUID            DeviceIdentifier;

    DWORD           WHQLLevel;

} D3DADAPTER_IDENTIFIER8;

#pragma pack()
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif

/********************************************************
*
*                      d3d8caps.h
* 
*********************************************************/

#if defined(_X86_) || defined(_IA64_)
#pragma pack(4)
#endif

typedef struct _D3DCAPS8 {
    /* Device Info */
    D3DDEVTYPE  DeviceType;
    UINT    AdapterOrdinal;

    /* Caps from DX7 Draw */
    DWORD   Caps;
    DWORD   Caps2;
    DWORD   Caps3;
    DWORD   PresentationIntervals;

    /* Cursor Caps */
    DWORD   CursorCaps;

    /* 3D Device Caps */
    DWORD   DevCaps;

    DWORD   PrimitiveMiscCaps;
    DWORD   RasterCaps;
    DWORD   ZCmpCaps;
    DWORD   SrcBlendCaps;
    DWORD   DestBlendCaps;
    DWORD   AlphaCmpCaps;
    DWORD   ShadeCaps;
    DWORD   TextureCaps;
    DWORD   TextureFilterCaps;          // D3DPTFILTERCAPS for IDirect3DTexture8's
    DWORD   CubeTextureFilterCaps;      // D3DPTFILTERCAPS for IDirect3DCubeTexture8's
    DWORD   VolumeTextureFilterCaps;    // D3DPTFILTERCAPS for IDirect3DVolumeTexture8's
    DWORD   TextureAddressCaps;         // D3DPTADDRESSCAPS for IDirect3DTexture8's
    DWORD   VolumeTextureAddressCaps;   // D3DPTADDRESSCAPS for IDirect3DVolumeTexture8's

    DWORD   LineCaps;                   // D3DLINECAPS

    DWORD   MaxTextureWidth, MaxTextureHeight;
    DWORD   MaxVolumeExtent;

    DWORD   MaxTextureRepeat;
    DWORD   MaxTextureAspectRatio;
    DWORD   MaxAnisotropy;
    float   MaxVertexW;

    float   GuardBandLeft;
    float   GuardBandTop;
    float   GuardBandRight;
    float   GuardBandBottom;

    float   ExtentsAdjust;
    DWORD   StencilCaps;

    DWORD   FVFCaps;
    DWORD   TextureOpCaps;
    DWORD   MaxTextureBlendStages;
    DWORD   MaxSimultaneousTextures;

    DWORD   VertexProcessingCaps;
    DWORD   MaxActiveLights;
    DWORD   MaxUserClipPlanes;
    DWORD   MaxVertexBlendMatrices;
    DWORD   MaxVertexBlendMatrixIndex;

    float   MaxPointSize;

    DWORD   MaxPrimitiveCount;          // max number of primitives per DrawPrimitive call
    DWORD   MaxVertexIndex;
    DWORD   MaxStreams;
    DWORD   MaxStreamStride;            // max stride for SetStreamSource

    DWORD   VertexShaderVersion;
    DWORD   MaxVertexShaderConst;       // number of vertex shader constant registers

    DWORD   PixelShaderVersion;
    float   MaxPixelShaderValue;        // max value of pixel shader arithmetic component

} D3DCAPS8;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif

extern IDirect3D8 *WINAPI Direct3DCreate8(UINT SDKVersion);

#undef INTERFACE
#define INTERFACE IDirect3D8

DECLARE_INTERFACE_(IDirect3D8, IUnknown) {
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /*** IDirect3D8 methods ***/
    STDMETHOD(RegisterSoftwareDevice)(THIS_ void *pInitializeFunction) PURE;
    STDMETHOD_(UINT, GetAdapterCount)(THIS) PURE;
    STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 * pIdentifier) PURE;
    STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter) PURE;
    STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter, UINT Mode, D3DDISPLAYMODE * pMode) PURE;
    STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter, D3DDISPLAYMODE * pMode) PURE;
    STDMETHOD(CheckDeviceType)(THIS_ UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed) PURE;
    STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) PURE;
    STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType) PURE;
    STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) PURE;
    STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8 * pCaps) PURE;
    STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter) PURE;
    STDMETHOD(CreateDevice)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS * pPresentationParameters, IDirect3DDevice8 * *ppReturnedDeviceInterface) PURE;
};

typedef struct IDirect3D8 *LPDIRECT3D8, *PDIRECT3D8;

#ifdef __cplusplus
};
#endif

#endif /* (DIRECT3D_VERSION >= 0x0800) */
#endif /* _D3D_H_ */
