#pragma once

#include <KRDirect3D/vtypes.h>
#include <KRWin/handle.h>

namespace kr
{
	namespace d3d
	{

#define RGBA(r,g,b,a)					((dword(r)<<16) | (dword(g)<<8) | (dword(b)) | (dword(a)<<24))
#define LOADINGSHADER(name,vb)			[]

		class VertexElement
		{
		public:
			dword uPSize[14];
			int nOffset[14];
			int nType[14];
			dword uSize;

			const D3DVERTEXELEMENT *pDECL;
			mutable IDirect3DVertexDeclaration* pVD;

			__declspec(noalias) __forceinline static dword GetUsageTypeSize(dword type)
			{
				switch(type)
				{
				case D3DDECLTYPE_FLOAT1: return sizeof(float)*1;
				case D3DDECLTYPE_FLOAT2: return sizeof(float)*2;
				case D3DDECLTYPE_FLOAT3: return sizeof(float)*3;
				case D3DDECLTYPE_FLOAT4: return sizeof(float)*4;
				case D3DDECLTYPE_D3DCOLOR: return sizeof(dword);

				case D3DDECLTYPE_UBYTE4: return sizeof(byte)*4;
				case D3DDECLTYPE_SHORT2: return sizeof(short)*2;
				case D3DDECLTYPE_SHORT4: return sizeof(short)*4;

				case D3DDECLTYPE_UBYTE4N: return sizeof(byte)*4;
				case D3DDECLTYPE_SHORT2N: return sizeof(short)*2;
				case D3DDECLTYPE_SHORT4N: return sizeof(short)*4;
				case D3DDECLTYPE_USHORT2N: return sizeof(short)*2;
				case D3DDECLTYPE_USHORT4N: return sizeof(short)*4;
				case D3DDECLTYPE_UDEC3: return 4;
				case D3DDECLTYPE_DEC3N: return 4;
				case D3DDECLTYPE_FLOAT16_2: return sizeof(short)*2;
				case D3DDECLTYPE_FLOAT16_4: return sizeof(short)*4;
				default: return 0;
				}
			}
			template <typename T> __declspec(noalias)VertexElement(T);

			void Initialize() const;
			void Uninitialize() const;
		};
		class Direct3DBase
		{
		protected:
			static bool bDataClear;

			static HRESULT _Begin();
			static HRESULT _DeviceTest();
			static HRESULT _Present();
			static HRESULT _Reset();
			static HRESULT _RenderState(D3DRENDERSTATETYPE rs, dword dwValue);
			static HRESULT _PrimitiveUP(D3DPRIMITIVETYPE pt, cptr list, dword count, dword stride);
			static bool _SizeTest();

