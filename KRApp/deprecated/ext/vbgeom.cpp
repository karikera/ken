#include "vbgeom.h"

namespace
{
	struct TexVertex
	{
		kr::vec3 position;
		kr::vec2 texcoord;
	};
	const kr::vec3 v[]=
	{
		{ 1,0,0 },
		{ -1,0,0 },
		{ 0,1,0 },
		{ 0,-1,0 },
		{ 0,0,1 },
		{ 0,0,-1 },
	};
	const std::uint16_t FRUSTUM_IB[]=
	{
		0,5, 0,6, 0,7, 0,8,
		5,6, 6,7, 7,8, 8,5,
		1,2, 2,3, 3,4, 4,1,
	};
	const TexVertex rect[] =
	{
		{{-1,  1, 0}, {0,0}},
		{{ 1,  1, 0}, {1,0}},
		{{-1, -1, 0}, {0,1}},
		{{ 1, -1, 0}, {1,1}},
	};				  
}


kr::gl::AxisVB::AxisVB() noexcept
	:m_vb(v)
{
}
void kr::gl::AxisVB::use() noexcept
{
	m_vb.use(sizeof(vec3));
}
void kr::gl::AxisVB::draw() noexcept
{
	use();
	wnd->vdraw(Draw::Lines, 6);
}

kr::gl::RectVB::RectVB() noexcept
	:m_vb(rect)
{
}
void kr::gl::RectVB::use() noexcept
{
	m_vb.use(sizeof(TexVertex));
}
void kr::gl::RectVB::draw() noexcept
{
	use();
	wnd->vdraw(Draw::TriangleStrip,4);
}

kr::gl::FrustumIB::FrustumIB() noexcept
	:m_ib(FRUSTUM_IB)
{
}
void kr::gl::FrustumIB::render(const VertexBuffer & line)
{
	line.use(sizeof(vec3));
	m_ib.use();
	wnd->idraw(kr::gl::Draw::Lines, 24);
}
