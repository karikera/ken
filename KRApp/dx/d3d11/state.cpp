#ifdef WIN32
#include "state.h"
#include "internal.h"

using namespace kr;
using namespace d3d11;

StateIndex::StateIndex() noexcept
{
	index = 0;
	maximum = 1;
}
StateIndex::operator int() const noexcept
{
	return index;
}

template <>
int StateIndex::make<Filter>(Filter value)
{
	int res;
	switch (value)
	{
	case Filter::MIN_MAG_MIP_POINT: res = 0; break;
	case Filter::MIN_MAG_POINT_MIP_LINEAR: res = 1; break;
	case Filter::MIN_POINT_MAG_LINEAR_MIP_POINT: res = 2; break;
	case Filter::MIN_POINT_MAG_MIP_LINEAR: res = 3; break;
	case Filter::MIN_LINEAR_MAG_MIP_POINT: res = 4; break;
	case Filter::MIN_LINEAR_MAG_POINT_MIP_LINEAR: res = 5; break;
	case Filter::MIN_MAG_LINEAR_MIP_POINT: res = 6; break;
	case Filter::MIN_MAG_MIP_LINEAR: res = 7; break;
	case Filter::ANISOTROPIC: res = 8; break;
	//case D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT: res = 10;
	//case D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR: res = 11;
	//case D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT: res = 12;
	//case D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR: res = 13;
	//case D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT: res = 14;
	//case D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR: res = 15;
	//case D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT: res = 16;
	//case D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR: res = 17;
	//case D3D11_FILTER_COMPARISON_ANISOTROPIC: res = 18;
	default: assert(!"unnkown value"); res = 0; break;
	}
	assert(res < StateConst<Filter>::count);
	return res;
}

#endif