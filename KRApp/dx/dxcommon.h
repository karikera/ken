#pragma once

#ifdef _M_AMD64 
#define USE_D2D2
#endif

#ifdef USE_D2D2
#define DXGI_HEADER <dxgi1_3.h>
#define D2D_HEADER <d2d1_2.h>
struct ID2D1Factory2;
struct IDXGIFactory2;
struct IDXGISurface2;
struct ID2D1Bitmap1;
struct D2D1_BITMAP_PROPERTIES1;
struct DXGI_SWAP_CHAIN_DESC1;
using ID2D1FactoryT = ID2D1Factory2;
using IDXGIFactoryT = IDXGIFactory2;
using IDXGISurfaceT = IDXGISurface2;
using ID2D1BitmapT = ID2D1Bitmap1;
using D2D1_BITMAP_PROPERTIES_T = D2D1_BITMAP_PROPERTIES1;
using DXGI_SWAP_CHAIN_DESC_T = DXGI_SWAP_CHAIN_DESC1;
#define CreateDXGIFactoryT(ppfactory) CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), ppfactory)
#else
#define DXGI_HEADER <dxgi.h>
#define D2D_HEADER <d2d1.h>
struct ID2D1Factory;
struct IDXGIFactory1;
struct IDXGISurface1;
struct ID2D1Bitmap;
struct D2D1_BITMAP_PROPERTIES;
struct DXGI_SWAP_CHAIN_DESC;
using ID2D1FactoryT = ID2D1Factory;
using IDXGIFactoryT = IDXGIFactory1;
using IDXGISurfaceT = IDXGISurface1;
using ID2D1BitmapT = ID2D1Bitmap;
using D2D1_BITMAP_PROPERTIES_T = D2D1_BITMAP_PROPERTIES;
using DXGI_SWAP_CHAIN_DESC_T = DXGI_SWAP_CHAIN_DESC;
#define CreateDXGIFactoryT(ppfactory) CreateDXGIFactory1(__uuidof(IDXGIFactory1), ppfactory)
#endif

#ifdef USE_D2D2
struct ID2D1DeviceContext;
struct IDCompositionTarget;
struct IDCompositionDevice;
#endif