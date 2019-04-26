#ifdef WIN32
#include "view.h"
#include "internal.h"

using namespace kr;
using namespace d3d11;

void ComMethod<ID3D11RenderTargetView>::create(ID3D11Texture2D * texture)
{
	remove();
	hrexcept(s_d3d11->CreateRenderTargetView(texture, nullptr, &ptr()));
}

void ComMethod<ID3D11DepthStencilView>::create(ID3D11Texture2D * texture)
{
	remove();
	hrexcept(s_d3d11->CreateDepthStencilView(texture, nullptr, &ptr()));
}

#endif