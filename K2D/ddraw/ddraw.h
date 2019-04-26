#pragma once

#ifndef __DDRAW_INCLUDED__

struct IDirectDraw7 * LPDIRECTDRAW;
struct IDirectDrawSurface7 * LPDIRECTDRAWSURFACE;
struct IDirectDrawClipper7 * LPDIRECTDRAWCLIPPER;

#else

typedef LPDIRECTDRAW7 LPDIRECTDRAW;
typedef LPDIRECTDRAWSURFACE7 LPDIRECTDRAWSURFACE;
typedef DDSURFACEDESC2 DDSURFACEDESC;
typedef DDSCAPS2 DDSCAPS;

#endif

struct FFastCopy;
struct FSurfCopy;
struct FColorKey;
template <class T> class TSurface;
class CSurface;
class CWindowScreenBase;
class CFullScreenBase;
class CBackSurface;
class COffscreenSurface;

namespace kr
{

	struct FFastCopy
	{
		int SrcColorKey : 1;
		int DestColorKey : 1;

		int junk : 2;

		int Wait : 1;
		int DoNotWait : 1;

		int junk2 : 26;

		inline FFastCopy(bool srckey = false, bool destkey = false)
		{
			mema::zero(*this);
			SrcColorKey = srckey;
			DestColorKey = destkey;
			Wait = true;
		}
	};
	struct FSurfCopy
	{
		bool AlphaDest : 1;
		bool AlphaDestConstOverride : 1;
		bool AlphaDestNeg : 1;
		bool AlphaDestSurfaceOverride : 1;
		bool AlphaEdgeBlend : 1;
		bool AlphaSrc : 1;
		bool AlphaSrcConstOverride : 1;
		bool AlphaSrcNeg : 1;
		bool AlphaSrcSurfaceOverride : 1;
		bool Async : 1;
		bool ColorFill : 1;
		bool FX : 1;
		bool ROPS : 1;
		bool KeyDest : 1;
		bool KeyDestOverride : 1;
		bool KeySrc : 1;
		bool KeySrcOverride : 1;
		bool ROP : 1;
		bool RotationAngle : 1;
		bool ZBuffer : 1;
		bool ZBufferDestConstOverride : 1;
		bool ZBufferDestOverride : 1;
		bool ZBufferSrcConstOverride : 1;
		bool ZBufferSrcOverride : 1;
		bool Wait : 1;
		bool DepthFill : 1;
		bool DoNotWait : 1;
		bool Presentation : 1;
		bool LastPresentation : 1;
		bool ExtendedFlags : 1;

		bool junk : 1;

		inline FSurfCopy(bool keysrc = false, bool keydest = false, bool srcover = false, bool destover = false)
		{
			mema::zero(*this);
			DoNotWait = true;
			KeyDest = keydest;
			KeySrc = keysrc;
			KeyDestOverride = destover;
			KeySrcOverride = srcover;

		}

	};
	struct FColorKey
	{
		bool ColorSpace : 1;
		bool DestBlt : 1;
		bool DestOverlay : 1;
		bool SrcBlt : 1;
		bool SrcOverlay : 1;

		bool _unuse1 : 3;
		bool _unuse2 : 8;
		bool _unuse3 : 8;
		bool _unuse4 : 8;

		inline FColorKey(bool srcblt = true, bool destblt = false, bool srcover = false, bool destover = false)
		{
			mema::zero(*this);
			SrcBlt = srcblt;
			DestBlt = destblt;
			DestOverlay = destover;
			SrcOverlay = srcover;
		}

	};

	class CSurface
	{
	protected:
		LPDIRECTDRAWSURFACE pSurface;
	public:
		static LPDIRECTDRAW pDD;
		static dword dwWidth, dwHeight;
		inline bool CheckError(HRESULT hr, int line, RStringA func);

		inline CSurface()
		{
			pSurface = nullptr;
		}

		void Delete();

		bool Lock(CDraw &draw);
		void Unlock();

		bool Clear(dword dwColor);
		bool Restore();
		void ColorKey(dword color, FColorKey flags = FColorKey());
		void GetSize(dword *pWidth, dword *pHeight);
		void ReadyCopy();

		void Copy(RCWH rect, PT pt, CSurface img, FFastCopy flags = FFastCopy());
		void Copy(const RC &rect, const RC &rect2, CSurface img, FSurfCopy flags = FSurfCopy());

		HDC GetDC();
		void ReleaseDC(HDC hDC);


		static void Initialize();
		static void Uninitialize();

		inline operator bool()
		{
			return pSurface != nullptr;
		}
		inline bool operator !()
		{
			return pSurface == nullptr;
		}
		inline operator ptr()
		{
			return (ptr)pSurface;
		}
	};
	class CSurfaceBase :public CSurface
	{
#define LINKDD(ret,func) inline ret DD##func(){ return func(); } 
	public:
		LINKDD(void, Delete);
		LINKDD(void, Unlock);
		LINKDD(bool, Restore);
		LINKDD(void, ReadyCopy);

		inline bool DDLock(CDraw &draw)
		{
			return Lock(draw);
		}
		inline bool DDClear(dword dwColor)
		{
			return Clear(dwColor);
		}
		inline void DDColorKey(dword color, FColorKey flags = FColorKey())
		{
			return ColorKey(color, flags);
		}
		inline void DDGetSize(dword *pWidth, dword *pHeight)
		{
			return GetSize(pWidth, pHeight);
		}
		inline void DDCopy(const RCWH &rect, PT pt, CSurface img, FFastCopy flags = FFastCopy())
		{
			return Copy(rect, pt, img, flags);
		}
		inline void DDCopy(const RC &rect, const RC &rect2, CSurface img, FSurfCopy flags = FSurfCopy())
		{
			return Copy(rect, rect2, img, flags);
		}
		inline HDC DDGetDC()
		{
			return GetDC();
		}
		inline void DDReleaseDC(HDC hDC)
		{
			return ReleaseDC(hDC);
		}

#undef DDLINK

	};
	class COffscreenSurface :public CSurface
	{
	public:
		bool Resize(int w, int h);
		void LoadFromBMPData(const BMPINFO* pbi, ptr data);
		bool LoadFromBMPFile(pcstr16 strFile);
		bool LoadFromPNGFile(pcstr16 Filename);
		bool SaveAsPNGFile(pcstr16 Filename, BOOL InterlaceAdam7);

		inline void Create(int w, int h)
		{
			Resize(w, h);
		}
	};
	class CWindowScreenBase :public CSurfaceBase
	{
	protected:
		LPDIRECTDRAWCLIPPER clip;
	public:
		int W, H;
		void DDCreate();
		bool DDResize(int w, int h);
		void DDCopy(CSurface surf);
		void DDUpdateSize();
		bool DDEqualize(COffscreenSurface& surf);

	};
	class CFullScreenBase :public CSurfaceBase
	{
		friend class CBackSurface;
	public:
		void DDGetSize(dword *pWidth, dword *pHeight);
		void DDCreate(int w, int h);
		bool DDResize(int w, int h);
		bool DDRestore();
	};
	class CBackSurface :public CSurface
	{
	protected:
		CFullScreenBase *pPrimary;
	public:
		void Create(CFullScreenBase surf);
		bool Flip();
		bool Restore();
	};
}