		public:
			static void D3DCreate(bool bVSync);
			static void D3DDelete();
			static void D3DClear(vec4 color);
			static void D3DEnd();
			static HRESULT D3DTextureStageState(dword nSampler, D3DTEXTURESTAGESTATETYPE d3dtss, dword dwValue);
			static HRESULT D3DSamplerState(dword nSampler, D3DSAMPLERSTATETYPE sampstate, dword dwValue);
			static void D3DSetViewport(irectwh rcvp);
			static void D3DEmptyTexture();
			static void D3DSetObjects(IDirect3D* pD3D, IDirect3DDevice* pDevice);
			static HRESULT D3DDeviceCaps(D3DCAPS * caps);
			static IDirect3DDevice* D3DGetDevice();
			static IDirect3D* D3DGetDirect3D();
			static HRESULT D3DDraw(D3DPRIMITIVETYPE pt, dword offset, dword count);
			template <typename T> static HRESULT D3DRenderState(D3DRENDERSTATETYPE rs, T tValue)
			{
				return _RenderState(rs, (dword&)tValue);
			}
			template <class TVertex> static HRESULT D3DDraw(D3DPRIMITIVETYPE pt, const TVertex *vb, dword count)
			{
				return _PrimitiveUP(pt, vb, count, sizeof(TVertex));
			}
			static inline void D3DSetFactorColor(dword dwColor)
			{
				D3DRenderState(D3DRS_TEXTUREFACTOR, dwColor);
			}
		};
		template <class T> class Direct3DT:public Direct3DBase
		{
		protected:
			HRESULT _ResetWithData()
			{
				HRESULT hr;
				if(!bDataClear) D3DLostData();

				if(SUCCEEDED(hr=_Reset()))
				{
					D3DResetData();
				}
				return hr;
			}
			bool _Check(HRESULT hr)
			{
#if KR_DXVERSION >= 9

#ifndef _D3D9_H_
#define _FACD3D					0x876
#define MAKE_D3DHRESULT( code ) MAKE_HRESULT( 1, _FACD3D, code )
#define D3DERR_DEVICELOST       MAKE_D3DHRESULT(2152)
#define D3DERR_DEVICENOTRESET   MAKE_D3DHRESULT(2153)
#define D3DERR_INVALIDCALL		MAKE_D3DHRESULT(2156)
#endif

				if(SUCCEEDED(hr)) return true;
				switch(hr)
				{
				case D3DERR_DEVICELOST:
					D3DResetData();
					hr=_DeviceTest();
					if(hr != D3DERR_DEVICENOTRESET) return false;
				case D3DERR_DEVICENOTRESET:
					return SUCCEEDED(_ResetWithData());
				case D3DERR_INVALIDCALL:
					return false;
				default:
					krrcom(("Direct3DT::_Check", hr));
					return true;
				}

#ifndef _D3D9_H_
#undef _FACD3D
#undef MAKE_D3DHRESULT
#undef D3DERR_INVALIDCALL
#undef D3DERR_DEVICELOST
#undef D3DERR_DEVICENOTRESET
#endif

#else
				return true;
#endif
			}

		public:
			void D3DResetData()
			{
				if(!bDataClear) return;
				bDataClear=false;
				scast<T*>(this)->OnReset();
			}
			void D3DLostData()
			{
				if(bDataClear) return;
				bDataClear=true;
				scast<T*>(this)->OnLost();
			}
			bool D3DBegin()
			{
				return _Check(_Begin());
			}
			bool D3DPresent()
			{
				return _Check(_Present());
			}
			bool D3DCheck()
			{
				return _Check(_DeviceTest());
			}
			bool D3DResize()
			{
				if(_SizeTest()) return true;
				D3DLostData();
				return SUCCEEDED(_ResetWithData());
			}
		};
		class Camera
		{
		public:
			void Set2DScreen(float width, float height, float znear, float zfar);
			void SetProjection(float width, float height, float ffovy, float fnear, float ffar);
			void SetProjection(const mat4 &mat);
			void SetView(const vec3 &from, const vec3 &dir, const vec3 &up);
			void SetView(const mat4 &mat);
			const vec3 GetLocaleMove(const vec3 &dir, const vec3 &up) const;
			const vec3 GetDirection() const;
			const mat4 &GetProjection() const;
			const mat4 &GetView() const;
			const mat4 &GetComplex() const;
			void UpdateComplex();

		protected:
			mat4 m_matView, m_matProj, m_matComplex;
		};
		class CSmoothCamera:public Camera
		{
		public:
			CSmoothCamera(const vec3 &up);
			void SetView(const vec3 &from, const vec3 &to, const vec3 &up);
			void SetView(const vec3 &from, const vec3 &to);
			void MoveView(const vec3 &from, const vec3 &to, const vec3 &up);
			void MoveView(const vec3 &from, const vec3 &to);
			void MoveView(const vec3 &move);
			void MoveLook(const vec3 &look);
			void LocaleMoveView(const vec3 &move);
			void EndMove();
			void Step();
			void MouseLine(const ivec2 &mouse, vec3 &from, vec3& to) const;
			const vec3 &GetEye() const;
			const vec3 &GetUp() const;

		protected:
			vec3 m_From, m_To, m_Up;
			vec3 m_FromDest, m_ToDest, m_UpDest;
		};

