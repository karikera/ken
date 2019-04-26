#include "stdafx.h"

#include <KRDirect3D/rtypes.h>
#include <KRDirect3D/d3d.h>
#include <KR/win/resource.h>

namespace kr
{
	namespace d3d
	{
		IDirect3D* g_pD3D;
		IDirect3DDevice* g_pDevice;
		static D3DPRESENT_PARAMETERS d3dpp;
#ifdef KR_DX11
		D3DDeviceContext g_pContext;
#endif
	}
}

using namespace kr;
using namespace d3d;

bool Direct3DBase::bDataClear=false;
	
static dword dwMeshFlags = 0;
static dword dwDTLockFlags = 0;
static dword dwDTUsage = 0;

	
HRESULT Direct3DBase::_Begin()
{
	return g_pDevice->BeginScene();
}
HRESULT Direct3DBase::_DeviceTest()
{
	return g_pDevice->TestCooperativeLevel();
}
HRESULT Direct3DBase::_Present()
{
	return g_pDevice->Present(nullptr,nullptr,nullptr,nullptr);
}
HRESULT Direct3DBase::_Reset()
{
	d3dpp.BackBufferHeight=0;
	d3dpp.BackBufferWidth=0;

	return g_pDevice->Reset(&d3dpp);
}
HRESULT Direct3DBase::_RenderState(D3DRENDERSTATETYPE rs,dword dwValue)
{
	return g_pDevice->SetRenderState(rs,dwValue);
}
HRESULT Direct3DBase::_PrimitiveUP(D3DPRIMITIVETYPE pt,cptr list,dword count,dword stride)
{
	return g_pDevice->DrawPrimitiveUP(pt,count,list,stride);
}
bool Direct3DBase::_SizeTest()
{
	if(g_pDevice == nullptr) return true;
	RC rc = g_pMainWindow->GetClientRect();
	return (d3dpp.BackBufferWidth == rc.r && d3dpp.BackBufferHeight == rc.b);
}
void Direct3DBase::D3DCreate(bool bVSync)
{
#if KR_DXVERSION == 9
	g_pD3D=Direct3DCreate9(D3D_SDK_VERSION);
#endif
#if KR_DXVERSION == 10
	g_pD3D=Direct3DCreate10(D3D_SDK_VERSION);
#endif
#if KR_DXVERSION == 11
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	dword uFlags=0;
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

#ifdef _DEBUG
	uFlags=D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr;
	D3D_DRIVER_TYPE driverType;
	D3D_FEATURE_LEVEL featureLevel;
	for(int i=0;i<3;i++)
	{
		driverType=driverTypes[i];
		hr = D3D11CreateDevice( nullptr, driverType, nullptr, uFlags, featureLevels, sizeof(featureLevels)/sizeof(D3D_FEATURE_LEVEL),
                                        D3D11_SDK_VERSION, &g_pDevice, &featureLevel, &g_pContext );
		if(SUCCEEDED(hr)) goto __DeviceCreate;
	}
	throw KRRDX("Direct3DCreate",hr);
__DeviceCreate:



ID3D11Texture2D* pBackBuffer = nullptr;
hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( ptr* )&pBackBuffer );
if( FAILED( hr ) )
    return hr;

hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, nullptr, &g_pRenderTargetView );
pBackBuffer->Release();
if( FAILED( hr ) )
    return hr;

g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, nullptr );

// Setup the viewport
D3D11_VIEWPORT vp;
vp.Width = (FLOAT)width;
vp.Height = (FLOAT)height;
vp.MinDepth = 0.0f;
vp.MaxDepth = 1.0f;
vp.TopLeftX = 0;
vp.TopLeftY = 0;
g_pImmediateContext->RSSetViewports( 1, &vp );
#else
	if(g_pD3D==nullptr) krr("Direct3DCreate");
	
	mema::zero(d3dpp);
	d3dpp.Windowed=TRUE;
	d3dpp.hDeviceWindow=g_pMainWindow;
	d3dpp.EnableAutoDepthStencil=TRUE;
	d3dpp.AutoDepthStencilFormat=D3DFMT_D16;
	d3dpp.SwapEffect=D3DSWAPEFFECT_DISCARD;
	d3dpp.PresentationInterval=bVSync ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;
	krrcom(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,g_pMainWindow,
		D3DCREATE_MIXED_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,&d3dpp,&g_pDevice));
		
	D3DCAPS d3dCaps;
	krrcom(g_pDevice->GetDeviceCaps(&d3dCaps));
		
	if( d3dCaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES )
	{
		dwDTLockFlags =  D3DLOCK_DISCARD;
		dwDTUsage = D3DUSAGE_DYNAMIC;
	}
	//dwMeshFlags= D3DXMESHOPT_VERTEXCACHE;
	if( d3dCaps.VertexShaderVersion >= D3DVS_VERSION( 1, 1 ) )
	{
		g_pDevice->SetSoftwareVertexProcessing( false );
		dwMeshFlags |= D3DXMESH_MANAGED;
	}
	else
	{
		g_pDevice->SetSoftwareVertexProcessing( true );
		dwMeshFlags |= D3DXMESH_SYSTEMMEM;
	}
