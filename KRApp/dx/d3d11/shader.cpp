#ifdef WIN32
#include "shader.h"
#include "internal.h"

using namespace kr;
using namespace d3d11;

ComMethod<ID3D11InputLayout>::ComMethod(kr::Buffer data, View<VertexLayout> vl)
{
	create(data, vl);
}
void ComMethod<ID3D11InputLayout>::_create(kr::Buffer data, View<D3D11_INPUT_ELEMENT_DESC> desc)
{
	remove();
	hrexcept(s_d3d11->CreateInputLayout(desc.data(), intact<UINT>(desc.size()), data.data(), data.size(), &ptr()));
}
void ComMethod<ID3D11InputLayout>::create(kr::Buffer data, View<VertexLayout> vl)
{
	remove();

	TmpArray<D3D11_INPUT_ELEMENT_DESC> ildesc;

	unsigned int offset = 0;
	for (size_t i = 0; i < vl.size(); i++)
	{
		const VertexLayout &d = vl[i];
		D3D11_INPUT_ELEMENT_DESC & ie = ildesc[i];
		switch (d.semantic)
		{
		case VertexLayout::BiNormal:		ie.SemanticName = "BINORMAL"; break;
		case VertexLayout::BlendIndices:	ie.SemanticName = "BLENDINDICES"; break;
		case VertexLayout::BlendWeight:		ie.SemanticName = "BLENDWEIGHT"; break;
		case VertexLayout::Color:			ie.SemanticName = "COLOR"; break;
		case VertexLayout::Normal:			ie.SemanticName = "NORMAL"; break;
		case VertexLayout::Position:		ie.SemanticName = "POSITION"; break;
		case VertexLayout::PositionT:		ie.SemanticName = "POSITIONT"; break;
		case VertexLayout::PSize:			ie.SemanticName = "PSIZE"; break;
		case VertexLayout::Tangent:			ie.SemanticName = "TANGENT"; break;
		case VertexLayout::TexCoord:		ie.SemanticName = "TEXCOORD"; break;
		default: assert(!"Unknown blend setting");
		}
		VertexFormatInfo finfo(d.format);

		ie.SemanticIndex = d.slot;
		ie.Format = d.format;
		ie.InputSlot = 0;
		ie.AlignedByteOffset = offset;
		ie.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		ie.InstanceDataStepRate = 0;
		offset += finfo.byteSize;
	}
	_create(data, ildesc);
}

ComMethod<ID3D11VertexShader>::ComMethod(kr::Buffer data)
{
	create(data);
}
void ComMethod<ID3D11VertexShader>::create(kr::Buffer data)
{
	remove();
	hrexcept(s_d3d11->CreateVertexShader(data.begin(), data.size(), nullptr, &ptr()));
}

ComMethod<ID3D11PixelShader>::ComMethod(kr::Buffer data)
{
	create(data);
}
void ComMethod<ID3D11PixelShader>::create(kr::Buffer data)
{
	remove();
	hrexcept(s_d3d11->CreatePixelShader(data.begin(), data.size(), nullptr, &ptr()));
}

#endif