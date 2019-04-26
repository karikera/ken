#ifdef WIN32
#include "device.h"
#include "texture.h"
#include "view.h"

#include "internal.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

using namespace kr;
using namespace d3d11;

void ComMethod<ID3D11DeviceContext>::copy(ID3D11Texture2D * to, ID3D11Texture2D * from) noexcept
{
	ptr()->CopyResource(to, from);
}
dxgi::SwapChain ComMethod<ID3D11DeviceContext>::create(win::Window * wnd, int width, int height, int multiSampling) throws(ErrorCode)
{
	assert(multiSampling >= 0);
	assert(multiSampling <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT);
	
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};
	D3D_FEATURE_LEVEL featureLevel;

	DXGI_SWAP_CHAIN_DESC sd;
	memset(&sd, 0, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = wnd;
	sd.SampleDesc.Count = multiSampling;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;


	dxgi::SwapChain chain;
	{
		HRESULT hr;
		const D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE
		};

		for (D3D_DRIVER_TYPE driverType : driverTypes)
		{
#ifdef _DEBUG
			hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG, featureLevels, 6, D3D11_SDK_VERSION,
				&sd, &chain, &s_d3d11, &featureLevel, &ptr());
			if (SUCCEEDED(hr)) goto __succeeded;
#endif
			hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, 6, D3D11_SDK_VERSION,
				&sd, &chain, &s_d3d11, &featureLevel, &ptr());
			if (SUCCEEDED(hr)) goto __succeeded;
		}
		throw ErrorCode(hr);
	}
__succeeded:
	return chain;
}
void ComMethod<ID3D11DeviceContext>::clear(ID3D11RenderTargetView * rtv, const vec4 & color) noexcept
{
	ptr()->ClearRenderTargetView(rtv, color.m_data);
}
void ComMethod<ID3D11DeviceContext>::clear(ID3D11DepthStencilView * dsv) noexcept
{
	ptr()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}