#endif
}
void Direct3DBase::D3DDelete()
{
	SafeRelease(g_pDevice);
	SafeRelease(g_pD3D);
}
void Direct3DBase::D3DClear(color3 color)
{
	g_pDevice->Clear(0,nullptr,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,color,1.f,0);
}
void Direct3DBase::D3DEnd()
{
	g_pDevice->EndScene();
}
HRESULT Direct3DBase::D3DTextureStageState(dword nSampler,D3DTEXTURESTAGESTATETYPE d3dtss,dword dwValue)
{
	return g_pDevice->SetTextureStageState(nSampler,d3dtss,dwValue);
}
HRESULT Direct3DBase::D3DSamplerState(dword nSampler,D3DSAMPLERSTATETYPE sampstate,dword dwValue)
{
	return g_pDevice->SetSamplerState(nSampler,sampstate,dwValue);
}
void Direct3DBase::D3DSetViewport(RCWH rcvp)
{
	D3DVIEWPORT9 vp;
	vp.X=rcvp.x;
	vp.Y=rcvp.y;
	vp.Width=rcvp.w;
	vp.Height=rcvp.h;
	vp.MinZ=0;
	vp.MaxZ=1;
	g_pDevice->SetViewport(&vp);
}
void Direct3DBase::D3DEmptyTexture()
{
	g_pDevice->SetTexture(0,nullptr);
}
void Direct3DBase::D3DSetObjects(IDirect3D* pD3D,IDirect3DDevice* pDevice)
{
	g_pD3D=pD3D;
	g_pDevice=pDevice;
}
HRESULT Direct3DBase::D3DDeviceCaps(D3DCAPS * caps)
{
	return g_pDevice->GetDeviceCaps(caps);
}
IDirect3DDevice* Direct3DBase::D3DGetDevice()
{
	return g_pDevice;
}
IDirect3D* Direct3DBase::D3DGetDirect3D()
{
	return g_pD3D;
}
HRESULT Direct3DBase::D3DDraw(D3DPRIMITIVETYPE pt,dword offset,dword count)
{
	return g_pDevice->DrawPrimitive(pt,offset,count);
}
	
Texture::Texture()
{
}
Texture::Texture(IDirect3DTexture * pTex)
{
	m_this = pTex;
}

HRESULT Texture::FillFromBitmap(const BMPINFO * pbmi)
{
	HRESULT hr;
	ptr pData = (pbyte)pbmi + pbmi->biSize;

	D3DLOCKED_RECT d3dlr;
	if(FAILED(hr = m_this->LockRect(0,&d3dlr,nullptr,0))) return hr;

	//d3dlr.pBits
		
	m_this->UnlockRect(0);

	return true;
}
	
