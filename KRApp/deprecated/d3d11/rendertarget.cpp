#define KRGL_PRIVATE
#include "rendertarget.h"
#include "include_d3d11.h"
#include "3d.h"

kr::gl::RenderTarget::RenderTarget() noexcept
{
}
kr::gl::RenderTarget::~RenderTarget() noexcept
{
}
kr::gl::RenderTarget::RenderTarget(const RenderTarget & copy) noexcept
{
	m_width = copy.m_width;
	m_height = copy.m_height;
	m_depth = copy.m_depth;
	m_rtv = copy.m_rtv;
	m_srv = copy.m_srv;
	m_tex = copy.m_tex;
}
kr::gl::RenderTarget::RenderTarget(RenderTarget && move) noexcept
{
	m_width = move.m_width;
	m_height = move.m_height;
	m_depth = std::move(move.m_depth);
	m_rtv = std::move(move.m_rtv);
	m_srv = std::move(move.m_srv);
	m_tex = std::move(move.m_tex);
}
kr::gl::RenderTarget::RenderTarget(int width, int height, int samples) noexcept
{
	m_width = width;
	m_height = height;

	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Width     = width;
	dsDesc.Height    = height;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;
	dsDesc.SampleDesc.Count = samples;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.Usage     = D3D11_USAGE_DEFAULT;
	dsDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags      = 0;

	ID3D11Device * d3ddevice = wnd->m_device;
	if (hrshouldbe(d3ddevice->CreateTexture2D(&dsDesc, nullptr, &m_tex)))
	{
		hrshouldbe(d3ddevice->CreateShaderResourceView(m_tex, nullptr, &m_srv));
		hrshouldbe(d3ddevice->CreateRenderTargetView(m_tex, nullptr, &m_rtv));
		m_depth = TempDepthTexture::create(width,height,samples);
	}
}
void kr::gl::RenderTarget::createFromSwapChain() noexcept
{
	m_tex = nullptr;
	m_rtv = nullptr;
	m_rtv = nullptr;
	m_depth = nullptr;
	m_width = wnd->m_wndrect.width;
	m_height = wnd->m_wndrect.height;
	if (hrshouldbe(wnd->m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_tex)))
	{
		hrshouldbe(wnd->m_device->CreateRenderTargetView(m_tex, nullptr, &m_rtv));
		m_depth = TempDepthTexture::create(wnd->m_wndrect.width, wnd->m_wndrect.height, wnd->m_samples);
	}
}
void kr::gl::RenderTarget::use() noexcept
{
	ID3D11DepthStencilView * dsv = m_depth->m_dsv;
	wnd->m_ctx->OMSetRenderTargets(1, &m_rtv, dsv);
}
void kr::gl::RenderTarget::useWithoutDepth() noexcept
{
	wnd->m_ctx->OMSetRenderTargets(1, &m_rtv, nullptr);
}
void kr::gl::RenderTarget::setViewport() noexcept
{
	wnd->setViewport({ {0, 0}, {m_width, m_height} });
}
int kr::gl::RenderTarget::getWidth() noexcept
{
	return m_width;
}
int kr::gl::RenderTarget::getHeight() noexcept
{
	return m_height;
}
kr::gl::TempDepthTexture * kr::gl::RenderTarget::getDepth() noexcept
{
	return m_depth;
}
kr::gl::RenderTarget & kr::gl::RenderTarget::operator=(const RenderTarget & tex) noexcept
{
	this->~RenderTarget();
	new(this) RenderTarget(tex);
	return *this;
}
kr::gl::RenderTarget & kr::gl::RenderTarget::operator=(RenderTarget && tex) noexcept
{
	this->~RenderTarget();
	new(this) RenderTarget(std::move(tex));
	return *this;
}
