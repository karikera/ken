#pragma once

#include "../3d.h"

namespace kr
{
	namespace gl
	{

		class FrustumIB;

		class AxisVB
		{
		public:
			AxisVB() noexcept;
			void use() noexcept;
			void draw() noexcept;

		private:
			VertexBuffer m_vb;
		};

		class RectVB
		{
		public:
			RectVB() noexcept;
			void use() noexcept;
			void draw() noexcept;

		private:
			VertexBuffer m_vb;
		};

		class FrustumIB
		{
		public:
			static constexpr std::size_t VERTEX_COUNT = 9;
			FrustumIB() noexcept;
			void render(const VertexBuffer &line);

		private:
			IndexBuffer m_ib;
		};
	}
}