#pragma once

#include "common.h"
#include "device.h"
#include "buffer.h"
#include "shader.h"
#include "statecache.h"

namespace kr
{
	namespace d3d11
	{
		class SkyBox
		{
		public:
			SkyBox() noexcept;
			SkyBox(StateCache * state, ShaderResourceView cubetex) noexcept;
			~SkyBox() noexcept;

			void draw() noexcept;

			static const float VERTECIES[8][3];
			static const unsigned short INDECES[36];
			static const VertexLayout VERTEX_LAYOUT[1];

		private:
			VertexBuffer m_vb;
			IndexBuffer m_ib;
			ShaderResourceView m_texsrv;
			StateCache * m_state;
		};
	}
}
