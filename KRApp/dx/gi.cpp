#ifdef WIN32
#include "gi.h"

#include <KR3/wl/windows.h>
#include "dxcommon.h"
#include DXGI_HEADER
#include D2D_HEADER

#include "d3d11/internal.h"
#include "d3d11/texture.h"

using namespace kr;
using namespace dxgi;

const Adapters & dxgi::adapters = nullref;

IDXGIDevice * dxgi::getDevice() noexcept
{
	return d3d11::s_dxgiDevice;
}

Adapter& Adapters::Iterator::operator *() noexcept
{
	return adapter;
}
Adapters::Iterator & Adapters::Iterator::operator ++() throws(ErrorCode)
{
	adapter = adapters[++index];
	return *this;
}
Adapters::Iterator & Adapters::Iterator::operator --() throws(ErrorCode)
{
	adapter = adapters[--index];
	return *this;
}
bool Adapters::Iterator::operator ==(const Iterator & iter) noexcept
{
	return adapter == iter.adapter;
}
bool Adapters::Iterator::operator !=(const Iterator & iter) noexcept
{
	return adapter != iter.adapter;
}
Adapters::Iterator Adapters::begin() const noexcept
{
	return { (*this)[0], 0 };
}
Adapters::Iterator Adapters::end() const noexcept
{
	return { nullptr, 0 };
}
Adapter Adapters::operator [](UINT idx) const throws(ErrorCode)
{
	Adapter adapter;
	HRESULT hr = d3d11::s_dxFactory->EnumAdapters(idx, &adapter);
	if (FAILED(hr))
	{
		if (hr != DXGI_ERROR_NOT_FOUND) throw ErrorCode(hr);
	}
	return adapter;
}

Output& ComMethod<IDXGIAdapter>::Iterator::operator *() noexcept
{
	return output;
}
ComMethod<IDXGIAdapter>::Iterator&  ComMethod<IDXGIAdapter>::Iterator::operator ++() throws(ErrorCode)
{
	output = (*adapter)[++index];
	return *this;
}
ComMethod<IDXGIAdapter>::Iterator & ComMethod<IDXGIAdapter>::Iterator::operator --() throws(ErrorCode)
{
	output = (*adapter)[--index];
	return *this;
}
bool ComMethod<IDXGIAdapter>::Iterator::operator ==(const Iterator & iter) noexcept
{
	return output == iter.output;
}
bool ComMethod<IDXGIAdapter>::Iterator::operator !=(const Iterator & iter) noexcept
{
	return output != iter.output;
}
ComMethod<IDXGIAdapter>::Iterator ComMethod<IDXGIAdapter>::begin() const noexcept
{
	return { static_cast<const Adapter*>(this), (*this)[0], 0};
}
ComMethod<IDXGIAdapter>::Iterator ComMethod<IDXGIAdapter>::end() const noexcept
{
	return { static_cast<const Adapter*>(this), nullptr, 0};
}
Output ComMethod<IDXGIAdapter>::operator [](UINT idx) const throws(ErrorCode)
{
	Output output;
	HRESULT hr = ptr()->EnumOutputs(idx, &output);
	if (FAILED(hr))
	{
		if (hr != DXGI_ERROR_NOT_FOUND) throw ErrorCode(hr);
	}
	return output;
}

#ifdef USE_D2D2
void ComMethod<IDXGISwapChain>::setTarget(ID2D1DeviceContext * target)
{
	Com<IDXGISurfaceT> surface;
	Com<ID2D1BitmapT> bitmap;

	// Retrieve the swap chain's back buffer
	hrexcept(ptr()->GetBuffer(0, __uuidof(IDXGISurfaceT), &surface));

	
	// Create a Direct2D bitmap that points to the swap chain surface
	D2D1_BITMAP_PROPERTIES1 properties = {};
	properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

	hrexcept(target->CreateBitmapFromDxgiSurface(surface, properties, &bitmap));

	// Point the device context to the bitmap for rendering
	target->SetTarget(bitmap);
}
#endif
void ComMethod<IDXGISwapChain>::present() noexcept
{
	ptr()->Present(
		1, // sync
		0 // flags
	);
}
Com<ID3D11Texture2D> ComMethod<IDXGISwapChain>::getTexture()
{
	Com<ID3D11Texture2D> texture;
	hrexcept(ptr()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture));
	return texture;
}

#ifdef USE_D2D2
void ComMethod<IDXGISwapChain1>::createForComposition(int width, int height)
{
	remove();

	DXGI_SWAP_CHAIN_DESC_T description = {};
	description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	description.BufferCount = 2;
	description.SampleDesc.Count = 1;
	description.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

	description.Width = width;
	description.Height = height;

	hrexcept(d3d11::s_dxFactory->CreateSwapChainForComposition(d3d11::s_dxgiDevice,
		&description,
		nullptr, // Don¡¯t restrict
		&ptr()));
}
void ComMethod<IDXGISurface>::setFrom(ID3D11Texture2D * texture)
{
	texture->QueryInterface<IDXGISurface>(&ptr());
}
void ComMethod<IDXGISurface1>::setFrom(ID3D11Texture2D * texture)
{
	texture->QueryInterface<IDXGISurface1>(&ptr());
}
#endif

#endif