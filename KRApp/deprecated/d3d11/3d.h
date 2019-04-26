#pragma once

#ifdef KRGL_USE_D3D11
namespace kr
{
	namespace gl
	{
		enum class Draw
		{
			Points,
			Lines,
			LineStrip,
			Triangles,
			TriangleStrip,
		};
	}
}

#else
#error Need implement
#endif

#include <KR3/main.h>
#include <KR3/wl/windows.h>
#include <KRWin/winx.h>
#include <KR3/math/coord.h>
#include "buffer.h"
#include "blendmode.h"
#include "depthmode.h"
#include "rendermode.h"
#include "samplerstate.h"
#include "tdepth.h"
#include "rendertarget.h"
#include "filetex.h"
#include "program.h"

#include <iomanip>
#include <unordered_map>

struct IDXGIAdapter;
struct ID3D11DeviceContext;
struct ID3D11Device;
struct IDXGISwapChain;

namespace kr
{
	namespace gl
	{
		struct Config
		{
			pcstr16 wndName;
			irectwh rect;
			bool fullScreen;
			int backBufferMultisample;
		};
		class Window3D abstract :public WindowProgram
		{
		public:
			Window3D(const Config * config) noexcept;
			~Window3D() noexcept;
		
			void vdraw(Draw draw, std::size_t vcount, std::size_t voffset=0) noexcept;
			void idraw(Draw draw, std::size_t icount, std::size_t ioffset=0, std::intptr_t voffset=0) noexcept;
			void setViewport(const irectwh &rc) noexcept;

			const irectwh & getWindowRect() noexcept;
			int getMultiSampleCount() noexcept;
			void initStates() noexcept;
			void clear(const vec4 &color) noexcept;

			int processing() noexcept;
		
			virtual void onStep() noexcept = 0;
			virtual void onRender() noexcept = 0;

	#ifndef KRGL_PRIVATE
		private:
	#endif

	#ifdef KRGL_USE_D3D11
			void _closeSwapChain() noexcept;
			void _setTopology(Draw draw) noexcept;
			void _setMonitor(IDXGIAdapter * adapter) noexcept;
			void _updateUniform() noexcept;

			struct UniformBuffer
			{
				Keep<ID3D11Buffer>	buffer;
				std::size_t			size;

				UniformBuffer() noexcept;
				bool update(const Array<byte> &data) noexcept;
			};
		
			std::unordered_map<Resolution, TempDepthTexture*> m_tempdepth;
			std::unordered_map<std::string, TextureFile::TextureFileRef*> m_texturelist;
			Keep<ID3D11DeviceContext>		m_ctx;
			Keep<ID3D11Device>				m_device;
			Keep<IDXGISwapChain>			m_swapchain;
			Keep<ID3D11RasterizerState>		m_rasterizers[RenderMode::Count];
			Keep<ID3D11SamplerState>		m_samplerStates[SamplerState::Count];
			Keep<ID3D11DepthStencilState>	m_depthStates[DepthMode::Count];
			Keep<ID3D11BlendState>			m_blendStates[BlendMode::Count];
			Keep<ID3D11Buffer>				m_fsbuffer;
			Program *						m_curProgram;
			RenderTarget					m_wndRenderTarget;
			DepthMode						m_curDepthState;
			BlendMode						m_curBlendState;
			RenderMode						m_curRenderState;
			UniformBuffer					m_vsBuffer, m_fsBuffer, m_gsBuffer;
			irectwh							m_wndrect;
			Draw							m_prevdraw;
			int								m_samples;
			bool							m_FSEnable;
			bool							m_isProgramG;
	#else
	#endif
		};
	}
}