void ComMethod<ID3D11DeviceContext>::setViewport(float width, float height) noexcept
{
	Viewport vp;
	vp.width = width;
	vp.height = height;
	vp.maxDepth = 1.f;
	vp.minDepth = 0.f;
	vp.topLeftX = 0.f;
	vp.topLeftY = 0.f;
	setViewport(vp);
}
void ComMethod<ID3D11DeviceContext>::setViewport(const d3d11::Viewport & viewport) noexcept
{
	ptr()->RSSetViewports(1, &(D3D11_VIEWPORT&)viewport);
}
d3d11::RenderTargetSet ComMethod<ID3D11DeviceContext>::getRenderTarget() noexcept
{
	RenderTargetSet rts;
	ptr()->OMGetRenderTargets(1, &rts.renderTargetView, &rts.depthStencilView);
	return rts;
}
void ComMethod<ID3D11DeviceContext>::setVertexShader(ID3D11VertexShader * vs) noexcept
{
	ptr()->VSSetShader(vs, nullptr, 0);
}
d3d11::VertexShader ComMethod<ID3D11DeviceContext>::getVertexShader() noexcept
{
	VertexShader shader;
	ptr()->VSGetShader(&shader, nullptr, nullptr);
	return shader;
}
void ComMethod<ID3D11DeviceContext>::setPixelShader(ID3D11PixelShader * ps) noexcept
{
	ptr()->PSSetShader(ps, nullptr, 0);
}
d3d11::PixelShader ComMethod<ID3D11DeviceContext>::getPixelShader() noexcept
{
	PixelShader shader;
	ptr()->PSGetShader(&shader, nullptr, nullptr);
	return shader;
}
void ComMethod<ID3D11DeviceContext>::setGeometry(ID3D11GeometryShader * gs) noexcept
{
	ptr()->GSSetShader(gs, nullptr, 0);
}
d3d11::GeometryShader ComMethod<ID3D11DeviceContext>::getGeometryShader() noexcept
{
	GeometryShader shader;
	ptr()->GSGetShader(&shader, nullptr, nullptr);
	return shader;
}
void ComMethod<ID3D11DeviceContext>::setRenderTarget(const d3d11::RenderTargetSet & rts) noexcept
{
	setRenderTarget(rts.renderTargetView, rts.depthStencilView);
}
void ComMethod<ID3D11DeviceContext>::setRenderTarget(ID3D11RenderTargetView * rtv, ID3D11DepthStencilView * dsv) noexcept
{
	ptr()->OMSetRenderTargets(1, &rtv, dsv);
}
d3d11::Viewport ComMethod<ID3D11DeviceContext>::getViewport() noexcept
{
	D3D11_VIEWPORT vp;
	UINT num = 1;
	ptr()->RSGetViewports(&num, &vp);
	return (Viewport&)vp;
}
void ComMethod<ID3D11DeviceContext>::setRasterizerState(ID3D11RasterizerState * state) noexcept
{
	ptr()->RSSetState(state);
}
void ComMethod<ID3D11DeviceContext>::setDepthStencilState(ID3D11DepthStencilState * state, uint ref) noexcept
{
	ptr()->OMSetDepthStencilState(state, ref);
}
d3d11::DepthStencilStateSet ComMethod<ID3D11DeviceContext>::getDepthStencilState() noexcept
{
	DepthStencilStateSet set;
	ptr()->OMGetDepthStencilState(&set.state, (UINT*)&set.ref);
	return set;
}
void ComMethod<ID3D11DeviceContext>::setBlendState(ID3D11BlendState * state, const vec4 &blendFactor, uint sampleMask) noexcept
{
	ptr()->OMSetBlendState(state, blendFactor.m_data, sampleMask);
}
void ComMethod<ID3D11DeviceContext>::setBlendState(ID3D11BlendState * state) noexcept
{
	ptr()->OMSetBlendState(state, nullptr, 0);
}
d3d11::BlendStateSet ComMethod<ID3D11DeviceContext>::getBlendState() noexcept
{
	d3d11::BlendStateSet set;
	ptr()->OMGetBlendState(&set.state, set.blendFactor.m_data, (UINT*)&set.sampleMask);
	return set;
}
void ComMethod<ID3D11DeviceContext>::setPixelResource(uint index, ID3D11ShaderResourceView * view) noexcept
{
	ptr()->PSSetShaderResources(index, 1, &view);
}
void ComMethod<ID3D11DeviceContext>::setPixelSampler(uint index, ID3D11SamplerState * sampler) noexcept
{
	ptr()->PSSetSamplers(index, 1, &sampler);
}
void ComMethod<ID3D11DeviceContext>::setVertexBuffer(uint slot, ID3D11Buffer * buffer, uint stride, uint offset) noexcept
{
	ptr()->IASetVertexBuffers(slot, 1, &buffer, &stride, &offset);
}
void ComMethod<ID3D11DeviceContext>::setIndexBuffer(ID3D11Buffer * buffer, DXGI_FORMAT format, uint offset) noexcept
{
	ptr()->IASetIndexBuffer(buffer, format, offset);
}
void ComMethod<ID3D11DeviceContext>::setPrimitiveTopology(PrimitiveTopology topology) noexcept
{
	ptr()->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)topology);
	
}
void ComMethod<ID3D11DeviceContext>::draw(uint vertexCount, uint startVertexLocation) noexcept
{
	ptr()->Draw(vertexCount, startVertexLocation);
}
void ComMethod<ID3D11DeviceContext>::drawIndexed(uint indexCount, uint startIndexLocation, uint baseVertexLocation) noexcept
{
	ptr()->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}

int d3d11::getTextureMaxSize() noexcept
{
	switch (s_d3d11->GetFeatureLevel())
	{
	case D3D_FEATURE_LEVEL_9_1:
	case D3D_FEATURE_LEVEL_9_2:
		return 2048; /*D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;

	case D3D_FEATURE_LEVEL_9_3:
		return 4096 /*D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;

	case D3D_FEATURE_LEVEL_10_0:
	case D3D_FEATURE_LEVEL_10_1:
		return 8192 /*D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;

	default:
		return D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	}
}


#endif