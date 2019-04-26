#define KRGL_PRIVATE
#include "3d.h"
#include "include_d3d11.h"

#include <tchar.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

namespace
{
	const std::uint32_t WHITE = 0xffffffff;
	kr::gl::Window3D * s_main;
}

//inline void glTestError()
//{
//	int err;
//	for (;;)
//	{
//		err = glGetError();
//		if (err == GL_NO_ERROR) break;
//		cerr << err << endl;
//	}
//}

bool kr::gl::Resolution::operator==(const Resolution & res) const noexcept
{
	return w == res.w && h == res.h && ms == res.ms;
}
bool kr::gl::Resolution::operator!=(const Resolution & res) const noexcept
{
	return w != res.w || h != res.h || ms != res.ms;
}
std::size_t std::hash<kr::gl::Resolution>::operator()(const kr::gl::Resolution& _Keyval) const
{
	return kr::mem::hash(&_Keyval, sizeof (kr::gl::Resolution));
}

kr::gl::Window3D::Window3D(const Config * config) noexcept
{
	s_main = this;
	createPrimary(config->wndName, WS_VISIBLE | WS_POPUP | WS_SYSMENU, config->rect);
	m_FSEnable = config->fullScreen;
	m_isProgramG = false;

#ifdef _DEBUG
	constexpr int flags =  D3D11_CREATE_DEVICE_DEBUG;
#else
	constexpr int flags = 0;
#endif
		
	m_wndrect = config->rect;
	m_samples = config->backBufferMultisample;

	{
		Keep<IDXGIFactory> factory;
		Keep<IDXGIAdapter> adapter;
		hrmustbe(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&factory)));
		factory->EnumAdapters(0, &adapter);
		
		hrmustbe(D3D11CreateDevice(adapter, adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
			nullptr, flags, nullptr, 0, D3D11_SDK_VERSION,
			&m_device, nullptr, &m_ctx));
			
		/////////////////////////////////////////////////////////
		// Create swap chain
		DXGI_SWAP_CHAIN_DESC scDesc = { 0 };
		scDesc.BufferCount = 2;
		scDesc.BufferDesc.Width  = config->rect.width;
		scDesc.BufferDesc.Height = config->rect.height;
		scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.BufferDesc.RefreshRate.Numerator = 0;
		scDesc.BufferDesc.RefreshRate.Denominator = 1;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.OutputWindow = getWindow();
		scDesc.SampleDesc.Count = m_samples;
		scDesc.SampleDesc.Quality = 0;
		scDesc.Windowed = !m_FSEnable;
		scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

		_closeSwapChain();
		hrmustbe(factory->CreateSwapChain(m_device, &scDesc, &m_swapchain));
	
		if(m_FSEnable) _setMonitor(adapter);
	}

	m_wndRenderTarget.createFromSwapChain();
	m_wndRenderTarget.use();

	initStates();
	
	Texture::empty = Texture(kr::gl::Texture::RGBA, 1, 1, &WHITE);
}
kr::gl::Window3D::~Window3D() noexcept
{
	Texture::empty = Texture();
	m_wndRenderTarget = RenderTarget();
	m_ctx->ClearState();
	_closeSwapChain();
}

void kr::gl::Window3D::vdraw(Draw draw, std::size_t vcount, std::size_t voffset) noexcept
{
	_setTopology(draw);
	_updateUniform();
	assert(vcount <= 0xffffffff);
	assert(voffset <= 0xffffffff);
	m_ctx->Draw((UINT)vcount, (UINT)voffset);
}
void kr::gl::Window3D::idraw(Draw draw, std::size_t icount, std::size_t ioffset, std::intptr_t voffset) noexcept
{
	_setTopology(draw);
	_updateUniform();
	assert(icount <= 0xffffffff);
	assert(ioffset <= 0xffffffff);
	assert(voffset <= 0x7fffffff);
	m_ctx->DrawIndexed((UINT)icount, (UINT)ioffset, (INT)voffset); 
}
void kr::gl::Window3D::setViewport(const irectwh &rc) noexcept
{
	// Set viewport
    D3D11_VIEWPORT  d3dvp;
    d3dvp.Width    = (float)rc.width;
    d3dvp.Height   = (float)rc.height;
    d3dvp.MinDepth = 0;
    d3dvp.MaxDepth = 1;
    d3dvp.TopLeftX = (float)rc.x;
    d3dvp.TopLeftY = (float)rc.y;    
    m_ctx->RSSetViewports(1, &d3dvp);
}

