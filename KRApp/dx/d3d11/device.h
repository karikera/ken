#pragma once

#include "common.h"
#include "texture.h"
#include "view.h"
#include <map>

#include <KRWin/handle.h>


namespace kr
{
	namespace d3d11
	{
		struct Viewport
		{
			float topLeftX;
			float topLeftY;
			float width;
			float height;
			float minDepth;
			float maxDepth;
		};
		struct RenderTargetSet
		{
			RenderTargetView renderTargetView;
			DepthStencilView depthStencilView;
		};
		struct DepthStencilStateSet
		{
			DepthStencilState state;
			uint ref;
		};
		struct BlendStateSet
		{
			BlendState state;
			vec4 blendFactor;
			uint sampleMask;
		};
		int getTextureMaxSize() noexcept;

		enum PrimitiveTopology
		{
			POINTLIST = 1,
			LINELIST = 2,
			LINESTRIP = 3,
			TRIANGLELIST = 4,
			TRIANGLESTRIP = 5,
		};
	}

	COM_CLASS_METHODS(ID3D11DeviceContext)
	{
		void copy(ID3D11Texture2D * to, ID3D11Texture2D * from) noexcept;
		dxgi::SwapChain create(win::Window * wnd, int width, int height, int multiSampling = 1) throws(ErrorCode);
		void clear(ID3D11RenderTargetView * rtv, const vec4& color = {0,0,0,0}) noexcept;
		void clear(ID3D11DepthStencilView * dsv) noexcept;
		void setViewport(float width, float height) noexcept;
		void setViewport(const d3d11::Viewport & viewport) noexcept;
		d3d11::Viewport getViewport() noexcept;
		d3d11::RenderTargetSet getRenderTarget() noexcept;
		void setVertexShader(ID3D11VertexShader * vs) noexcept;
		d3d11::VertexShader getVertexShader() noexcept;
		void setPixelShader(ID3D11PixelShader * ps) noexcept;
		d3d11::PixelShader getPixelShader() noexcept;
		void setGeometry(ID3D11GeometryShader * gs) noexcept;
		d3d11::GeometryShader getGeometryShader() noexcept;
		void setRenderTarget(const d3d11::RenderTargetSet & rts) noexcept;
		void setRenderTarget(ID3D11RenderTargetView * rtv, ID3D11DepthStencilView * dsv) noexcept;
		void setRasterizerState(ID3D11RasterizerState * state) noexcept;
		void setDepthStencilState(ID3D11DepthStencilState * state, uint ref) noexcept;
		d3d11::DepthStencilStateSet getDepthStencilState() noexcept;
		void setBlendState(ID3D11BlendState * state, const vec4 &blendFactor, uint sampleMask) noexcept;
		void setBlendState(ID3D11BlendState * state) noexcept;
		d3d11::BlendStateSet getBlendState() noexcept;
		void setPixelResource(uint index, ID3D11ShaderResourceView * view) noexcept;
		void setPixelSampler(uint index, ID3D11SamplerState * sampler) noexcept;
		void setVertexBuffer(uint slot, ID3D11Buffer * buffer, uint stride, uint offset) noexcept;
		void setIndexBuffer(ID3D11Buffer * buffer, DXGI_FORMAT format, uint offset) noexcept;
		void setPrimitiveTopology(d3d11::PrimitiveTopology topology) noexcept;
		void draw(uint vertexCount, uint startVertexLocation) noexcept;
		void drawIndexed(uint indexCount, uint startIndexLocation, uint baseVertexLocation) noexcept;
	};

}