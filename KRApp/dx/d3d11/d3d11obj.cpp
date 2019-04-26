#ifdef WIN32
#include "d3d11obj.h"
#include "internal.h"

using namespace kr;
using namespace d3d11;

Direct3D11::Init::Init() noexcept
{
	//Com<IDXGIFactory1> dxgiFactory;
	//hrexcept(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory));
	//if (dxgiFactory->EnumAdapters(0, (IDXGIAdapter**)&s_adapter) == DXGI_ERROR_NOT_FOUND)
	//{
	//	FatalAppExitA(0, "You don't have a display adapter");
	//}

	hrmustbe(D3D11CreateDevice(nullptr,    // Adapter
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,    // Module
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		nullptr, 0, // Highest available feature level
		D3D11_SDK_VERSION,
		&s_d3d11,
		nullptr,    // Actual feature level
		nullptr));  // Device context
	s_dxgiDevice = s_d3d11.as<IDXGIDevice>();
	hrmustbe(s_dxgiDevice->GetAdapter(&s_adapter));
	hrmustbe(CreateDXGIFactoryT(&s_dxFactory));
}
Direct3D11::Init::~Init() noexcept
{
}

void Direct3D11::setDevice(ID3D11Device * device) noexcept
{
	s_dxFactory = nullptr;
	s_adapter = nullptr;
	s_dxgiDevice = nullptr;
	s_d3d11 = device;
	if (device != nullptr)
	{
		s_dxgiDevice = s_d3d11.as<IDXGIDevice>();
		hrmustbe(s_dxgiDevice->GetAdapter(&s_adapter));
		hrmustbe(CreateDXGIFactoryT(&s_dxFactory));
	}
}
ID3D11Device * Direct3D11::getDevice() noexcept
{
	return s_d3d11;
}
Direct3D11::Direct3D11() noexcept
{
}
DeviceContext * Direct3D11::operator ->() noexcept
{
	return getContext();
}
void Direct3D11::create(win::Window * wnd, int width, int height, int multiSampling) noexcept
{
	dxgi::SwapChain chain = m_ctx.create(wnd, width, height, multiSampling);
	m_rtv.create(chain.getTexture());
	m_dsv.create(Texture2D(d3d11::BindDepthStencil, Usage::Default, DXGI_FORMAT_D24_UNORM_S8_UINT, width, height, multiSampling));

	m_ctx->OMSetRenderTargets(1, &m_rtv, m_dsv);
	m_ctx.setViewport((float)width, (float)height);
}
void Direct3D11::clear() noexcept
{
	// 색 버퍼 지우기
	float colors[4] = { 0,0,0,1 };
	m_ctx.clear(m_rtv);
	m_ctx.clear(m_dsv);
}

#endif