HRESULT Texture::CreateFromImage(draw::CImage * pImage)
{
	HRESULT hr;

	int w = pImage->Width();
	int h = pImage->Height();

	int w2 = math::pow2upper(w);
	int h2 = math::pow2upper(h);

	if(FAILED(hr = Create(w2,h2))) return hr;
				
	d3d::D3DLOCKED_RECT lock;
	if(FAILED(hr = m_pTexture->LockRect(0,&lock,nullptr,D3DLOCK_NOOVERWRITE))) return hr;
	lock.pBits;
	m_pTexture->UnlockRect(0);

	return S_OK;
}
HRESULT Texture::Create(dword uWidth,dword uHeight,D3DFORMAT format)
{
	return g_pDevice->CreateTexture(uWidth,uHeight,1,0,format,D3DPOOL_MANAGED,&m_this,nullptr);
}
HRESULT Texture::CreateRenderTarget(dword uWidth,dword uHeight,D3DFORMAT format)
{
	return g_pDevice->CreateTexture(uWidth,uHeight,1,D3DUSAGE_RENDERTARGET,format,D3DPOOL_DEFAULT,&m_this,nullptr);
}
HRESULT Texture::Load(pcstr strName)
{
	return D3DXCreateTextureFromFileA(g_pDevice,strName,&m_this);
}
HRESULT Texture::Load(pcwstr strName)
{
	return D3DXCreateTextureFromFileW(g_pDevice,strName,&m_this);
}
HRESULT Texture::Load(HMODULE hModule,dword id,dword type)
{
	Resource<ptr> resource(hModule,id,type);
	return D3DXCreateTextureFromFileInMemory(g_pDevice,resource,resource.GetSize(),&m_this);
}
HRESULT Texture::Load(cptr p,size_t size)
{
	return D3DXCreateTextureFromFileInMemory(g_pDevice,p,size,&m_this);
}
HRESULT Texture::Load(File *file,size_t len)
{
	MAPPING m= file->BeginMapping(file->GetPointer(),len);
	HRESULT hr = Load(m.point,len);
	file->EndMapping(m);
	file->Skip(len);
	return hr;
}
HRESULT Texture::Save(pcstr strName)
{
	return D3DXSaveTextureToFileA(strName,D3DXIFF_PNG,m_this,nullptr);
}
HRESULT Texture::Save(pcwstr strName)
{
	return D3DXSaveTextureToFileW(strName,D3DXIFF_PNG,m_this,nullptr);
}
HRESULT Texture::Save(ID3DXBuffer ** buf)
{
	return D3DXSaveTextureToFileInMemory((ID3DXBuffer**)buf,D3DXIFF_PNG,m_this,nullptr);
}
HRESULT Texture::Use()
{
	return g_pDevice->SetTexture(0,m_this);
}
HRESULT Texture::Unuse()
{
	return g_pDevice->SetTexture(0,nullptr);
}
HRESULT Texture::LockRect(D3DLOCKED_RECT *pd3dlr)
{
	return m_this->LockRect(0, pd3dlr, nullptr,0);
}
HRESULT Texture::UnlockRect()
{
	return m_this->UnlockRect(0);
}
	
CDynamicTexture::CDynamicTexture()
{
}
CDynamicTexture::CDynamicTexture(IDirect3DTexture * pTex)
{
	m_this = pTex;
}
HRESULT CDynamicTexture::Create(dword uWidth,dword uHeight,D3DFORMAT format)
{
	return g_pDevice->CreateTexture(uWidth,uHeight,1,dwDTUsage,format,D3DPOOL_DEFAULT,&m_this,nullptr);
}
HRESULT CDynamicTexture::LockRect(D3DLOCKED_RECT *pd3dlr)
{
	return m_this->LockRect(0, pd3dlr, nullptr,dwDTLockFlags);
}

VertexBuffer::VertexBuffer()
{
}
VertexBuffer::VertexBuffer(IDirect3DVertexBuffer * p)
{
	m_this = p;
}
void VertexBuffer::Create(dword size,dword fvf)
{
	krrcom(g_pDevice->CreateVertexBuffer(size,0,fvf,D3DPOOL_MANAGED,&m_this,nullptr));
}
void VertexBuffer::Use(dword uOffset,dword uVSize)
{
	g_pDevice->SetStreamSource(0,m_this,uOffset,uVSize);
}

CIndexBuffer::CIndexBuffer()
{
	m_pIB=nullptr;
}
CIndexBuffer::CIndexBuffer(IDirect3DIndexBuffer * p)
{
	m_this = p;
}
void CIndexBuffer::Create(dword uSize)
{
	krrcom(g_pDevice->CreateIndexBuffer(uSize,D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&m_pIB,nullptr));
}
void CIndexBuffer::Use()
{
	g_pDevice->SetIndices(m_pIB);
}

CVIBuffer::CVIBuffer()
{
}
void CVIBuffer::Create(int vbsize,int ibsize,dword fvf)
{
	m_VB.Create(vbsize,fvf);
	m_IB.Create(ibsize);
}
void CVIBuffer::Delete()
{
	SafeRelease(m_IB);
	SafeRelease(m_VB);
}
void CVIBuffer::Use(dword uVSize)
{
	m_IB.Use();
	m_VB.Use(0,uVSize);
}

