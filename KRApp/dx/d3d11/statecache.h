#pragma once

#include "common.h"
#include "device.h"
#include <KR3/data/map.h>

namespace kr
{
	namespace d3d11
	{
		class StateIndex;
		class StateCache;

		class CachedTexture:public Referencable<CachedTexture>
		{
			friend StateCache;
		public:
			CachedTexture(const CachedTexture&) = delete;
			CachedTexture& operator =(const CachedTexture&) = delete;

			~CachedTexture() noexcept;

			ShaderResourceView getShaderResourceView() noexcept;

		private:
			explicit CachedTexture(StateCache * cache, ID3D11DeviceContext * ctx, AText16 filename);

			const AText16 m_filename;
			Com<ID3D11ShaderResourceView> m_view;
			StateCache * const m_cache;
		};

		class TextureFile
		{
		public:
			TextureFile() = default;
			TextureFile(Keep<CachedTexture> texture) noexcept;

		private:
			Keep<CachedTexture> m_texture;
		};

		class StateCache
		{
			friend CachedTexture;
		public:
			TextureFile loadTexture(AText16 name) noexcept;

			StateCache() noexcept;
			StateCache(ID3D11DeviceContext * ctx) noexcept;

			void setContext(DeviceContext context) noexcept;
			DeviceContext* getContext() noexcept;
			void cleanUpState() noexcept;


			BlendState blend(BlendMode mode);
			SamplerState sampler(TextureAddressMode ta, Filter filter);
			RasterizerState rasterizer(CullMode cull, FillMode fill, bool depthClipping);
			DepthStencilState depthStencil(ComparationFunc cmp, DepthWriteMask mask);
			DepthStencilState depthStencil(bool enable);

		protected:
			DeviceContext m_ctx;

		private:
			ID3D11BlendState * m_blends[
				(size_t)BlendMode::Count
			];

			ID3D11SamplerState *m_samplers[
				StateConst<TextureAddressMode>::count * StateConst<Filter>::count
			];
			ID3D11RasterizerState *m_rasterizer[
				StateConst<CullMode>::count * StateConst<FillMode>::count
			];
			ID3D11DepthStencilState *m_depthStencil[
				StateConst<ComparationFunc>::count * StateConst<DepthWriteMask>::count + 1
			];
			Map<Text16, CachedTexture*, true> m_textures;

		};
	}
}
