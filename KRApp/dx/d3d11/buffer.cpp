
#ifdef WIN32

#include "buffer.h"
#include "internal.h"

using namespace kr;
using namespace d3d11;

ComMethod<ID3D11Buffer>::ComMethod(UINT bindflags, size_t size)
{
	create(Usage::Default, (CpuAccess)0, bindflags, size);
}
ComMethod<ID3D11Buffer>::ComMethod(UINT bindflags, kr::Buffer initdata)
{
	create(Usage::Default, (CpuAccess)0, bindflags, initdata);
}

void ComMethod<ID3D11Buffer>::create(Usage usage, CpuAccess cpuflags, uint bindflags, size_t size)
{
	remove();

	D3D11_BUFFER_DESC bd;
	memset(&bd, 0, sizeof(bd));
	bd.Usage = (D3D11_USAGE)usage;
	bd.CPUAccessFlags = (D3D11_CPU_ACCESS_FLAG)cpuflags;
	bd.ByteWidth = intact<UINT>(size);
	bd.BindFlags = bindflags;
	hrexcept(s_d3d11->CreateBuffer(&bd, nullptr, &ptr()));
}
void ComMethod<ID3D11Buffer>::create(Usage usage, CpuAccess cpuflags, uint bindflags, kr::Buffer initdata)
{
	remove();

	D3D11_SUBRESOURCE_DATA sd;
	memset(&sd, 0, sizeof(sd));
	sd.pSysMem = initdata.begin();

	D3D11_BUFFER_DESC bd;
	memset(&bd, 0, sizeof(bd));
	bd.Usage = (D3D11_USAGE)usage;
	bd.CPUAccessFlags = (D3D11_CPU_ACCESS_FLAG)cpuflags;
	bd.ByteWidth = intact<UINT>(initdata.size());
	bd.BindFlags = bindflags;

	hrexcept(s_d3d11->CreateBuffer(&bd, &sd, &ptr()));
}

kr::d3d11::DynamicBuffer::DynamicBuffer(uint bindflags, size_t size)
{
	create(Usage::Dynamic, CpuAccess::Write, bindflags, size);
}
kr::d3d11::DynamicBuffer::DynamicBuffer(uint bindflags, kr::Buffer initdata)
{
	create(Usage::Dynamic, CpuAccess::Write, bindflags, initdata);
}
void kr::d3d11::DynamicBuffer::create(uint bindflags, size_t size)
{
	create(Usage::Dynamic, CpuAccess::Write, bindflags, size);
}
void kr::d3d11::DynamicBuffer::create(uint bindflags, kr::Buffer initdata)
{
	create(Usage::Dynamic, CpuAccess::Write, bindflags, initdata);
}
void * kr::d3d11::DynamicBuffer::map(ID3D11DeviceContext * dc)
{
	D3D11_MAPPED_SUBRESOURCE res;
	hrexcept(dc->Map(ptr(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res));
	return res.pData;
}
void kr::d3d11::DynamicBuffer::unmap(ID3D11DeviceContext * dc) noexcept
{
	dc->Unmap(ptr(), 0);
}

kr::d3d11::VertexBuffer::VertexBuffer(kr::Buffer initdata) :Buffer(BindVertexBuffer, initdata)
{
}

kr::d3d11::IndexBuffer::IndexBuffer(kr::Buffer initdata) : Buffer(BindIndexBuffer, initdata)
{
}

kr::d3d11::ConstBuffer::ConstBuffer(size_t sz) : Buffer(BindConstantBuffer, sz)
{
}

#endif