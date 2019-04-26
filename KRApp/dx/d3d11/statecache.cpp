#ifdef WIN32
#include "statecache.h"
#include <K2D/wic/WICTextureLoader.h>
#include <KR3/util/wide.h>

#include "state.h"
#include "internal.h"

using namespace kr;
using namespace d3d11;

CachedTexture::~CachedTexture() noexcept
{
	if (!m_filename.empty())
	{
		m_cache->m_textures.erase(m_filename);
	}
}
CachedTexture::CachedTexture(StateCache * cache, ID3D11DeviceContext * ctx, AText16 filename)
	:m_cache(cache), m_filename(std::move(filename))
{
	hrexcept(DirectX::CreateWICTextureFromFile(s_d3d11, ctx, wide(m_filename.data()), nullptr, &m_view));
}
ShaderResourceView CachedTexture::getShaderResourceView() noexcept
{
	return m_view;
}

BlendState StateCache::blend(BlendMode mode)
{
	ID3D11BlendState *& bs = m_blends[(size_t)mode];
	if (bs == nullptr)
	{
		D3D11_BLEND_DESC bm = {};
		switch (mode)
		{
		case BlendMode::None:
			bm.RenderTarget[0].BlendEnable = false;
			bm.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;
		case BlendMode::Normal:
			bm.RenderTarget[0].BlendEnable = true;
			bm.RenderTarget[0].BlendOp = bm.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			bm.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			bm.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			bm.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			bm.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			bm.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;
		case BlendMode::Add:
			bm.RenderTarget[0].BlendEnable = true;
			bm.RenderTarget[0].BlendOp = bm.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			bm.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			bm.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			bm.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			bm.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			bm.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;
		case BlendMode::Subtract:
			bm.RenderTarget[0].BlendEnable = true;
			bm.RenderTarget[0].BlendOp = bm.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_SUBTRACT;
			bm.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			bm.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			bm.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			bm.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			bm.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;
		case BlendMode::Multiply:
			bm.RenderTarget[0].BlendEnable = true;
			bm.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			bm.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_SUBTRACT;
			bm.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
			bm.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			bm.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			bm.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			bm.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;
		default:
			assert(!"Undefined blend type");
			break;
		}
		hrexcept(s_d3d11->CreateBlendState(&bm, &bs));
	}
	return bs;
}
SamplerState StateCache::sampler(TextureAddressMode ta, Filter filter)
{
	int index = (StateIndex)ta | filter;
	assert(index < countof(m_samplers));
	ID3D11SamplerState * & s = m_samplers[index];
	if (s == nullptr)
	{
		D3D11_SAMPLER_DESC sd = {};
		sd.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)ta;
		sd.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)ta;
		sd.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)ta;
		sd.Filter = (D3D11_FILTER)filter;

		if (filter == Filter::ANISOTROPIC)
		{
			sd.MaxAnisotropy = 4;
		}

		sd.MinLOD = 0;
		sd.MaxLOD = D3D11_FLOAT32_MAX;

		hrexcept(s_d3d11->CreateSamplerState(&sd, &s));
	}
	return s;
}
RasterizerState StateCache::rasterizer(CullMode cull, FillMode fill, bool depthClipping)
{
	int index = (StateIndex)cull | fill;
	assert(index < countof(m_rasterizer));
	ID3D11RasterizerState * & r = m_rasterizer[index];
	if (r == nullptr)
	{
		D3D11_RASTERIZER_DESC rd = {};
		rd.CullMode = (D3D11_CULL_MODE)cull;
		rd.FillMode = (D3D11_FILL_MODE)fill;
		rd.DepthClipEnable = depthClipping;
		hrexcept(s_d3d11->CreateRasterizerState(&rd, &r));
	}
	return r;
}
DepthStencilState StateCache::depthStencil(ComparationFunc cmp, DepthWriteMask mask)
{
	int index = (StateIndex)cmp | mask;
	assert(index < countof(m_depthStencil));
	ID3D11DepthStencilState * & d = m_depthStencil[index];
	if (d == nullptr)
	{
		D3D11_DEPTH_STENCIL_DESC dsd = {};
		dsd.DepthEnable = true;
		dsd.DepthFunc = (D3D11_COMPARISON_FUNC)cmp;
		dsd.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK)mask;
		hrexcept(s_d3d11->CreateDepthStencilState(&dsd, &d));
	}
	return d;
}
DepthStencilState StateCache::depthStencil(bool enable)
{
	if (enable)
	{
		return depthStencil(ComparationFunc::LESS, DepthWriteMask::ALL);
	}

	int index = StateConst<ComparationFunc>::count * StateConst<DepthWriteMask>::count;
	assert(index < countof(m_depthStencil));
	ID3D11DepthStencilState * & d = m_depthStencil[index];
	if (d == nullptr)
	{
		D3D11_DEPTH_STENCIL_DESC dsd = {};
		dsd.DepthEnable = false;
		hrexcept(s_d3d11->CreateDepthStencilState(&dsd, &d));
	}
	return d;
}

TextureFile StateCache::loadTexture(AText16 name) noexcept
{
	if (name.empty()) return nullptr;

	name.c_str();

	auto res = m_textures.insert(name, nullptr);
	if (res.second)
	{
		res.first->second = _new CachedTexture(this, m_ctx, move(name));
	}
	return res.first->second;
}

StateCache::StateCache() noexcept
	:m_samplers{},
	m_rasterizer{},
	m_depthStencil{}
{
}
StateCache::StateCache(ID3D11DeviceContext * ctx) noexcept
	:m_ctx(move(ctx)),
	m_samplers{},
	m_rasterizer{},
	m_depthStencil{}
{
}
void StateCache::setContext(DeviceContext context) noexcept
{
	m_ctx = context;
}
DeviceContext* StateCache::getContext() noexcept
{
	return &m_ctx;
}
void StateCache::cleanUpState() noexcept
{
	for (auto * p : m_samplers)
	{
		if (p) p->Release();
	}
	for (auto * p : m_rasterizer)
	{
		if (p) p->Release();
	}
	for (auto * p : m_depthStencil)
	{
		if (p) p->Release();
	}
	memset(&m_samplers, 0, sizeof(m_samplers));
	memset(&m_samplers, 0, sizeof(m_rasterizer));
	memset(&m_samplers, 0, sizeof(m_depthStencil));
}

TextureFile::TextureFile(Keep<CachedTexture> texture) noexcept
	:m_texture(move(texture))
{
}

#endif