void Camera::Set2DScreen(float width,float height,float znear,float zfar)
{
	m_matComplex.SetProjection2D(width,height,znear,zfar);
}
void Camera::SetProjection(float width,float height,float ffovy,float fnear,float ffar)
{
	m_matProj.SetProjection3D(ffovy,width/height,fnear,ffar);
}
void Camera::SetProjection(const matrix4 &mat)
{
	m_matProj=mat;
}
void Camera::SetView(const float3 &from,const float3 &dir,const float3 &up)
{
	m_matView.SetView(from,dir,up);
}
void Camera::SetView(const matrix4 &mat)
{
	m_matView=mat;
}
const float3 Camera::GetLocaleMove(const float3 &dir,const float3 &up) const
{
	matrix4 tmat=m_matView;
	tmat.vec4.set(0,0,0,1);
	float3 nvec=tmat*dir;
	float3 norm=cross(nvec,up);
	nvec=cross(up,norm);

	float fLen=nvec.length();
	if(fLen == 0) return float3(0,0,0);

	return nvec / fLen;
}
const float3 Camera::GetDirection() const
{
	return float3(m_matView._13,m_matView._23,m_matView._33);
}
const matrix4 &Camera::GetProjection() const
{
	return m_matProj;
}
const matrix4 &Camera::GetView() const
{
	return m_matView;
}
const matrix4 &Camera::GetComplex() const
{
	return m_matComplex;
}
void Camera::UpdateComplex()
{
	m_matComplex=m_matView * m_matProj;
}
	
CSmoothCamera::CSmoothCamera(const float3 &up)
{
	m_Up=m_UpDest=up;
}
void CSmoothCamera::SetView(const float3 &from,const float3 &to,const float3 &up)
{
	m_FromDest=m_From=from;
	m_ToDest=m_To=to;
	m_UpDest=m_Up=up;
	m_matView.SetView(m_From,m_To-m_From,m_Up);
}
void CSmoothCamera::SetView(const float3 &from,const float3 &to)
{
	m_FromDest=m_From=from;
	m_ToDest=m_To=to;
	m_matView.SetView(m_From,m_To-m_From,m_Up);
}
void CSmoothCamera::MoveView(const float3 &from,const float3 &to,const float3 &up)
{
	m_FromDest = from;
	m_ToDest = to;
	m_UpDest = up;
}
void CSmoothCamera::MoveView(const float3 &from,const float3 &to)
{
	m_FromDest=from;
	m_ToDest=to;
}
void CSmoothCamera::MoveView(const float3 &move)
{
	m_FromDest+=move;
	m_ToDest+=move;
}
void CSmoothCamera::MoveLook(const float3 &look)
{
	m_ToDest=m_FromDest + look;
}
void CSmoothCamera::LocaleMoveView(const float3 &move)
{
	matrix4 tmat=m_matView;
	tmat.vec4.set(0,0,0,1);
	float3 nmove= tmat * move;
		
	m_FromDest+=nmove;
	m_ToDest+=nmove;
}
void CSmoothCamera::EndMove()
{
	m_From=m_FromDest;
	m_To=m_ToDest;
}
void CSmoothCamera::Step()
{
	float3 fmove=m_FromDest-m_From;

	m_From=(m_From*15.f+m_FromDest)/16.f;
	m_To=(m_To*15.f+m_ToDest)/16.f;
	m_Up=(m_Up*15.f+m_UpDest)/16.f;

	m_matView.SetView(m_From,m_To-m_From,m_Up);
}
const float3 &CSmoothCamera::GetEye() const
{
	return m_From;
}
const float3 &CSmoothCamera::GetUp() const
{
	return m_Up;
}

void DXFont::Create(const D3DXFONT_DESC *fd) noexcept
{
	krrcom(D3DXCreateFontIndirect(g_pDevice,(LPD3DXFONT_DESC)fd,&pFont));
}
void DXFont::Delete() noexcept
{
	assert(pFont != nullptr);
	pFont->Release();
}
void DXFont::Draw(RString str,RC &rc,dword dwFlags,color3 color) noexcept
{
	assert(pFont != nullptr);
	pFont->DrawTextW(nullptr,str.string,str.length,(LPRECT)&rc,dwFlags,color);
}
void DXFont::Lost() noexcept
{
	assert(pFont != nullptr);
	pFont->OnLostDevice();
}
void DXFont::Reset() noexcept
{
	assert(pFont != nullptr);
	pFont->OnResetDevice();
}
DXFont::operator bool() noexcept
{
	return pFont != nullptr;
}
bool DXFont::operator !() noexcept
{
	return pFont == nullptr;
}
void DXFont::Draw(RString str, const RC &rc, color3 color) noexcept
{
	Draw(str, (RC&)rc, 0, color);
}
void DXFont::Draw(RString str, PT pt, color3 color) noexcept
{
	Draw(str, RC(pt.x, pt.y, 0, 0), DT_NOCLIP, color);
}
void DXFont::CalcRect(RString str, RC &rc) noexcept
{
	Draw(str, rc, DT_CALCRECT | DT_NOCLIP, 0);
}