#ifdef WIN32
#include "internal.h"

kr::Com<ID3D11Device> kr::d3d11::s_d3d11;
kr::Com<IDXGIDevice> kr::d3d11::s_dxgiDevice;
kr::Com<IDXGIAdapter> kr::d3d11::s_adapter;
kr::Com<IDXGIFactoryT> kr::d3d11::s_dxFactory;

bool kr::d3d11::isSupportMipAutogen(DXGI_FORMAT format) noexcept
{
	UINT fmtSupport = 0;
	HRESULT hr = s_d3d11->CheckFormatSupport(format, &fmtSupport);
	return (SUCCEEDED(hr) && (fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN));
}
#endif