		template <class T> class CFiller
		{
		public:
			CFiller(T* pHost, dword size)
			{
				m_pHost = pHost;
				m_nUsed = 0;
				m_nSize = size;
			}
			bool Update(cptr pData, dword nSize)
			{
				dword nNext = m_nUsed + nSize;
				if(m_nSize < nNext) return false;
				m_pHost->Update(pData, m_nUsed, nSize);
				m_nUsed = nNext;
				return true;
			}
			template <class T, typename LS> bool Update(T *out, dword uCount, LS ls = nulllambda)
			{
				__if_exists(T::IsUsed)
				{
					if(T::IsUsed(D3DDECLUSAGE_COLOR))
					{
						for(dword i=0; i<uCount; i++) out[i].SetDiffUse(0xffffffff);
					}
				}
				if(ls != nulllambda)
				{
					for(dword i=0; i<uCount; i++) ls(out[i]);
				}
				uCount*=sizeof(T);
				return Update(out, uCount);
			}

		protected:
			T * m_pHost;
			dword m_nUsed;
			dword m_nSize;
		};

		class Texture:public Keep<IDirect3DTexture>
		{
		public:
			Texture();
			Texture(IDirect3DTexture * pTex);

			HRESULT FillFromBitmap(const BMPINFO* pbmi);
			HRESULT CreateFromImage(draw::CImage * pImage);
			HRESULT Create(dword uWidth, dword uHeight, D3DFORMAT format = D3DFMT_A8R8G8B8);
			HRESULT CreateRenderTarget(dword uWidth, dword uHeight, D3DFORMAT format = D3DFMT_A8R8G8B8);
			HRESULT Load(pcstr strName);
			HRESULT Load(pcwstr strName);
			HRESULT Load(HMODULE hModule, dword id, dword type);
			HRESULT Load(cptr p, size_t size);
			HRESULT Load(File *file, size_t len);
			bool Preload(pcstr strName);
			bool Preload(pcwstr strName);
			void Preload(HMODULE hModule, dword id, dword type=(dword)RT_BITMAP);
			void Preload(File *file, size_t len);
			HRESULT Save(pcstr strName);
			HRESULT Save(pcwstr strName);
			HRESULT Save(ID3DXBuffer ** buf);
			HRESULT Use();
			HRESULT LockRect(D3DLOCKED_RECT *pd3dlr);
			HRESULT UnlockRect();

			static HRESULT Unuse();

		private:
			IDirect3DTexture * m_pTexture;
		};
		class CDynamicTexture final: public Texture
		{
		public:
			CDynamicTexture();
			CDynamicTexture(IDirect3DTexture * pTex);
			HRESULT Create(dword uWidth, dword uHeight, D3DFORMAT format = D3DFMT_A8R8G8B8);
			HRESULT LockRect(D3DLOCKED_RECT *pd3dlr);
		};

		class VertexBuffer:public Pointer<IDirect3DVertexBuffer>
		{
		public:
			VertexBuffer();
			VertexBuffer(IDirect3DVertexBuffer * p);
			void Create(dword size, dword fvf);
			void Use(dword uOffset, dword uVSize);

			CFiller<VertexBuffer> * GetFiller();

		};

		class CIndexBuffer:public Pointer<IDirect3DIndexBuffer>
		{
		public:
			CIndexBuffer();
			CIndexBuffer(IDirect3DIndexBuffer * p);
			void Create(dword uSize);
			void Use();

			CFiller<CIndexBuffer> * GetFiller();
		protected:
			IDirect3DIndexBuffer* m_pIB;
		};

		class CVIBuffer
		{
		public:
			CVIBuffer();
			void Create(int vbsize, int ibsize, dword fvf);
			void Delete();
			void Use(dword uVSize);

		protected:
			CIndexBuffer m_IB;
			VertexBuffer m_VB;
		};

		class DXFont
		{
		protected:
			LPD3DXFONT pFont;
		public:

			void Create(const XFONTDESC *fd) noexcept;
			void Delete() noexcept;
			void Draw(RString str, RC &rc, dword dwFlags, color3 color) noexcept;
			void Lost() noexcept;
			void Reset() noexcept;

			operator bool() noexcept;
			bool operator !() noexcept;
			void Draw(RString str, const RC &rc, color3 color) noexcept;
			void Draw(RString str, ivec2 pt, color3 color) noexcept;
			void CalcRect(RString str, RC &rc) noexcept;
		};
	}
}