const kr::irectwh & kr::gl::Window3D::getWindowRect() noexcept
{
	return m_wndrect;
}
int kr::gl::Window3D::getMultiSampleCount() noexcept
{
	return m_samples;
}
void kr::gl::Window3D::initStates() noexcept
{
	RenderMode(RenderMode::CullBack).use();
	setViewport({ {0,0}, {m_wndrect.width, m_wndrect.height} });
}
void kr::gl::Window3D::clear(const vec4 &color) noexcept
{
	Keep<ID3D11RenderTargetView> rv;
	Keep<ID3D11DepthStencilView> dv;
	m_ctx->OMGetRenderTargets(1, &rv, &dv);
	m_ctx->ClearRenderTargetView( rv, color.m_data );
	m_ctx->ClearDepthStencilView( dv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}

int kr::gl::Window3D::processing() noexcept
{
	MSG msg;
	do
	{
		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else
		{
			onRender();
			m_swapchain->Present( 0, 0 );
			flushKey();
		}
	}
	while (msg.message != WM_QUIT);
	return (int)msg.wParam;
}

void kr::gl::Window3D::_closeSwapChain() noexcept
{
	if (m_swapchain)
	{
		if (m_FSEnable) m_swapchain->SetFullscreenState(false, nullptr);
		m_swapchain = nullptr;
	}
}
void kr::gl::Window3D::_setTopology(Draw draw) noexcept
{
	static const D3D_PRIMITIVE_TOPOLOGY topologies[]=
	{
		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
	};
	if (m_prevdraw != draw)
	{
		m_prevdraw = draw;
		m_ctx->IASetPrimitiveTopology(topologies[(int)draw]);
	}
}
void kr::gl::Window3D::_setMonitor(IDXGIAdapter * adapter) noexcept
{
	for(UINT OutputIndex = 0; ; OutputIndex++)
	{
		Keep<IDXGIOutput> output;
		{
			HRESULT hr = adapter->EnumOutputs(OutputIndex, &output);
			if (hr == DXGI_ERROR_NOT_FOUND) break;
		}

		DXGI_OUTPUT_DESC OutDesc;
		output->GetDesc(&OutDesc);

		MONITORINFOEX monitor;
		monitor.cbSize = sizeof(monitor);
		if (!::GetMonitorInfo(OutDesc.Monitor, &monitor)) continue;
		if (monitor.szDevice[0] == _T('\0')) continue;
		
		DISPLAY_DEVICE dispDev;
		memset(&dispDev, 0, sizeof(dispDev));
		dispDev.cb = sizeof(dispDev);

		if (!::EnumDisplayDevices(monitor.szDevice, 0, &dispDev, 0)) continue;
		
		m_wndrect.x = monitor.rcMonitor.left;
		m_wndrect.y = monitor.rcMonitor.top;
		m_wndrect.width = monitor.rcMonitor.right - monitor.rcMonitor.left;
		m_wndrect.height = monitor.rcMonitor.bottom - monitor.rcMonitor.top;
		
		if (!hrshouldbe(m_swapchain->SetFullscreenState(true, output)))
		{
			goto __failed;
		}
		return;
	}
	
	EnumDisplayMonitors(0, 0, [](HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData)->BOOL
	{
		Window3D* w = (Window3D*)dwData;

		MONITORINFOEX monitor;
		monitor.cbSize = sizeof(monitor);
		if (!::GetMonitorInfo(hMonitor, &monitor)) return TRUE;
		if (monitor.szDevice[0] == _T('\0')) return TRUE;

		DISPLAY_DEVICE dispDev = {sizeof(DISPLAY_DEVICE) ,0};
		if (!::EnumDisplayDevices(monitor.szDevice, 0, &dispDev, 0)) return TRUE;

		w->m_wndrect.x = monitor.rcMonitor.left;
		w->m_wndrect.y = monitor.rcMonitor.top;
		w->m_wndrect.width = monitor.rcMonitor.right - monitor.rcMonitor.left;
		w->m_wndrect.height = monitor.rcMonitor.bottom - monitor.rcMonitor.top;
		return FALSE;
	}, (LPARAM)this);

__failed:
	m_FSEnable = false;
	
	MoveWindow(getWindow(), m_wndrect.x, m_wndrect.y, m_wndrect.width, m_wndrect.height, false);
	m_swapchain->SetFullscreenState(false, nullptr);
}
void kr::gl::Window3D::_updateUniform() noexcept
{
	assert(m_curProgram != nullptr);

	if (m_fsBuffer.update(m_curProgram->m_fsUniform))
	{
		m_ctx->PSSetConstantBuffers(0, 1, &m_fsBuffer.buffer);
	}

	if (m_vsBuffer.update(m_curProgram->m_vsUniform))
	{
		m_ctx->VSSetConstantBuffers(0, 1, &m_vsBuffer.buffer);
	}

	if (m_isProgramG)
	{
		if (m_gsBuffer.update((static_cast<ProgramG*>(m_curProgram))->m_gsUniform))
		{
			s_main->m_ctx->GSSetConstantBuffers(0, 1, &m_gsBuffer.buffer);
		}
	}
}

kr::gl::Window3D::UniformBuffer::UniformBuffer() noexcept
{
	size = 0;
}
bool kr::gl::Window3D::UniformBuffer::update(const Array<byte> &data) noexcept
{
	if (data.size() > size)
	{
		size = data.size();
		assert(size <= 0xffffffff);

		D3D11_BUFFER_DESC desc;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; 
		desc.ByteWidth = (UINT)size;

		D3D11_SUBRESOURCE_DATA initdata;
		initdata.pSysMem = data.begin();
		initdata.SysMemPitch = 0;
		initdata.SysMemSlicePitch = 0;
		
		buffer = nullptr;
		return hrshouldbe(s_main->m_device->CreateBuffer(&desc, &initdata, &buffer));
	}
	else
	{
		if (buffer == nullptr) return false;

		D3D11_MAPPED_SUBRESOURCE map;
		if (hrshouldbe(s_main->m_ctx->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
		{
			memcpy(map.pData,data.begin(), size);
			s_main->m_ctx->Unmap(buffer, 0);
		}
		return false;
	}
}
