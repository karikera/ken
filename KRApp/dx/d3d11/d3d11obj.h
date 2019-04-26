#pragma once

#include "device.h"
#include "view.h"
#include "statecache.h"

namespace kr
{
	namespace d3d11
	{
		class Direct3D11:public StateCache
		{
		public:
			struct Init
			{
				Init() noexcept;
				~Init() noexcept;
			};

			Direct3D11() noexcept;
			DeviceContext * operator ->() noexcept;

			static void setDevice(ID3D11Device * device) noexcept;
			static ID3D11Device * getDevice() noexcept;
			void create(win::Window * wnd, int width, int height, int multiSampling = 1) noexcept;
			void clear() noexcept;

		private:
			RenderTargetView m_rtv;
			DepthStencilView m_dsv;
			
		};

	}

}