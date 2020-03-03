#include "stdafx.h"
#include "fixedmatrix.h"

kr::fixedmatrix2::fixedmatrix2() noexcept
{
}
kr::fixedmatrix2::fixedmatrix2(fixed i11, fixed i12, fixed i21, fixed i22) noexcept
{
	_11=i11;
	_12=i12;
	_21=i21;
	_22=i22;
}
void kr::fixedmatrix2::Identity() noexcept
{
	_11=1;
	_21=0;
	_12=0;
	_22=1;
}
void kr::fixedmatrix2::SetRotation(float angle) noexcept
{
	float C=math::cos(angle);
	float S=math::sin(angle);

	_11=(fixed)C;
	_12=(fixed)S;
	_21=(fixed)-S;
	_22=(fixed)S;
}
void kr::fixedmatrix2::SetScale(fixed sx, fixed sy) noexcept
{
	_11=(fixed)sx;
	_21=0;
	_12=0;
	_22=(fixed)sy;
}
#ifdef WIN32
#include <KR3/win/windows.h>

kr::fixedmatrix2::operator MAT2() noexcept
{
	return (MAT2&)*this;
}
#endif