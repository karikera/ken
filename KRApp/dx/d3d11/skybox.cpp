#ifdef WIN32
#include "skybox.h"
#include "state.h"
#include "internal.h"

using namespace kr;
using namespace d3d11;

const float SkyBox::VERTECIES[8][3] =
{
	{ -1.f, 1.f,-1.f },
	{ 1.f, 1.f,-1.f },
	{ -1.f,-1.f,-1.f },
	{ 1.f,-1.f,-1.f },

	{ 1.f, 1.f, 1.f },
	{ -1.f, 1.f, 1.f },
	{ 1.f,-1.f, 1.f },
	{ -1.f,-1.f, 1.f },
};
const unsigned short SkyBox::INDECES[36] =
{
	0,2,1, 1,2,3,
	4,6,5, 5,6,7,
	1,3,4, 4,3,6,
	5,7,0, 0,7,2,
	1,4,0, 0,4,5,
	2,7,3, 3,7,6,
};
const VertexLayout SkyBox::VERTEX_LAYOUT[1] = {
	VertexLayout(VertexLayout::Position, DXGI_FORMAT_R32G32B32_FLOAT)
};

SkyBox::SkyBox() noexcept
{
}
SkyBox::SkyBox(StateCache * state, Com<ID3D11ShaderResourceView> cubetex) noexcept
	: m_state(state), m_vb(kr::Buffer::fromAny(VERTECIES)), m_ib(kr::Buffer::fromAny(INDECES))
{
	m_texsrv = cubetex;
}
SkyBox::~SkyBox() noexcept
{
}
void SkyBox::draw() noexcept
{
	UINT oref;
	ID3D11DeviceContext *context = *m_state->getContext();

	DepthStencilState ods;
	context->OMGetDepthStencilState(&ods, &oref);
	context->OMSetDepthStencilState(m_state->depthStencil(false), 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT vbstride = sizeof(VERTECIES[0]);
	UINT vboffset = 0;
	context->IASetVertexBuffers(0, 1, &m_vb, &vbstride, &vboffset);
	context->IASetIndexBuffer(m_ib, DXGI_FORMAT_R16_UINT, 0);
	context->PSSetShaderResources(0, 1, &m_texsrv);
	context->DrawIndexed(_countof(INDECES), 0, 0);

	context->OMSetDepthStencilState(ods, oref);
}
#endif