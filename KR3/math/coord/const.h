#pragma once

#include <KR3/main.h>
#include "vector.h"
#include "matrix.h"

namespace kr
{
	namespace math
	{
		extern "C" ATTR_ANY const vec4a CV_MINUS_X = vec4a(-1, 1, 1, 1);
		extern "C" ATTR_ANY const vec4a CV_MINUS_Y = vec4a(1, -1, 1, 1);
		extern "C" ATTR_ANY const vec4a CV_MINUS_Z = vec4a(1, 1, -1, 1);
		extern "C" ATTR_ANY const vec4a CV_MINUS_W = vec4a(1, 1, 1, -1);
		extern "C" ATTR_ANY const vec4a CV_MINUS_XZ = vec4a(-1, 1, -1, 1);
		extern "C" ATTR_ANY const mat4a CM_IDENTITY = {
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f,
		};
		extern "C" ATTR_ANY const vec4a & CV_1000 = CM_IDENTITY.v[0];
		extern "C" ATTR_ANY const vec4a & CV_0100 = CM_IDENTITY.v[1];
		extern "C" ATTR_ANY const vec4a & CV_0010 = CM_IDENTITY.v[2];
		extern "C" ATTR_ANY const vec4a & CV_0001 = CM_IDENTITY.v[3];
		extern "C" ATTR_ANY const vec4a CV_1100 = vec4a(1, 1, 0, 0);
		extern "C" ATTR_ANY const vec4a CV_1010 = vec4a(1, 0, 1, 0);
		extern "C" ATTR_ANY const vec4a CV_0110 = vec4a(0, 1, 1, 0);
		extern "C" ATTR_ANY const vec4a CV_1110 = vec4a(1, 1, 1, 0);
		extern "C" ATTR_ANY const vec4a CV_1001 = vec4a(1, 0, 0, 1);
		extern "C" ATTR_ANY const vec4a CV_0101 = vec4a(0, 1, 0, 1);
		extern "C" ATTR_ANY const vec4a CV_1101 = vec4a(1, 1, 0, 1);
		extern "C" ATTR_ANY const vec4a CV_0011 = vec4a(0, 0, 1, 1);
		extern "C" ATTR_ANY const vec4a CV_1011 = vec4a(1, 0, 1, 1);
		extern "C" ATTR_ANY const vec4a CV_0111 = vec4a(0, 1, 1, 1);
		extern "C" ATTR_ANY const vec4a CV_1 = vec4a(1, 1, 1, 1);
		extern "C" ATTR_ANY const ivec4a CV_ZERO = { 0, 0, 0, 0 };
		extern "C" ATTR_ANY const ivec4a CV_ALL_BITS = { -1, -1, -1, -1 };
		extern "C" ATTR_ANY const ivec4a CV_MASK_YZW = { 0, -1, -1, -1 };
		extern "C" ATTR_ANY const ivec4a CV_MASK_XZW = { -1, 0, -1, -1 };
		extern "C" ATTR_ANY const ivec4a CV_MASK_XYW = { -1, -1, 0, -1 };
		extern "C" ATTR_ANY const ivec4a CV_MASK_XYZ = { -1, -1, -1, 0 };
		extern "C" ATTR_ANY const ivec4a CV_MASK_X = { -1, 0, 0, 0 };
		extern "C" ATTR_ANY const ivec4a CV_MASK_Y = { 0, -1, 0, 0 };
		extern "C" ATTR_ANY const ivec4a CV_MASK_Z = { 0, 0, -1, 0 };
		extern "C" ATTR_ANY const ivec4a CV_MASK_W = { 0, 0, 0, -1 };
		extern "C" ATTR_ANY const ivec4a CV_MASK_ZW = { 0, 0, -1, -1 };
		extern "C" ATTR_ANY const vec4a CV_NEG_0010 = { 0, 0, -1, 0 };
		extern "C" ATTR_ANY const vec4a CV_00_N1_P1 = { 0, 0, -1, 1 };
		extern "C" ATTR_ANY const vec4a CV_P2N2_0_P1 = { 2, -2, 0, 1 };
		extern "C" ATTR_ANY const vec4a CV_N22_0_P1 = { -2, -2, 0, 1 };
		extern "C" ATTR_ANY const vec4a CV_000_PH = { 0, 0, 0, 0.5f };
		extern "C" ATTR_ANY const ivec4a CV_SIGNMASK((int)0x80000000);
	}
}